/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "BasicEventDataGPUExporter.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "AthenaKernel/errorcheck.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/DataHandle.h"
#include "CaloRec/CaloBadCellHelper.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

BasicEventDataGPUExporter::BasicEventDataGPUExporter(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<ICaloClusterGPUInputTransformer> (this);
}


StatusCode BasicEventDataGPUExporter::initialize()
{

  ATH_CHECK( m_noiseCDOKey.initialize() );
  ATH_CHECK( m_cellsKey.value().initialize() );
  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );
  return StatusCode::SUCCESS;
}


static inline bool passCellTimeCut(const CaloCell * pCell, const float threshold)
//Copied from the standard algorithm.
//Could possibly make better use of available info
//in the point where this gets executed,
//but I hope the compiler is smart enough to figure it out.
{
  // get the cell time to cut on (the same as in CaloEvent/CaloCluster.h)

  // need sampling number already for time
  CaloSampling::CaloSample sam = pCell->caloDDE()->getSampling();
  // check for unknown sampling
  if (sam != CaloSampling::PreSamplerB && sam != CaloSampling::PreSamplerE && sam != CaloSampling::Unknown)
    {
      const unsigned pmask = pCell->caloDDE()->is_tile() ? 0x8080 : 0x2000;
      //0x2000 is used to tell that time and quality information are available for this channel
      //(from TWiki: https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/CaloEventDataModel#The_Raw_Data_Model)
      // Is time defined?
      if (pCell->provenance() & pmask)
        {
          return std::abs(pCell->time()) < threshold;
        }
    }
  return true;
}

StatusCode BasicEventDataGPUExporter::convert(const EventContext & ctx,
                                              const ConstantDataHolder & cd,
                                              const xAOD::CaloClusterContainer * cluster_collection,
                                              EventDataHolder & ed) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  ed.m_cell_info.allocate();

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::RECOVERABLE;
    }
  for (int i = 0; i < NCaloCells; ++i)
    {
      ed.m_cell_info->energy[i] = 0;
      ed.m_cell_info->gain[i] = 0;

      GainConversion::mark_invalid_cell(ed.m_cell_info->gain[i]);
    }

  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  const CaloNoise * noise_tool = *noise_handle;

  for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells)
    {
      const CaloCell * cell = (*iCells);

      const int index = m_calo_id->calo_cell_hash(cell->ID());

      const float energy = cell->energy();

      const int gain = static_cast<int>(GainConversion::from_standard_gain(cell->gain()));

      ed.m_cell_info->energy[index] = energy;
      ed.m_cell_info->gain[index] = gain;

      if (CaloBadCellHelper::isBad(cell, m_treatL1PredictedCellsAsGood))
        {
          GainConversion::mark_bad_cell(ed.m_cell_info->gain[index]);

          continue;
        }

      if (m_cutCellsInTime)
        {
          if (!passCellTimeCut(cell, m_timeThreshold))
            {
              const float snr = energy / (m_useCPUStoredNoise ? cd.m_cell_noise->noise[gain][index] : noise_tool->getNoise(index, cell->gain()));

              if (std::abs(snr) >= m_seedThreshold)
                //We'll not worry about the cases with negative snr here
                //since it's reasonable to expect these cells to be excluded
                //from clustering anyway in the non-abs case, so no worries
                //excluding them from yet another perspective...
                {
                  if (!m_keepSignificantCells || snr <= m_thresholdForKeeping)
                    //If snr > m_thresholdForKeeping and m_keepSignificantCells is true,
                    //we still keep it as seed so the energy keeps the original value!
                    {
                      if (!m_excludeCutSeedsFromClustering)
                        //If m_excludeCutSeedsFromClustering is false,
                        //seed cells are still kept as candidates for neighbour or terminal.
                        {
                          GainConversion::mark_invalid_seed_cell(ed.m_cell_info->gain[index]);
                        }
                      else
                        {
                          GainConversion::mark_invalid_cell(ed.m_cell_info->gain[index]);
                        }
                    }
                }
            }
        }
    }

  const auto post_cells = clock_type::now();

  if (cluster_collection->size() > 0)
    {
      ed.m_clusters.allocate();
      ed.m_cell_state.allocate();

      for (int i = 0; i < NCaloCells; ++i)
        {
          ed.m_cell_state->clusterTag[i] = Tags::InvalidTag;
        }

      const auto cluster_end = cluster_collection->end();
      auto cluster_iter = cluster_collection->begin();

      for (int cluster_number = 0; cluster_iter != cluster_end; ++cluster_iter, ++cluster_number )
        {
          const xAOD::CaloCluster * cluster = (*cluster_iter);
          const CaloClusterCellLink * cell_links = cluster->getCellLinks();
          if (!cell_links)
            {
              ATH_MSG_ERROR("Can't get valid links to CaloCells (CaloClusterCellLink)!");
              return StatusCode::FAILURE;
            }

          ed.m_clusters->clusterEnergy[cluster_number] = cluster->e();
          ed.m_clusters->clusterEt[cluster_number] = cluster->et();
          ed.m_clusters->clusterEta[cluster_number] = cluster->eta();
          ed.m_clusters->clusterPhi[cluster_number] = cluster->phi();

          const int seed_cell_index = m_calo_id->calo_cell_hash(cluster->cell_begin()->ID());

          ed.m_clusters->seedCellID[cluster_number] = seed_cell_index;

          const tag_type tag = Tags::make_seed_tag(0x7f7fffff, seed_cell_index, cluster_number);
          //0x7f7fffff is the largest possible valid (finite non-NaN) floating point value.

          for (const CaloCell * cell : (*cell_links))
            {
              IdentifierHash tmpHashid = m_calo_id->calo_cell_hash(cell->ID());
              const int cell_ID = tmpHashid;
              ed.m_cell_state->clusterTag[cell_ID] = tag;
            }
        }

      ed.m_clusters->number = cluster_collection->size();
    }

  const auto post_clusters = clock_type::now();
  
  const bool has_cluster_info = cluster_collection->size() > 0;

  ed.sendToGPU(!m_keepCPUData, has_cluster_info, has_cluster_info, false);

  const auto post_send = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, post_cells),
                   time_cast(post_cells, post_clusters),
                   time_cast(post_clusters, post_send)
                  );
    }

  return StatusCode::SUCCESS;

}


StatusCode BasicEventDataGPUExporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Cells Clusters Transfer_to_GPU", 3);
    }
  return StatusCode::SUCCESS;
}


BasicEventDataGPUExporter::~BasicEventDataGPUExporter()
{
  //Nothing!
}
