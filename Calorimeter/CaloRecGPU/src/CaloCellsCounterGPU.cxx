/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCellsCounterGPU.h"
#include "CaloRecGPU/StandaloneDataIO.h"

#include <map>
#include <vector>

using namespace CaloRecGPU;

CaloCellsCounterGPU::CaloCellsCounterGPU(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}


struct size_struct
{
  unsigned int total = 0, seed = 0, grow = 0, term = 0, invalid = 0, bad = 0;
  template <class Str>
  friend Str & operator << (Str & s, const size_struct & sst)
  {
    s << sst.total << " " << sst.seed << " " << sst.grow << " " << sst.term << " " << sst.invalid << " " << sst.bad;
    return s;
  }
};

StatusCode CaloCellsCounterGPU::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const
{
  Helpers::CPU_object<CellNoiseArr> cell_noise(constant_data.m_cell_noise_dev);
  //We could try to keep this in CPU memory by default,
  //but, since it isn't quite needed and we don't mind taking a bit longer for this
  //since we're only debugging, let this be as generic as possible.

  Helpers::CPU_object<CellInfoArr> cell_info(event_data.m_cell_info_dev);
  Helpers::CPU_object<CellStateArr> cell_state(event_data.m_cell_state_dev);
  Helpers::CPU_object<ClusterInfoArr> clusters(event_data.m_clusters_dev);


  unsigned int gain_counts[GainConversion::num_gain_values()] = {0};

  size_struct global_counts, global_cluster_counts;

  std::vector<int> cluster_max_cell(clusters->number, -1);
  std::vector<float> cluster_max_snr(clusters->number, -1);

  std::vector<size_struct> cluster_counts(clusters->number);

  for (int i = 0; i < NCaloCells; ++i)
    {
      const int gain = cell_info->gain[i];
      ++gain_counts[gain - GainConversion::min_gain_value()];


      float SNR = 0.00001f;

      if (GainConversion::is_normal_cell(gain) || GainConversion::is_invalid_seed_cell(gain))
        {
          const int corr_gain = GainConversion::recover_invalid_seed_cell_gain(gain);
          SNR = std::abs( cell_info->energy[i] / cell_noise->noise[corr_gain][i] );
        }

      const tag_type tag = cell_state->clusterTag[i];
      const bool is_cluster = Tags::is_part_of_cluster(tag);

      if (GainConversion::is_normal_cell(gain))
        {
          if (SNR > m_seedThreshold)
            {
              ++global_counts.seed;
              global_cluster_counts.seed += is_cluster;
            }
          else if (SNR > m_growThreshold)
            {
              ++global_counts.grow;
              global_cluster_counts.grow += is_cluster;
            }
          else if (SNR > m_cellThreshold)
            {
              ++global_counts.term;
              global_cluster_counts.term += is_cluster;
            }
          else
            {
              ++global_counts.invalid;
              global_cluster_counts.invalid += is_cluster;
            }
        }
      else
        {
          ++global_counts.bad;
          global_cluster_counts.bad += is_cluster;
        }
      if (is_cluster)
        {
          const int cluster = Tags::get_index_from_tag(tag);
          if (SNR > cluster_max_snr[cluster])
            {
              cluster_max_snr[cluster] = SNR;
              cluster_max_cell[cluster] = i;
            }
          ++cluster_counts[cluster].total;

          if (GainConversion::is_normal_cell(gain))
            {
              if (SNR > m_seedThreshold)
                {
                  ++cluster_counts[cluster].seed;
                }
              else if (SNR > m_growThreshold)
                {
                  ++cluster_counts[cluster].grow;
                }
              else if (SNR > m_cellThreshold)
                {
                  ++cluster_counts[cluster].term;
                }
              else
                {
                  ++cluster_counts[cluster].invalid;
                }
            }
          else
            {
              ++cluster_counts[cluster].bad;
            }
        }
    }

  std::map<int, size_struct> cluster_sizes;

  for (int i = 0; i < clusters->number; ++i)
    {
      if (cluster_max_cell[i] >= 0)
        {
          cluster_sizes[cluster_max_cell[i]] = cluster_counts[i];
        }
    }


  const auto err1 = StandaloneDataIO::prepare_folder_for_output(std::string(m_savePath));
  if (err1 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  const boost::filesystem::path save_file = m_savePath + "/" +  StandaloneDataIO::build_filename((m_filePrefix.size() > 0 ? m_filePrefix + "_counts" : "counts"),
                                                                                               ctx.evt(), m_fileSuffix, "txt", m_numWidth);

  std::ofstream out_file(save_file);

  if (!out_file.is_open())
    {
      return StatusCode::FAILURE;
    }

  out_file << global_counts << "\n\n";
  out_file << global_cluster_counts << "\n\n";
  for (int i = 0; i < GainConversion::num_gain_values(); ++i)
    {
      out_file << gain_counts[i] << " ";
    }
  out_file << "\n\n";
  for (const auto & it : cluster_sizes)
    {
      out_file << it.first << " " << it.second << "\n";
    }
  out_file << std::endl;

  if (!out_file.good())
    {
      return StatusCode::FAILURE;
    }

  out_file.close();

  return StatusCode::SUCCESS;


}


CaloCellsCounterGPU::~CaloCellsCounterGPU()
{
  //Nothing!
}