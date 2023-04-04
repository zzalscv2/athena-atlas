//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "GPUToAthenaImporterWithMoments.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "AthenaKernel/errorcheck.h"

#include <vector>
#include <algorithm>
#include <memory>

#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

GPUToAthenaImporterWithMoments::GPUToAthenaImporterWithMoments(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<ICaloClusterGPUOutputTransformer> (this);

}

#include "MacroHelpers.h"

StatusCode GPUToAthenaImporterWithMoments::initialize()
{
  ATH_CHECK( m_cellsKey.value().initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  ATH_CHECK(m_caloMgrKey.initialize());

  auto get_cluster_size_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, xAOD::CaloCluster,
                                        SW_55ele,
                                        SW_35ele,
                                        SW_37ele,
                                        SW_55gam,
                                        SW_35gam,
                                        SW_37gam,
                                        SW_55Econv,
                                        SW_35Econv,
                                        SW_37Econv,
                                        SW_softe,
                                        Topo_420,
                                        Topo_633,
                                        SW_7_11,
                                        SuperCluster,
                                        Tower_01_01,
                                        Tower_005_005,
                                        Tower_fixed_area
                                      )
    )
    //I know Topological Clustering only supports a subset of those,
    //but this is supposed to be a general data exporting tool...
    else
      {
        //failed = true;
        return xAOD::CaloCluster::CSize_Unknown;
      }
  };

  bool size_failed = false;
  m_clusterSize = get_cluster_size_from_string(m_clusterSizeString, size_failed);

  if (m_clusterSize == xAOD::CaloCluster::CSize_Unknown)
    {
      ATH_MSG_ERROR("Invalid Cluster Size: " << m_clusterSizeString);
    }

  if (size_failed)
    {
      return StatusCode::FAILURE;
    }


  auto get_moment_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, xAOD::CaloCluster,
                                        FIRST_PHI,
                                        FIRST_ETA,
                                        SECOND_R,
                                        SECOND_LAMBDA,
                                        DELTA_PHI,
                                        DELTA_THETA,
                                        DELTA_ALPHA,
                                        CENTER_X,
                                        CENTER_Y,
                                        CENTER_Z,
                                        CENTER_MAG,
                                        CENTER_LAMBDA,
                                        LATERAL,
                                        LONGITUDINAL,
                                        ENG_FRAC_EM,
                                        ENG_FRAC_MAX,
                                        ENG_FRAC_CORE,
                                        FIRST_ENG_DENS,
                                        SECOND_ENG_DENS,
                                        ISOLATION,
                                        ENG_BAD_CELLS,
                                        N_BAD_CELLS,
                                        N_BAD_CELLS_CORR,
                                        BAD_CELLS_CORR_E,
                                        BADLARQ_FRAC,
                                        ENG_POS,
                                        SIGNIFICANCE,
                                        CELL_SIGNIFICANCE,
                                        CELL_SIG_SAMPLING,
                                        AVG_LAR_Q,
                                        AVG_TILE_Q,
                                        ENG_BAD_HV_CELLS,
                                        N_BAD_HV_CELLS,
                                        PTD,
                                        MASS,
                                        EM_PROBABILITY,
                                        HAD_WEIGHT,
                                        OOC_WEIGHT,
                                        DM_WEIGHT,
                                        TILE_CONFIDENCE_LEVEL,
                                        SECOND_TIME,
                                        NCELL_SAMPLING,
                                        VERTEX_FRACTION,
                                        NVERTEX_FRACTION,
                                        ETACALOFRAME,
                                        PHICALOFRAME,
                                        ETA1CALOFRAME,
                                        PHI1CALOFRAME,
                                        ETA2CALOFRAME,
                                        PHI2CALOFRAME,
                                        ENG_CALIB_TOT,
                                        ENG_CALIB_OUT_L,
                                        ENG_CALIB_OUT_M,
                                        ENG_CALIB_OUT_T,
                                        ENG_CALIB_DEAD_L,
                                        ENG_CALIB_DEAD_M,
                                        ENG_CALIB_DEAD_T,
                                        ENG_CALIB_EMB0,
                                        ENG_CALIB_EME0,
                                        ENG_CALIB_TILEG3,
                                        ENG_CALIB_DEAD_TOT,
                                        ENG_CALIB_DEAD_EMB0,
                                        ENG_CALIB_DEAD_TILE0,
                                        ENG_CALIB_DEAD_TILEG3,
                                        ENG_CALIB_DEAD_EME0,
                                        ENG_CALIB_DEAD_HEC0,
                                        ENG_CALIB_DEAD_FCAL,
                                        ENG_CALIB_DEAD_LEAKAGE,
                                        ENG_CALIB_DEAD_UNCLASS,
                                        ENG_CALIB_FRAC_EM,
                                        ENG_CALIB_FRAC_HAD,
                                        ENG_CALIB_FRAC_REST)
    )
    else
      {
        failed = true;
        return xAOD::CaloCluster::ENERGY_DigiHSTruth;
      }
  };


  auto process_moments = [&](const std::vector<std::string> & moment_names, std::string & invalid_names)
  {
    for (const std::string & mom_name : moment_names)
      {
        bool failed = false;
        const int linear_num = MomentsOptionsArray::moment_to_linear(get_moment_from_string(mom_name, failed));

        failed = failed || linear_num >= MomentsOptionsArray::num_moments;

        if (failed)
          {
            if (invalid_names.size() == 0)
              {
                invalid_names = "'" + mom_name + "'";
              }
            else
              {
                invalid_names += ", '" + mom_name + "'";
              }
          }
        else
          {
            m_momentsToDo.array[linear_num] = true;
          }
      }
  };

  std::string invalid_names;

  process_moments(m_momentsNames, invalid_names);

  if (invalid_names.size() > 0)
    {
      ATH_MSG_ERROR( "Moments " << invalid_names
                     << " are not valid moments and will be ignored!" );
    }

  if (m_momentsToDo[xAOD::CaloCluster::CENTER_LAMBDA])
    {
      CHECK(m_caloDepthTool.retrieve());
    }

  if (m_momentsToDo[xAOD::CaloCluster::ENG_BAD_HV_CELLS] || m_momentsToDo[xAOD::CaloCluster::N_BAD_HV_CELLS])
    {
      ATH_CHECK(m_larHVFraction.retrieve());
    }

  return StatusCode::SUCCESS;
}


StatusCode GPUToAthenaImporterWithMoments::convert (const EventContext & ctx,
                                                    const ConstantDataHolder &,
                                                    EventDataHolder & ed,
                                                    xAOD::CaloClusterContainer * cluster_container) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };
  
  
  const auto start = clock_type::now();
  
  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::RECOVERABLE;
    }
  const DataLink<CaloCellContainer> cell_collection_link (cell_collection.name(), ctx);

  //ed.returnToCPU(!m_keepGPUData, true, true, true);

  const auto pre_processing = clock_type::now();
  
  ed.returnClusterNumberToCPU();
  CaloRecGPU::CUDA_Helpers::GPU_synchronize();
  
  
  const auto cluster_number = clock_type::now();
  
  ed.returnSomeClustersToCPU(ed.m_clusters->number);

  std::vector<std::unique_ptr<CaloClusterCellLink>> cell_links;

  cell_links.reserve(ed.m_clusters->number);

  size_t valid_clusters = 0;

  CaloRecGPU::CUDA_Helpers::GPU_synchronize();

  const auto clusters = clock_type::now();
  
  ed.returnCellsToCPU();

  for (int i = 0; i < ed.m_clusters->number; ++i)
    {
      if (ed.m_clusters->seedCellID[i] >= 0)
        {
          cell_links.emplace_back(std::make_unique<CaloClusterCellLink>(cell_collection_link));
          cell_links.back()->reserve(256);
          //To be adjusted.
          ++valid_clusters;
        }
      else
        {
          cell_links.emplace_back(nullptr);
          //The excluded clusters don't have any cells.
        }
    }
  CaloRecGPU::CUDA_Helpers::GPU_synchronize();

  const auto cells = clock_type::now();
  
  ed.returnSomeMomentsToCPU(ed.m_clusters->number);

  if (cell_collection->isOrderedAndComplete())
    //Fast path: cell indices within the collection and identifierHashes match!
    {
      for (int cell_index = 0; cell_index < NCaloCells; ++cell_index)
        {
          const ClusterTag this_tag = ed.m_cell_state->clusterTag[cell_index];

          if (this_tag.is_part_of_cluster())
            {
              const int this_index = this_tag.cluster_index();
              const int32_t weight_pattern = this_tag.secondary_cluster_weight();

              float tempf = 1.0f;

              std::memcpy(&tempf, &weight_pattern, sizeof(float));
              //C++20 would give us bit cast to do this more properly.
              //Still, given how the bit pattern is created,
              //it should be safe.

              const float reverse_weight = tempf;

              const float this_weight = 1.0f - reverse_weight;

              cell_links[this_index]->addCell(cell_index, this_weight);

              if (cell_index == ed.m_clusters->seedCellID[this_index] && cell_links[this_index]->size() > 1)
                //Seed cells aren't shared,
                //so no need to check this on the other case.
                {
                  CaloClusterCellLink::iterator begin_it = cell_links[this_index]->begin();
                  CaloClusterCellLink::iterator back_it  = (--cell_links[this_index]->end());

                  const unsigned int first_idx = begin_it.index();
                  const double first_wgt = begin_it.weight();

                  begin_it.reindex(back_it.index());
                  begin_it.reweight(back_it.weight());

                  back_it.reindex(first_idx);
                  back_it.reweight(first_wgt);

                  //Of course, this is to ensure the first cell is the seed cell,
                  //in accordance to the way some cluster properties
                  //(mostly phi-related) are calculated.
                }

              if (this_tag.is_shared_between_clusters())
                {
                  const int other_index = this_tag.secondary_cluster_index();
                  cell_links[other_index]->addCell(cell_index, reverse_weight);
                }
            }
        }
    }
  else
    //Slow path: be careful.
    {
      CaloCellContainer::const_iterator iCells = cell_collection->begin();

      for (int cell_count = 0; iCells != cell_collection->end(); ++iCells, ++cell_count)
        {
          const CaloCell * cell = (*iCells);

          //const int cell_index = m_calo_id->calo_cell_hash(cell->ID());
          const int cell_index = cell->caloDDE()->calo_hash();

          const ClusterTag this_tag = ed.m_cell_state->clusterTag[cell_index];

          if (this_tag.is_part_of_cluster())
            {
              const int this_index = this_tag.cluster_index();
              const int32_t weight_pattern = this_tag.secondary_cluster_weight();

              float tempf = 1.0f;

              std::memcpy(&tempf, &weight_pattern, sizeof(float));
              //C++20 would give us bit cast to do this more properly.
              //Still, given how the bit pattern is created,
              //it should be safe.

              const float reverse_weight = tempf;

              const float this_weight = 1.0f - reverse_weight;

              cell_links[this_index]->addCell(cell_count, this_weight);
              //So we put this in the right cell link.

              if (cell_index == ed.m_clusters->seedCellID[this_index] && cell_links[this_index]->size() > 1)
                //Seed cells aren't shared,
                //so no need to check this on the other case.
                {
                  CaloClusterCellLink::iterator begin_it = cell_links[this_index]->begin();
                  CaloClusterCellLink::iterator back_it  = (--cell_links[this_index]->end());

                  const unsigned int first_idx = begin_it.index();
                  const double first_wgt = begin_it.weight();

                  begin_it.reindex(back_it.index());
                  begin_it.reweight(back_it.weight());

                  back_it.reindex(first_idx);
                  back_it.reweight(first_wgt);

                  //Of course, this is to ensure the first cell is the seed cell,
                  //in accordance to the way some cluster properties
                  //(mostly phi-related) are calculated.
                }

              if (this_tag.is_shared_between_clusters())
                {
                  const int other_index = this_tag.secondary_cluster_index();
                  cell_links[other_index]->addCell(cell_count, reverse_weight);
                }
            }
        }
    }

  const auto end_cell_cycle = clock_type::now();
  
  std::vector<int> cluster_order(ed.m_clusters->number);

  std::iota(cluster_order.begin(), cluster_order.end(), 0);

  std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
  {
    return ed.m_clusters->clusterEt[a] > ed.m_clusters->clusterEt[b];
  } );

  //Ordered by Et as in the default algorithm...
  //The fact that some invalid clusters
  //(with possibly trash values for Et)
  //can crop up is irrelevant since
  //we don't add those anyway:
  //the rest is still ordered like we want it to be.

  const auto ordered = clock_type::now();
  
  cluster_container->clear();
  cluster_container->reserve(cell_links.size());

  std::vector<int> real_cluster_order;
  real_cluster_order.reserve(cluster_order.size());

  for (size_t i = 0; i < cluster_order.size(); ++i)
    {
      const int cluster_index = cluster_order[i];

      if (cell_links[cluster_index] != nullptr && cell_links[cluster_index]->size() > 0)
        {
          xAOD::CaloCluster * cluster = new xAOD::CaloCluster();
          cluster_container->push_back(cluster);

          cluster->addCellLink(cell_links[cluster_index].release());
          cluster->setClusterSize(m_clusterSize);

          cluster->setEta(ed.m_clusters->clusterEta[cluster_index]);
          cluster->setPhi(ed.m_clusters->clusterPhi[cluster_index]);
          
          cluster->setE(ed.m_clusters->clusterEnergy[cluster_index]);
          cluster->setM(0.0);

          real_cluster_order.push_back(cluster_index);
        }

    }

  const auto pre_moments = clock_type::now();
  
  CaloRecGPU::CUDA_Helpers::GPU_synchronize();

  
  const auto post_moments = clock_type::now();
  
  for (size_t i = 0; i < cluster_container->size(); ++i)
    {
      xAOD::CaloCluster * cluster = (*cluster_container)[i];
      const int cluster_index = real_cluster_order[i];

      cluster->setTime(ed.m_moments->time[cluster_index]);
      cluster->setSecondTime(ed.m_moments->secondTime[cluster_index]);
      cluster->clearSamplingData();

      uint32_t sampling_pattern = 0;
      for (int i = 0; i < NumSamplings; ++i)
        {
          const int cells_per_sampling = ed.m_moments->nCellSampling[i][cluster_index];

          if (cells_per_sampling > 0)
            {
              sampling_pattern |= (0x1U << i);
            }
        }
      cluster->setSamplingPattern(sampling_pattern);
      
      for (int i = 0; i < NumSamplings; ++i)
        {
          const int cells_per_sampling = ed.m_moments->nCellSampling[i][cluster_index];

          if (cells_per_sampling > 0)
            {
              cluster->setEnergy  ((CaloSampling::CaloSample) i, ed.m_moments->energyPerSample [i][cluster_index]);
              cluster->setEta     ((CaloSampling::CaloSample) i, ed.m_moments->etaPerSample    [i][cluster_index]);
              cluster->setPhi     ((CaloSampling::CaloSample) i, ed.m_moments->phiPerSample    [i][cluster_index]);
              cluster->setEmax    ((CaloSampling::CaloSample) i, ed.m_moments->maxEPerSample   [i][cluster_index]);
              cluster->setEtamax  ((CaloSampling::CaloSample) i, ed.m_moments->maxEtaPerSample [i][cluster_index]);
              cluster->setPhimax  ((CaloSampling::CaloSample) i, ed.m_moments->maxPhiPerSample [i][cluster_index]);
            }

          if (m_momentsToDo[xAOD::CaloCluster::NCELL_SAMPLING])
            {
              cluster->setNumberCellsInSampling((CaloSampling::CaloSample) i, cells_per_sampling, false);
            }
        }
      
#define CALORECGPU_MOMENTS_CONVERSION_HELPER(MOMENT_ENUM, MOMENT_ARRAY)                                     \
  if (m_momentsToDo[xAOD::CaloCluster:: MOMENT_ENUM ] )                                                     \
    {                                                                                                       \
      cluster->insertMoment(xAOD::CaloCluster:: MOMENT_ENUM , ed.m_moments-> MOMENT_ARRAY [cluster_index]); \
    }


#define CALORECGPU_MOMENTS_CONVERSION_INVALID(MOMENT_ENUM)                                                  \
  if (m_momentsToDo[xAOD::CaloCluster:: MOMENT_ENUM ] )                                                     \
    {                                                                                                       \
      ATH_MSG_WARNING("Moment '" << # MOMENT_ENUM <<                                                        \
                      "' given as a calculated moment, but not yet supported on the GPU side...");          \
    }

      CALORECGPU_MOMENTS_CONVERSION_HELPER(FIRST_PHI,         firstPhi          );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(FIRST_ETA,         firstEta          );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(SECOND_R,          secondR           );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(SECOND_LAMBDA,     secondLambda      );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(DELTA_PHI,         deltaPhi          );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(DELTA_THETA,       deltaTheta        );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(DELTA_ALPHA,       deltaAlpha        );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(CENTER_X,          centerX           );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(CENTER_Y,          centerY           );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(CENTER_Z,          centerZ           );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(CENTER_MAG,        centerMag         );

      //Center lambda ---> is *the* special case.

      CALORECGPU_MOMENTS_CONVERSION_HELPER(LATERAL,           lateral           );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(LONGITUDINAL,      longitudinal      );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(ENG_FRAC_EM,       engFracEM         );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(ENG_FRAC_MAX,      engFracMax        );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(ENG_FRAC_CORE,     engFracCore       );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(FIRST_ENG_DENS,    firstEngDens      );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(SECOND_ENG_DENS,   secondEngDens     );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(ISOLATION,         isolation         );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(ENG_BAD_CELLS,     engBadCells       );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(N_BAD_CELLS,       nBadCells         );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(N_BAD_CELLS_CORR,  nBadCellsCorr     );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(BAD_CELLS_CORR_E,  badCellsCorrE     );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(BADLARQ_FRAC,      badLArQFrac       );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(ENG_POS,           engPos            );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(SIGNIFICANCE,      significance      );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(CELL_SIGNIFICANCE, cellSignificance  );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(CELL_SIG_SAMPLING, cellSigSampling   );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(AVG_LAR_Q,         avgLArQ           );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(AVG_TILE_Q,        avgTileQ          );

      //ENG_BAD_HV_CELLS ---\ These ones are also
      //N_BAD_HV_CELLS   ---/ (less) special cases.

      CALORECGPU_MOMENTS_CONVERSION_HELPER(PTD,               PTD               );
      CALORECGPU_MOMENTS_CONVERSION_HELPER(MASS,              mass              );

      CALORECGPU_MOMENTS_CONVERSION_INVALID(EM_PROBABILITY                      );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(HAD_WEIGHT                          );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(OOC_WEIGHT                          );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(DM_WEIGHT                           );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(TILE_CONFIDENCE_LEVEL               );


      CALORECGPU_MOMENTS_CONVERSION_HELPER(SECOND_TIME, secondTime);

      if (m_momentsToDo[xAOD::CaloCluster::NCELL_SAMPLING])
        {
          const int extra_sampling_count = ed.m_moments->nExtraCellSampling[cluster_index];
          if (extra_sampling_count > 0)
            {
              cluster->setNumberCellsInSampling(CaloSampling::EME2, extra_sampling_count, true);
            }
        }

      CALORECGPU_MOMENTS_CONVERSION_INVALID(VERTEX_FRACTION                     );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(NVERTEX_FRACTION                    );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ETACALOFRAME                        );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(PHICALOFRAME                        );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ETA1CALOFRAME                       );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(PHI1CALOFRAME                       );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ETA2CALOFRAME                       );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(PHI2CALOFRAME                       );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_TOT                       );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_OUT_L                     );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_OUT_M                     );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_OUT_T                     );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_L                    );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_M                    );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_T                    );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_EMB0                      );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_EME0                      );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_TILEG3                    );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_TOT                  );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_EMB0                 );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_TILE0                );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_TILEG3               );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_EME0                 );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_HEC0                 );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_FCAL                 );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_LEAKAGE              );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_DEAD_UNCLASS              );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_FRAC_EM                   );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_FRAC_HAD                  );
      CALORECGPU_MOMENTS_CONVERSION_INVALID(ENG_CALIB_FRAC_REST                 );

      //Maybe things to do with DigiHSTruth, if needed?

      //Now the special cases.
      //Hopefully, we might find a solution
      //for those in the not-too-distant future.

      if (m_momentsToDo[xAOD::CaloCluster::ENG_BAD_HV_CELLS] || m_momentsToDo[xAOD::CaloCluster::N_BAD_HV_CELLS])
        {
          const auto hvFrac = m_larHVFraction->getLArHVFrac(cluster->getCellLinks(), ctx);
          const double eBadLArHV = hvFrac.first;
          const double nBadLArHV = hvFrac.second;
          if (m_momentsToDo[xAOD::CaloCluster::ENG_BAD_HV_CELLS])
            {
              cluster->insertMoment(xAOD::CaloCluster::ENG_BAD_HV_CELLS, eBadLArHV);
            }
          if (m_momentsToDo[xAOD::CaloCluster::N_BAD_HV_CELLS])
            {
              cluster->insertMoment(xAOD::CaloCluster::N_BAD_HV_CELLS, nBadLArHV);
            }
        }

      if (m_momentsToDo[xAOD::CaloCluster::CENTER_LAMBDA])
        //Ooof.
        {
          //Copied/adapted from the CPU version.
          //Not the most optimized thing in the world,
          //especially since the get_* functions are expensive...

          SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
          const CaloDetDescrManager * caloDDMgr = *caloMgrHandle;

          Amg::Vector3D showerCenter(ed.m_moments->centerX[cluster_index], ed.m_moments->centerY[cluster_index], ed.m_moments->centerZ[cluster_index]);
          Amg::Vector3D showerAxis(ed.m_moments->EMProbability[cluster_index], ed.m_moments->hadWeight[cluster_index], ed.m_moments->OOCweight[cluster_index]);

          //These are the non-overwritten temporaries from the GPU code.
          //Dirty, I know.
          //But we plan on having this on the GPU side instead, right?

          double r_calo(0), z_calo(0), lambda_c(0);
          r_calo = m_caloDepthTool->get_entrance_radius(CaloCell_ID::EMB1,
                                                        showerCenter.eta(),
                                                        showerCenter.phi(),
                                                        caloDDMgr);
          if ( r_calo == 0 )
            {
              z_calo = m_caloDepthTool->get_entrance_z(CaloCell_ID::EME1,
                                                       showerCenter.eta(),
                                                       showerCenter.phi(),
                                                       caloDDMgr);
              if ( z_calo == 0 )
                {
                  z_calo = m_caloDepthTool->get_entrance_z(CaloCell_ID::EME2,
                                                           showerCenter.eta(),
                                                           showerCenter.phi(),
                                                           caloDDMgr);
                }
              if ( z_calo == 0 )
                {
                  z_calo = m_caloDepthTool->get_entrance_z(CaloCell_ID::FCAL0,
                                                           showerCenter.eta(),
                                                           showerCenter.phi(),
                                                           caloDDMgr);
                }
              if ( z_calo == 0 )  // for H6 TB without EMEC outer wheel
                {
                  z_calo = m_caloDepthTool->get_entrance_z(CaloCell_ID::HEC0,
                                                           showerCenter.eta(),
                                                           showerCenter.phi(),
                                                           caloDDMgr);
                }
              if ( z_calo != 0 && showerAxis.z() != 0 )
                {
                  lambda_c = fabs((z_calo - showerCenter.z()) / showerAxis.z());
                }
            }
          else
            {
              double r_s2 = showerAxis.x() * showerAxis.x()
                            + showerAxis.y() * showerAxis.y();
              double r_cs = showerAxis.x() * showerCenter.x()
                            + showerAxis.y() * showerCenter.y();
              double r_cr = showerCenter.x() * showerCenter.x()
                            + showerCenter.y() * showerCenter.y() - r_calo * r_calo;
              if ( r_s2 > 0 )
                {
                  double det = r_cs * r_cs / (r_s2 * r_s2) - r_cr / r_s2;
                  if ( det > 0 )
                    {
                      det = sqrt(det);
                      double l1(-r_cs / r_s2);
                      double l2(l1);
                      l1 += det;
                      l2 -= det;
                      if ( fabs(l1) < fabs(l2) )
                        {
                          lambda_c = fabs(l1);
                        }
                      else
                        {
                          lambda_c = fabs(l2);
                        }
                    }
                }
            }
          cluster->insertMoment(xAOD::CaloCluster::CENTER_LAMBDA, lambda_c);
        }
    }
  
  const auto end = clock_type::now();
  
  if (!m_keepGPUData)
    {
      ed.clear_GPU();
    }

  
  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, pre_processing),
                   time_cast(pre_processing, cluster_number),
                   time_cast(cluster_number, clusters),
                   time_cast(clusters, cells),
                   time_cast(cells, end_cell_cycle),
                   time_cast(end_cell_cycle, ordered),
                   time_cast(ordered, pre_moments),
                   time_cast(pre_moments, post_moments),
                   time_cast(post_moments, end)
                  );
    }

  
  return StatusCode::SUCCESS;

}


StatusCode GPUToAthenaImporterWithMoments::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Cluster_Number Clusters Cells Cell_Cycle Ordering Cluster_Creation Moments_Transfer Moments_Fill",9);
    }
  return StatusCode::SUCCESS;
}


GPUToAthenaImporterWithMoments::~GPUToAthenaImporterWithMoments()
{
  //Nothing!
}