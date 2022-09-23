/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
#define CALORECGPU_BASICEVENTDATAGPUEXPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloConditions/CaloNoise.h"

class CaloCell_ID;

/**
 * @class BasicEventDataGPUExporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 29 May 2022
 * @brief Standard tool to export cell energy and gain to the GPU.
 *
 * @warning The two gaussian noise for evaluating the seed cell cuts
 * based on out of time seeds is not implemented! We currently assume
 * noise as estimated without that flag.
 *
 */

class BasicEventDataGPUExporter :
  public AthAlgTool, virtual public ICaloClusterGPUInputTransformer, public CaloGPUTimed
{
 public:

  BasicEventDataGPUExporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;
  
  virtual StatusCode convert (const EventContext & ctx, const ConstantDataHolder & constant_data,
                              const xAOD::CaloClusterContainer * cluster_collection, EventDataHolder & event_data) const override;

  virtual StatusCode finalize() override;
  
  virtual ~BasicEventDataGPUExporter();

 private:

  /** @brief If @p true, do not delete the CPU version of the GPU-friendly data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepCPUData {this, "KeepCPUData", true, "Keep CPU version of GPU data format"};


  /** @brief If @p true and the noise is necessary to exclude out of time seeds,
   *  reads the noise from the values kept in CPU memory inside the @p ConstantDataHolder
   *  Reads from the usual Athena CaloNoise if @p false.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_useCPUStoredNoise {this, "UseCPUStoredNoise", true, "If necessary to use noise to exclude OOT seeds, use the CPU equivalent of the GPU stored noise rather than going through the normal interfaces"};


  /** @brief Key of the CaloNoise Conditions data object. Typical values
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default)

      Really only used if @p m_useCPUStoredNoise is @p false.

      */
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  Gaudi::Property<SG::ReadHandleKey<CaloCellContainer>> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

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

  /** @brief Value to consider for the threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure seed cell time cuts work properly.
   */
  Gaudi::Property<float> m_seedThreshold {this, "SeedThresholdOnEorAbsEinSigma", 4., "Seed threshold (in units of noise Sigma)"};

  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID* m_calo_id{nullptr};
};

#endif //CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
