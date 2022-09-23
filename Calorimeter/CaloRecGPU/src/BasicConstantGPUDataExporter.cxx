/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "BasicConstantGPUDataExporter.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "CxxUtils/checker_macros.h"

#include "AthenaKernel/errorcheck.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/DataHandle.h"
#include "CaloConditions/CaloNoise.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

BasicConstantGPUDataExporter::BasicConstantGPUDataExporter(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this),
  m_hasBeenInitialized(false)
{
  declareInterface<ICaloClusterGPUConstantTransformer> (this);
}

#include "MacroHelpers.h"

StatusCode BasicConstantGPUDataExporter::initialize()
{
  if (m_hasBeenInitialized)
    {
      ATH_MSG_DEBUG("Initializing data tool again...");
      return StatusCode::SUCCESS;
    }

  ATH_CHECK(m_noiseCDOKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());
  auto get_option_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
    CRGPU_CHEAP_STRING_TO_ENUM( str, LArNeighbours,
                                prevInPhi,
                                nextInPhi,
                                prevInEta,
                                nextInEta,
                                faces2D,
                                corners2D,
                                all2D,
                                prevInSamp,
                                nextInSamp,
                                upAndDown,
                                prevSubDet,
                                nextSubDet,
                                all3D,
                                corners3D,
                                all3DwithCorners,
                                prevSuperCalo,
                                nextSuperCalo,
                                super3D
                              )
    )
    //I know Topological Clustering only supports a subset of those,
    //but this is supposed to be a general data exporting tool...
    else
      {
        failed = true;
        return LArNeighbours::super3D;
      }
  };

  bool neigh_failed = false;
  m_neighborOption = get_option_from_string(m_neighborOptionString, neigh_failed);

  if (neigh_failed)
    {
      ATH_MSG_ERROR("Invalid Neighbour Option: " << m_neighborOptionString);
    }

  if (neigh_failed)
    {
      return StatusCode::FAILURE;
    }

  m_hasBeenInitialized = true;

  return StatusCode::SUCCESS;
}

StatusCode BasicConstantGPUDataExporter::convert(ConstantDataHolder &) const
{
  ATH_MSG_ERROR("BasicConstantGPUDataExporter (" << this->name() << ") must be used with the "
                "GPU data preparation happening on the first event.");

  return StatusCode::FAILURE;
}

StatusCode BasicConstantGPUDataExporter::convert(const EventContext & ctx, ConstantDataHolder & cd) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  auto start = clock_type::now();

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey,ctx};
  const CaloDetDescrManager * calo_dd_man = *caloMgrHandle;

  cd.m_geometry.allocate();

  const CaloCell_ID * calo_id = calo_dd_man->getCaloCell_ID();
  auto neigh_option_test = [&](const CaloCell_ID::SUBCALO sub_detector, const Identifier cell_id)
  {
    const bool doRestrictHECIWandFCal = m_restrictHECIWandFCalNeighbors && (m_neighborOption & LArNeighbours::nextInSamp);
    const bool doRestrictPS = m_restrictPSNeighbors && (m_neighborOption & LArNeighbours::nextInSamp);
    if (doRestrictHECIWandFCal && sub_detector != CaloCell_ID::LAREM)
      {
        if (sub_detector == CaloCell_ID::LARHEC && calo_id->region(cell_id) == 1)
          {
            return true;
          }
        if (sub_detector == CaloCell_ID::LARFCAL && calo_id->sampling(cell_id) > 1)
          {
            return true;
          }
      }
    if (doRestrictPS && sub_detector == CaloCell_ID::LAREM && calo_id->sampling(cell_id) == 0)
      {
        return true;
      }
    return false;
  };

  std::vector<IdentifierHash> neighbour_vector;
  
  for (int cell = 0; cell < NCaloCells; ++cell)
  {
    cd.m_geometry->nNeighbours[cell] = 0;
  }
  
  for (int cell = 0; cell < NCaloCells; ++cell)
    {
      const CaloDetDescrElement * caloElement = calo_dd_man->get_element((IdentifierHash) cell);
      CaloCell_ID::SUBCALO sub_detector = caloElement->getSubCalo();

      const LArNeighbours::neighbourOption opt = ( neigh_option_test(sub_detector, calo_id->cell_id((IdentifierHash) cell)) ? LArNeighbours::nextInSamp : m_neighborOption );

      calo_id->get_neighbours((IdentifierHash) cell, opt, neighbour_vector);

      //Fill the GPU Geometry
      cd.m_geometry->caloSample[cell] = calo_id->calo_sample(calo_id->cell_id((IdentifierHash) cell));
      cd.m_geometry->x[cell] = caloElement->x();
      cd.m_geometry->y[cell] = caloElement->y();
      cd.m_geometry->z[cell] = caloElement->z();
      cd.m_geometry->eta[cell] = caloElement->eta();
      cd.m_geometry->phi[cell] = caloElement->phi();
      //cd.m_geometry->nNeighbours[cell] = neighbour_vector.size();
      
      for (size_t j = 0; j < neighbour_vector.size(); ++j)
        {
          const int neigh = neighbour_vector[j];
          int & neigh_num_neighs = cd.m_geometry->nNeighbours[neigh];
          cd.m_geometry->neighbours[neigh][neigh_num_neighs] = cell;
          ++neigh_num_neighs;
        }
      //We want to list the cells from which a tag can be propagated,
      //not the other way around.
      
      cd.m_geometry->nReverseNeighbours[cell] = neighbour_vector.size();
      for (size_t j = 0; j < neighbour_vector.size(); ++j)
        {
          cd.m_geometry->reverseNeighbours[cell][j] = neighbour_vector[j];
        }
      
    }

  auto after_geo = clock_type::now();
/*
  if (m_correctNonSymmetricNeighs)
    {

      for (int cell = 0; cell < NCaloCells; ++cell)
        {
          const int n_neighs = cd.m_geometry->nNeighbours[cell];

          for (int i = 0; i < n_neighs; ++i)
            {
              const int neigh = cd.m_geometry->neighbours[cell][i];

              const int neigh_neighs = cd.m_geometry->nNeighbours[neigh];

              bool symmetry = false;

              for (int j = 0; j < neigh_neighs; ++j)
                {
                  if (cd.m_geometry->neighbours[neigh][j] == cell)
                    {
                      symmetry = true;
                      break;
                    }
                }
              if (symmetry == false)
                {
                  cd.m_geometry->neighbours[neigh][neigh_neighs] = cell;
                  cd.m_geometry->nNeighbours[neigh] += 1;
                  ATH_MSG_INFO("Non-bijective neighbours: " << cell << "</-  -> " << neigh);
                }
            }
        }
    }

  auto after_corr = clock_type::now();
  */

  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  CaloNoise * noise_tool ATLAS_THREAD_SAFE = const_cast<CaloNoise *>(*noise_handle);
  //We are committing the sin of const-casting to access larStorage and tileStorage.
  //I think const views into these could reasonably be added to the CaloNoise itself,
  //but I don't want to change unrelated portions of the code and I promise I'll behave.
  //Also suppress non-thread-safe complaints as we'll only be reading...
  //(Furthermore, constant data preparation should only happen in one thread anyway.)

  IdentifierHash temp1, temp2;
  calo_id->calo_cell_hash_range(CaloCell_ID::TILE, temp1, temp2);

  const int tile_offset = temp1;

  cd.m_cell_noise.allocate();

  //Were it not for the fact that the LAr noise array inside
  //the CaloNoise has three gains while the Tile has all four,
  //we might try to do something more memcpy-y...
  //(Still, with hope, it's highly optimized anyway.)

  for (int gain_state = 0; gain_state < CaloRecGPU::NumGainStates; ++gain_state)
    {
      for (int cell = 0; cell < tile_offset; ++cell)
        {
          cd.m_cell_noise->noise[gain_state][cell] = noise_tool->larStorage()[(gain_state > 2 ? 0 : gain_state)][cell];
        }
      for (int cell = tile_offset; cell < NCaloCells; ++cell)
        {
          cd.m_cell_noise->noise[gain_state][cell] = noise_tool->tileStorage()[gain_state][cell - tile_offset];
        }
    }

  auto after_noise = clock_type::now();

  cd.sendToGPU(!m_keepCPUData);


  auto after_send = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(), time_cast(start, after_geo),
                   time_cast(after_geo/*, after_corr),
                   time_cast(after_corr*/, after_noise),
                   time_cast(after_noise, after_send)
                  );
    }

  return StatusCode::SUCCESS;

}

StatusCode BasicConstantGPUDataExporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Geometry Geometry_Correction Noise Transfer_to_GPU", 3 /*4*/);
    }
  return StatusCode::SUCCESS;
}


BasicConstantGPUDataExporter::~BasicConstantGPUDataExporter()
{
  //Nothing!
}
