/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloGPUOutput.h"

#include "CaloRecGPU/StandaloneDataIO.h"

#include <unordered_map>

using namespace CaloRecGPU;

CaloGPUOutput::CaloGPUOutput(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  m_constantDataSaved(false)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode CaloGPUOutput::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const
{
  if (!m_constantDataSaved)
    {
      std::lock_guard<std::mutex> lock_guard(m_mutex);
      if (!m_constantDataSaved)
        {
          const auto err1 = StandaloneDataIO::save_constants_to_folder(std::string(m_savePath), constant_data.m_geometry,
                            constant_data.m_cell_noise, m_filePrefix, m_fileSuffix);
          if (err1 != StandaloneDataIO::ErrorState::OK)
            {
              return StatusCode::FAILURE;
            }
          m_constantDataSaved = true;
        }
    }

  Helpers::CPU_object<CellInfoArr> cell_info(event_data.m_cell_info_dev);
  Helpers::CPU_object<CellStateArr> cell_state(event_data.m_cell_state_dev);
  Helpers::CPU_object<ClusterInfoArr> clusters(event_data.m_clusters_dev);

  std::unordered_map<int, int> tag_map;

  if (m_sortedAndCutClusters)
    {
      std::vector<int> cluster_order(clusters->number);

      std::iota(cluster_order.begin(), cluster_order.end(), 0);

      std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
      {
        if (clusters->seedCellID[a] < 0)
          {
            return false;
            //This means that clusters with no cells
            //(marked as invalid) always compare lower,
            //so they appear in the end.
          }
        else if (clusters->seedCellID[b] < 0)
          {
            return true;
          }
        return clusters->clusterEt[a] > clusters->clusterEt[b];
      } );

      int real_cluster_numbers = clusters->number;

      for (size_t i = 0; i < cluster_order.size(); ++i)
        {
          const int this_id = cluster_order[i];
          if ( clusters->seedCellID[this_id] < 0 ||
               (m_cutClustersInAbsE ?
                std::abs(clusters->clusterEt[this_id]) :
                clusters->clusterEt[this_id])             < m_clusterETThreshold )
            {
              tag_map[this_id] = -1;
              --real_cluster_numbers;
            }
          else
            {
              tag_map[this_id] = i;
            }
        }

      const Helpers::CPU_object<ClusterInfoArr> temp_clusters(clusters);

      clusters->number = real_cluster_numbers;

      for (int i = 0; i < temp_clusters->number; ++i)
        {
          clusters->clusterEnergy[i] = temp_clusters->clusterEnergy[cluster_order[i]];
          clusters->clusterEt[i] = temp_clusters->clusterEt[cluster_order[i]];
          clusters->clusterEta[i] = temp_clusters->clusterEta[cluster_order[i]];
          clusters->clusterPhi[i] = temp_clusters->clusterPhi[cluster_order[i]];
          clusters->seedCellID[i] = temp_clusters->seedCellID[cluster_order[i]];
        }

    }
  for (int i = 0; i < NCaloCells; ++i)
    {
      const tag_type this_tag = cell_state->clusterTag[i];
      if (!Tags::is_part_of_cluster(this_tag) || GainConversion::is_invalid_cell(cell_info->gain[i]))
        {
          cell_state->clusterTag[i] = Tags::InvalidTag;
        }
      else if (Tags::is_part_of_cluster(this_tag) && m_sortedAndCutClusters)
        {
          const int old_idx = Tags::get_index_from_tag(this_tag);
          const int new_idx = tag_map[old_idx];
          if (new_idx < 0)
            {
              cell_state->clusterTag[i] = Tags::InvalidTag;
            }
          else
            {
              cell_state->clusterTag[i] = Tags::make_seed_tag(0x7f7fffff, clusters->seedCellID[new_idx], new_idx);
              //To match what we do on the CPU side...
            }
        }
    }

  const auto err2 =  StandaloneDataIO::save_event_to_folder(ctx.evt(), std::string(m_savePath), cell_info, cell_state, clusters,
                     m_filePrefix, m_fileSuffix, m_numWidth);

  if (err2 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  return StatusCode::SUCCESS;


}


CaloGPUOutput::~CaloGPUOutput()
{
  //Nothing!
}