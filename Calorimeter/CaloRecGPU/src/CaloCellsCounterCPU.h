//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_CALOCELLSCOUNTERCPU_H
#define CALORECGPU_CALOCELLSCOUNTERCPU_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRec/CaloClusterCollectionProcessor.h"
#include <string>

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloConditions/CaloNoise.h"

#include "CLHEP/Units/SystemOfUnits.h"

class CaloCell_ID;

/**
 * @class CaloCellsCounterCPU
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 20 July 2022
 * @brief Outputs counts of cells, both by type and cluster presence, with each cluster being identified by its cell with the largest signal-to-noise ratio.
 */

class CaloCellsCounterCPU :
  public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  CaloCellsCounterCPU(const std::string & type, const std::string & name, const IInterface * parent);
  
  using CaloClusterCollectionProcessor::execute;
  
  virtual StatusCode initialize() override;
  
  virtual StatusCode execute (const EventContext& ctx, xAOD::CaloClusterContainer* cluster_collection) const override;

  virtual ~CaloCellsCounterCPU();

 private:


  /**
   * @brief The path specifying the folder to which the files should be saved.
     Default @p ./cell_counts
   */
  Gaudi::Property<std::string> m_savePath{this, "SavePath", "./cell_counts", "Path to where the files should be saved"};

  /**
   * @brief The prefix of the saved files. Empty string by default.
   */
  Gaudi::Property<std::string> m_filePrefix{this, "FilePrefix", "", "Prefix of the saved files"};

  /**
   * @brief The suffix of the saved files. Empty string by default.
   */
  Gaudi::Property<std::string> m_fileSuffix{this, "FileSuffix", "", "Suffix of the saved files"};

  /**
   * @brief The number of digits to reserve for the events. 9 by default.
   */
  Gaudi::Property<unsigned int> m_numWidth{this, "NumberWidth", 9, "The number of digits to reserve for the events"};
  
    /**
   * @brief vector of names of the cell containers to use as input.
   */
  Gaudi::Property<SG::ReadHandleKey<CaloCellContainer>> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};
  
  /** @brief Key of the CaloNoise Conditions data object. Typical values
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default)

      */
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};
  
  
  /**
   * if set to true, time cut is applied to seed cells, no cut otherwise
   */
  Gaudi::Property<bool> m_cutCellsInTime {this, "SeedCutsInT", false, "Do seed cuts in time"};

  /**
   * threshold used for timing cut on seed cells. Implemented as |seed_cell_time|<m_timeThreshold. No such cut on neighbouring cells.*/
  Gaudi::Property<float> m_timeThreshold {this, "SeedThresholdOnTAbs", 12.5 * CLHEP::ns, "Time thresholds (in abs. val.)"};

  /**
   * upper limit on the energy significance, for applying the cell time cut */
  Gaudi::Property<float> m_thresholdForKeeping {this, "TimeCutUpperLimit", 20., "Significance upper limit for applying time cut"};


  /**
   * @brief if set to true treat cells with a dead OTX which can be
   * predicted by L1 trigger info as good instead of bad cells */
  Gaudi::Property<bool> m_treatL1PredictedCellsAsGood {this, "TreatL1PredictedCellsAsGood", true, "Treat bad cells with dead OTX if predicted from L1 as good"};

  /**
   * @brief if set to true, seed cells failing the time cut are also excluded from cluster at all
   */
  Gaudi::Property<bool> m_excludeCutSeedsFromClustering {this, "CutOOTseed", true, "Exclude out-of-time seeds from neighbouring and cell stage"};

  /**
   * @brief if set to true, the time cut is not applied on cell of large significance
   */
  Gaudi::Property<bool> m_keepSignificantCells {this, "UseTimeCutUpperLimit", false, "Do not apply time cut on cells of large significance"};

  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_seedThreshold {this, "SeedThresholdOnEorAbsEinSigma", 4., "Seed threshold (in units of noise Sigma)"};
  
  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_growThreshold {this, "NeighborThresholdOnEorAbsEinSigma", 2., "Neighbor (grow) threshold (in units of noise Sigma)"};
  
  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_cellThreshold {this, "CellThresholdOnEorAbsEinSigma", 0., "Cell (terminal) threshold (in units of noise Sigma)"};
  
  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID* m_calo_id{nullptr};

  //TO DO: Consider non-absolute SNR...

};

#endif //CALORECGPU_CALOCELLSCOUNTERCPU_H
