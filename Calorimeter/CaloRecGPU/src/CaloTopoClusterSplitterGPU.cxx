/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloTopoClusterSplitterGPU.h"
#include "CaloTopoClusterSplitterGPUImpl.h"


#include <string>
#include <climits> //For CHAR_BIT... though it's a slightly inefficient way of saying 8.

#include "CaloIdentifier/CaloCell_ID.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

#include "MacroHelpers.h"

using namespace CaloRecGPU;

CaloTopoClusterSplitterGPU::CaloTopoClusterSplitterGPU(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode CaloTopoClusterSplitterGPU::initialize()
{

  m_options.allocate();


  using PackType = decltype(m_options.m_options->m_useSampling);

  static_assert(CaloCell_ID::getNumberOfSamplings() <= sizeof(PackType) * CHAR_BIT,  "We are assuming that we have fewer samplings that bits per int!");

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


  auto process_sampling = [&get_option_from_string](const std::vector<std::string> & sampling_names, std::string & invalid_names, PackType & sampling_option)
  {
    sampling_option = 0;
    for (const std::string & samp_name : sampling_names)
      {
        bool failed = false;
        const PackType sampling = (PackType) get_option_from_string(samp_name, failed);

        if (failed)
          {
            if (invalid_names.size() == 0)
              {
                invalid_names = "'" + samp_name + "'";
              }
            else
              {
                invalid_names += ", '" + samp_name + "'";
              }
          }
        else
          {
            sampling_option |= ((PackType) 1) << sampling;
          }
      }
  };

  std::string invalid_names;

  process_sampling(m_samplingNames, invalid_names, m_options.m_options->m_useSampling);

  if (invalid_names.size() > 0)
    {
      ATH_MSG_ERROR( "Calorimeter samplings " << invalid_names
                     << " are not a valid Calorimeter sampling name and will be ignored! "
                     << "Valid names are: "
                     << "PreSamplerB, EMB1, EMB2, EMB3, "
                     << "PreSamplerE, EME1, EME2, EME3, "
                     << "HEC0, HEC1, HEC2, HEC3, "
                     << "TileBar0, TileBar1, TileBar2, "
                     << "TileGap1, TileGap2, TileGap3, "
                     << "TileExt0, TileExt1, TileExt2, "
                     << "FCAL0, FCAL1, FCAL2."  );
    }

  invalid_names.clear();

  process_sampling(m_secondarySamplingNames, invalid_names, m_options.m_options->m_useSecondarySampling);

  if (invalid_names.size() > 0)
    {
      ATH_MSG_ERROR( "Calorimeter samplings " << invalid_names
                     << " are not a valid Calorimeter sampling name and will be ignored! "
                     << "Valid names are: "
                     << "PreSamplerB, EMB1, EMB2, EMB3, "
                     << "PreSamplerE, EME1, EME2, EME3, "
                     << "HEC0, HEC1, HEC2, HEC3, "
                     << "TileBar0, TileBar1, TileBar2, "
                     << "TileGap1, TileGap2, TileGap3, "
                     << "TileExt0, TileExt1, TileExt2, "
                     << "FCAL0, FCAL1, FCAL2."  );
    }


  //We must repeat this printing part because ATH_MSG_ERROR
  //is a macro that apparently calls a this->msg(...) function.
  //Of course it won't work within a lambda...

  m_options.m_options->m_nCells = m_nCells;
  m_options.m_options->m_minEnergy = m_minEnergy;
  m_options.m_options->m_emShowerScale = m_emShowerScale;
  m_options.m_options->m_shareBorderCells = m_shareBorderCells;
  m_options.m_options->m_absOpt = m_absOpt;

  m_options.sendToGPU(true);


  if (m_numPreAllocatedGPUData > 0)
    {
      ATH_MSG_DEBUG("Pre-allocating temporaries for " << m_numPreAllocatedGPUData << " events.");

      m_temporariesHolder.resize(m_numPreAllocatedGPUData);
      //This will allocate the object holders.

      m_temporariesHolder.operate_on_all( [&](GPUSplitterTemporariesHolder & tth)
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

StatusCode CaloTopoClusterSplitterGPU::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const
{

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  GPUSplitterTemporariesHolder * temp_data_ptr = nullptr;

  Helpers::separate_thread_accessor<GPUSplitterTemporariesHolder> sep_th_acc(m_temporariesHolder, temp_data_ptr);
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

  preProcessingPreparation(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto before_maxima = clock_type::now();

  findLocalMaxima(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto before_propagate = clock_type::now();

  propagateTags(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto before_refill = clock_type::now();

  refillClusters(event_data, *temp_data_ptr, constant_data, m_options, m_measureTimes);

  const auto end = clock_type::now();


  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, before_maxima),
                   time_cast(before_maxima, before_propagate),
                   time_cast(before_propagate, before_refill),
                   time_cast(before_refill, end)
                  );
    }

  return StatusCode::SUCCESS;


}


StatusCode CaloTopoClusterSplitterGPU::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Find_Local_Maxima Splitter_Tag_Propagation Cluster_Refilling", 4);
    }
  return StatusCode::SUCCESS;
}


CaloTopoClusterSplitterGPU::~CaloTopoClusterSplitterGPU()
{
  //Nothing!
}