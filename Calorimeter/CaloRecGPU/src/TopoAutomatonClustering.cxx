/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TopoAutomatonClustering.h"
#include "TopoAutomatonClusteringGPU.h"


#include <string>
#include <climits> //For CHAR_BIT... though it's a slightly inefficient way of saying 8.

#include "CaloIdentifier/CaloCell_ID.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

#include "MacroHelpers.h"

using namespace CaloRecGPU;

TopoAutomatonClustering::TopoAutomatonClustering(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode TopoAutomatonClustering::initialize()
{

  m_options.allocate();


  using PackType = decltype(m_options.m_options->validSamplingSeed);

  PackType & seed_samplings = m_options.m_options->validSamplingSeed;

  seed_samplings = 0;

  static_assert(CaloCell_ID::getNumberOfSamplings() <= sizeof(PackType) * CHAR_BIT,  "We are assuming that we have fewer samplings that bits per int!");

  //Possibly more elegant alternative: compile-time packed bool vector (std::bitset?) with CUDA compat.
  //Been there, done that, overkill since number of samplings shouldn't change unexpectedly overnight
  //and it doesn't seem that likely to me that it'll reach anything that a 64-bit int wouldn't cover
  //(at least without implying such a major overhaul that the code will need a deeper redesign anyway...)

  auto get_option_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
    CRGPU_CHEAP_STRING_TO_ENUM( str, CaloCell_ID,
                                PreSamplerB,
                                EMB1,
                                EMB2,
                                EMB3,
                                PreSamplerE,
                                EME1,
                                EME2,
                                EME3,
                                HEC0,
                                HEC1,
                                HEC2,
                                HEC3,
                                TileBar0,
                                TileBar1,
                                TileBar2,
                                TileGap1,
                                TileGap2,
                                TileGap3,
                                TileExt0,
                                TileExt1,
                                TileExt2,
                                FCAL0,
                                FCAL1,
                                FCAL2
                              )
    )
    else
      {
        failed = true;
        return CaloCell_ID::Unknown;
      }
  };


  for (const std::string & samp_name : m_samplingNames)
    {
      bool failed = false;
      const PackType sampling = (PackType) get_option_from_string(samp_name, failed);

      if (failed)
        {
        ATH_MSG_ERROR( "Calorimeter sampling" << samp_name
                       << " is not a valid Calorimeter sampling name and will be ignored! "
                       << "Valid names are: "
                       << "PreSamplerB, EMB1, EMB2, EMB3, "
                       << "PreSamplerE, EME1, EME2, EME3, "
                       << "HEC0, HEC1, HEC2, HEC3, "
                       << "TileBar0, TileBar1, TileBar2, "
                       << "TileGap1, TileGap2, TileGap3, "
                       << "TileExt0, TileExt1, TileExt2, "
                       << "FCAL0, FCAL1, FCAL2."  );
        }
      else
        {
          seed_samplings |= ((PackType) 1) << sampling;
        }
    }

  m_options.m_options->seed_threshold = m_seedThresholdOnEorAbsEinSigma;
  m_options.m_options->grow_threshold = m_neighborThresholdOnEorAbsEinSigma;
  m_options.m_options->terminal_threshold = m_cellThresholdOnEorAbsEinSigma;
  m_options.m_options->abs_seed = m_seedCutsInAbsE;
  m_options.m_options->abs_grow = m_neighborCutsInAbsE;
  m_options.m_options->abs_terminal = m_cellCutsInAbsE;
  m_options.m_options->use_two_gaussian = m_twoGaussianNoise;

  m_options.sendToGPU(true);


  if (m_numPreAllocatedGPUData > 0)
    {
      ATH_MSG_DEBUG("Pre-allocating temporaries for " << m_numPreAllocatedGPUData << " events.");

      m_temporariesHolder.resize(m_numPreAllocatedGPUData);
      //This will allocate the object holders.

      m_temporariesHolder.operate_on_all( [&](TACTemporariesHolder & tth)
      {
        tth.allocate();
      }
                                        );
      //This will allocate all the memory at this point.
      //Also useful to prevent/debug potential allocation issues?
      //But the main point is really reducing the execute times...
    }

  return StatusCode::SUCCESS;

}

StatusCode TopoAutomatonClustering::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const
{

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  TACTemporariesHolder * temp_data_ptr = nullptr;

  Helpers::separate_thread_accessor<TACTemporariesHolder> sep_th_acc(m_temporariesHolder, temp_data_ptr);
  //This is a RAII wrapper to access an object held by Helpers::separate_thread_holder,
  //to ensure the event data is appropriately released when we are done processing.

  if (temp_data_ptr == nullptr)
    {
      ATH_MSG_ERROR("Could not get valid Temporary Data Holder! Event: " << ctx.evt() );
      return StatusCode::FAILURE;
    }

  temp_data_ptr->allocate();
  //Does nothing if it is already allocated
  //(which it should be unless a new holder had to be created
  // due to m_numPreAllocatedGPUData being less than the number of threads.)

  const auto before_snr = clock_type::now();

  signalToNoise(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto before_pairs = clock_type::now();

  cellPairs(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto before_growing = clock_type::now();

  clusterGrowing(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto end = clock_type::now();


  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, before_snr),
                   time_cast(before_snr, before_pairs),
                   time_cast(before_pairs, before_growing),
                   time_cast(before_growing, end)
                  );
    }

  return StatusCode::SUCCESS;


}


StatusCode TopoAutomatonClustering::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Signal-to-Noise_Ratio Cell_Pair_Creation Cluster_Growing", 4);
    }
  return StatusCode::SUCCESS;
}


TopoAutomatonClustering::~TopoAutomatonClustering()
{
  //Nothing!
}