/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCPUOutput.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/StandaloneDataIO.h"
#include "StoreGate/DataHandle.h"
#include "CaloRec/CaloBadCellHelper.h"

using namespace CaloRecGPU;

CaloCPUOutput::CaloCPUOutput(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
}


StatusCode CaloCPUOutput::initialize()
{
  ATH_CHECK( m_noiseCDOKey.initialize() );
  ATH_CHECK( m_cellsKey.value().initialize() );
  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );
  return StatusCode::SUCCESS;
}

static inline bool passCellTimeCut(const CaloCell * pCell, float threshold)
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

StatusCode CaloCPUOutput::execute (const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{

  Helpers::CPU_object<CellInfoArr> ret_info;
  Helpers::CPU_object<CellStateArr> ret_state;
  Helpers::CPU_object<ClusterInfoArr> ret_clusts;

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::RECOVERABLE;
    }


  ret_info.allocate();
  ret_state.allocate();
  ret_clusts.allocate();

  for (int i = 0; i < NCaloCells; ++i)
    {
      ret_info->energy[i] = 0;
      ret_info->gain[i] = 0;

      GainConversion::mark_invalid_cell(ret_info->gain[i]);
    }

  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  const CaloNoise * noise_tool = *noise_handle;

  for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells)
    {
      const CaloCell * cell = (*iCells);

      const int index = m_calo_id->calo_cell_hash(cell->ID());

      const float energy = cell->energy();

      const int gain = static_cast<int>(GainConversion::from_standard_gain(cell->gain()));

      ret_state->clusterTag[index] = Tags::InvalidTag;
      ret_info->energy[index] = energy;
      ret_info->gain[index] = gain;

      if (CaloBadCellHelper::isBad(cell, m_treatL1PredictedCellsAsGood))
        {
          GainConversion::mark_bad_cell(ret_info->gain[index]);

          continue;
        }

      if (m_cutCellsInTime)
        {
          if (!passCellTimeCut(cell, m_timeThreshold))
            {
              const float snr = energy / noise_tool->getNoise(index, cell->gain());

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
                          GainConversion::mark_invalid_seed_cell(ret_info->gain[index]);
                        }
                      else
                        {
                          GainConversion::mark_invalid_cell(ret_info->gain[index]);
                        }
                    }
                }
            }
        }
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

      //--- fill data ---
      ret_clusts->clusterEnergy[cluster_number] = cluster->e();
      ret_clusts->clusterEt[cluster_number] = cluster->et();
      ret_clusts->clusterEta[cluster_number] = cluster->eta();
      ret_clusts->clusterPhi[cluster_number] = cluster->phi();

      const int seed_cell_index = m_calo_id->calo_cell_hash(cluster->cell_begin()->ID());

      ret_clusts->seedCellID[cluster_number] = seed_cell_index;

      const tag_type tag = Tags::make_seed_tag(0x7f7fffff, seed_cell_index, cluster_number);
      //0x7f7fffff is the largest possible valid (finite non-NaN) floating point value.

      for (const CaloCell * cell : (*cell_links))
        {
          if (cell == nullptr)
            {
              continue;
            }
          IdentifierHash tmpHashid = m_calo_id->calo_cell_hash(cell->ID());
          const int cell_ID = tmpHashid;
          ret_state->clusterTag[cell_ID] = tag;
        }
    }

  ret_clusts->number = cluster_collection->size();

  const auto err =  StandaloneDataIO::save_event_to_folder(ctx.evt(), std::string(m_savePath), ret_info, ret_state, ret_clusts,
                    m_filePrefix, m_fileSuffix, m_numWidth);

  if (err != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  return StatusCode::SUCCESS;

}


CaloCPUOutput::~CaloCPUOutput()
{
  //Nothing!
}
