/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCellsCounterCPU.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/StandaloneDataIO.h"
#include "StoreGate/DataHandle.h"
#include "CaloRec/CaloBadCellHelper.h"

#include <map>

using namespace CaloRecGPU;

CaloCellsCounterCPU::CaloCellsCounterCPU(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
}


StatusCode CaloCellsCounterCPU::initialize()
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

StatusCode CaloCellsCounterCPU::execute (const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::RECOVERABLE;
    }


  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  const CaloNoise * noise_tool = *noise_handle;

  unsigned int gain_counts[GainConversion::num_gain_values()] = {0};

  size_struct global_counts, global_cluster_counts;

  for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells)
    {
      const CaloCell * cell = (*iCells);

      const float energy = cell->energy();

      const float SNR = std::abs( energy / noise_tool->getNoise(m_calo_id->calo_cell_hash(cell->ID()), cell->gain()) );

      int gain = static_cast<int>(GainConversion::from_standard_gain(cell->gain()));

      if (CaloBadCellHelper::isBad(cell, m_treatL1PredictedCellsAsGood))
        {
          GainConversion::mark_bad_cell(gain);
        }

      if (m_cutCellsInTime && !GainConversion::is_bad_cell(gain))
        {
          if (!passCellTimeCut(cell, m_timeThreshold))
            {
              if (std::abs(SNR) >= m_seedThreshold)
                //We'll not worry about the cases with negative snr here
                //since it's reasonable to expect these cells to be excluded
                //from clustering anyway in the non-abs case, so no worries
                //excluding them from yet another perspective...
                {
                  if (!m_keepSignificantCells || SNR <= m_thresholdForKeeping)
                    //If SNR > m_thresholdForKeeping and m_keepSignificantCells is true,
                    //we still keep it as seed so the energy keeps the original value!
                    {
                      if (!m_excludeCutSeedsFromClustering)
                        //If m_excludeCutSeedsFromClustering is false,
                        //seed cells are still kept as candidates for neighbour or terminal.
                        {
                          GainConversion::mark_invalid_seed_cell(gain);
                        }
                      else
                        {
                          GainConversion::mark_invalid_cell(gain);
                        }
                    }
                }
            }
        }

      ++gain_counts[gain - GainConversion::min_gain_value()];

      if (GainConversion::is_normal_cell(gain))
        {
          if (SNR > m_seedThreshold)
            {
              ++global_counts.seed;
            }
          else if (SNR > m_growThreshold)
            {
              ++global_counts.grow;
            }
          else if (SNR > m_cellThreshold)
            {
              ++global_counts.term;
            }
          else
            {
              ++global_counts.invalid;
            }
        }
      else
        {
          ++global_counts.bad;
        }
    }

  std::map<IdentifierHash, size_struct> cluster_sizes;

  for (const xAOD::CaloCluster * cluster : *cluster_collection)
    {
      //const xAOD::CaloCluster * cluster = (*cluster_iter);
      const CaloClusterCellLink * cell_links = cluster->getCellLinks();
      if (!cell_links)
        {
          ATH_MSG_ERROR("Can't get valid links to CaloCells (CaloClusterCellLink)!");
          return StatusCode::FAILURE;
        }

      float max_snr = -1;
      IdentifierHash max_snr_hash;

      size_struct num_cells;

      for (const CaloCell * cell : (*cell_links))
        {
          if (cell == nullptr)
            {
              continue;
            }


          const float this_snr = std::abs(cell->energy()) / noise_tool->getNoise(m_calo_id->calo_cell_hash(cell->ID()), cell->gain());

          if (this_snr > max_snr)
            {
              max_snr = this_snr;
              max_snr_hash = m_calo_id->calo_cell_hash(cell->ID());
            }

          int gain = static_cast<int>(GainConversion::from_standard_gain(cell->gain()));

          if (CaloBadCellHelper::isBad(cell, m_treatL1PredictedCellsAsGood))
            {
              GainConversion::mark_bad_cell(gain);
            }

          if (m_cutCellsInTime && !GainConversion::is_bad_cell(gain))
            {
              if (!passCellTimeCut(cell, m_timeThreshold))
                {
                  if (std::abs(this_snr) >= m_seedThreshold)
                    //We'll not worry about the cases with negative snr here
                    //since it's reasonable to expect these cells to be excluded
                    //from clustering anyway in the non-abs case, so no worries
                    //excluding them from yet another perspective...
                    {
                      if (!m_keepSignificantCells || this_snr <= m_thresholdForKeeping)
                        //If SNR > m_thresholdForKeeping and m_keepSignificantCells is true,
                        //we still keep it as seed so the energy keeps the original value!
                        {
                          if (!m_excludeCutSeedsFromClustering)
                            //If m_excludeCutSeedsFromClustering is false,
                            //seed cells are still kept as candidates for neighbour or terminal.
                            {
                              GainConversion::mark_invalid_seed_cell(gain);
                            }
                          else
                            {
                              GainConversion::mark_invalid_cell(gain);
                            }
                        }
                    }
                }
            }

          if (GainConversion::is_normal_cell(gain))
            {
              if (this_snr > m_seedThreshold)
                {
                  ++global_cluster_counts.seed;
                  ++num_cells.seed;
                }
              else if (this_snr > m_growThreshold)
                {
                  ++global_cluster_counts.grow;
                  ++num_cells.grow;
                }
              else if (this_snr > m_cellThreshold)
                {
                  ++global_cluster_counts.term;
                  ++num_cells.term;
                }
              else
                {
                  ++global_cluster_counts.invalid;
                  ++num_cells.invalid;
                }
            }
          else
            {
              ++global_cluster_counts.bad;
              ++num_cells.bad;
            }

          ++num_cells.total;
        }
      cluster_sizes[max_snr_hash] = num_cells;
    }

  const auto err1 = StandaloneDataIO::prepare_folder_for_output(std::string(m_savePath));
  if (err1 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  const boost::filesystem::path save_file = m_savePath + "/" + StandaloneDataIO::build_filename((m_filePrefix.size() > 0 ? m_filePrefix + "_counts" : "counts"),
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


CaloCellsCounterCPU::~CaloCellsCounterCPU()
{
  //Nothing!
}
