/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_BASICGPUTOATHENAIMPORTER_H
#define CALORECGPU_BASICGPUTOATHENAIMPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"

class CaloCell_ID;

/**
 * @class BasicGPUToAthenaImporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 30 May 2022
 * @brief Standard tool to convert the GPU data representation back to CPU.
 *
 *
 */

class BasicGPUToAthenaImporter :
  public AthAlgTool, virtual public ICaloClusterGPUOutputTransformer, public CaloGPUTimed
{
 public:

  BasicGPUToAthenaImporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (const EventContext & ctx, const ConstantDataHolder & constant_data,
                              EventDataHolder & event_data, xAOD::CaloClusterContainer * cluster_collection) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicGPUToAthenaImporter();

 private:

  /** @brief If @p true, do not delete the GPU data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepGPUData {this, "KeepGPUData", true, "Keep GPU allocated data"};

  /**
  * @brief if set to true, cluster properties are (re-)calculated using @p CaloClusterKineHelper::calculateKine.
  * Else, the GPU-calculated values are used. Default is @p false.
  */
  Gaudi::Property<bool> m_useCPUPropertiesCalculation {this, "UseCPUClusterPropertiesCalculation", false, "Use CaloClusterKineHelper::calculateKine instead of GPU-calculated cluster properties"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  Gaudi::Property<SG::ReadHandleKey<CaloCellContainer>> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /**
  * @brief if set to @p true cluster cuts are on \f$|E|_\perp\f$, if @p false on \f$E_\perp\f$. Default is @p true.
  *
  */
  Gaudi::Property<bool> m_cutClustersInAbsE {this, "ClusterCutsInAbsE", true, "Do cluster cuts in Abs E instead of E"};

  /**
   * @brief \f$E_\perp\f$ cut on the clusters.
   *
   * The clusters have to pass this cut (which is on \f$E_\perp\f$
   * or \f$|E|_\perp\f$ of the cluster depending on the above switch)
   * in order to be inserted into the CaloClusterContainer.  */

  Gaudi::Property<float> m_clusterETThreshold {this, "ClusterEtorAbsEtCut", 0.*CLHEP::MeV, "Cluster E_t or Abs E_t cut"};

  /// Cluster size. Set accordingly to the threshold.
  Gaudi::Property<std::string> m_clusterSizeString{this, "ClusterSize", "Topo_420", "The size/type of the clusters"};

  xAOD::CaloCluster::ClusterSize m_clusterSize;

  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID* m_calo_id{nullptr};
};

#endif //CALORECGPU_BASICGPUTOATHENAIMPORTER_H
