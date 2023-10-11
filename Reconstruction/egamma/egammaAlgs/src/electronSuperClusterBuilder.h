/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_ELECTRONSUPERCLUSTERBUILDER_H
#define EGAMMAALGS_ELECTRONSUPERCLUSTERBUILDER_H

#include "egammaSuperClusterBuilderBase.h"

#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "CaloDetDescr/CaloDetDescrManager.h"

// Fwd declarations
#include "egammaInterfaces/IEMTrackMatchBuilder.h"
#include "egammaRecEvent/egammaRecContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterFwd.h"

#include <string>
#include <vector>

/**
 * @brief Create supercluster under electron hypothesis
 *
 * The algorithm creates superclusters for electrons merging topoclusters.
 * Input containers:
 * - \ref electronSuperClusterBuilder.m_inputEgammaRecContainerKey
 * "InputEgammaRecContainerName" (default=egammaRecCollection): collection of
 * EgammaRec objects to be used
 *
 * Output containers:
 * - \ref electronSuperClusterBuilder.m_electronSuperRecCollectionKey
 * "OutputEgammaRecContainerKey" (default=ElectronSuperRecCollection):
 * collection of EgammaRec objects with the cluster set to be the supercluster
 * - \ref electronSuperClusterBuilder.m_outputElectronSuperClustersKey
 * "SuperClusterCollectionName" (default=ElectronSuperClusters): collection of
 * clusters (the supercluster)
 *
 * The loop is on the clusters of the EgammaRec objects from the input
 * container. Fist, the first cluster is considered as a seed. The cluster seed
 * must pass some selection:
 * - having a second sampling with |eta| not larger than 10
 * - pT (from the sum of the three accordion layer) not below
 *   \ref egammaSuperClusterBuilderBase.m_EtThresholdCut "EtThresholdCut"
 * - having at least one track with minimum number of Pixel and Si hits (
 *  \ref electronSuperClusterBuilder.m_numberOfPixelHits "NumberOfReqPixelHits",
 *  \ref electronSuperClusterBuilder.m_numberOfSiHits "NumberOfReqSiHits")
 *
 * Clusters to be merged in a supercluster are selected using the
 * electronSuperClusterBuilder::searchForSecondaryClusters function. Then the
 * procedure is redone, testing new seeds, for all the other clusters that have
 * not been used to make superclusters. The building of the supercluster is done
 * with egammaSuperClusterBuilderBase::createNewCluster which selects the cells
 * to be used.
 *
 * Add the end, if the property \ref
 * electronSuperClusterBuilder.m_doTrackMatching "doTrackMatching" is true the
 * track matching is redone on top of new superclusters, using the tool
 * configured by the property TrackMatchBuilderTool, by default
 * EMTrackMatchBuilder.
 *
 * \see photonSuperClusterBuilder
 */

using xAOD::EgammaHelpers::summaryValueInt;

class electronSuperClusterBuilder : public egammaSuperClusterBuilderBase
{

public:
  // Constructor/destructor.
  electronSuperClusterBuilder(const std::string& name,
                              ISvcLocator* pSvcLocator);

  // Tool standard routines.
  virtual StatusCode initialize() override final;

private:
  bool egammaRecPassesSelection(const egammaRec *egRec) const override final;
  xAOD::EgammaParameters::EgammaType getEgammaRecType(const egammaRec *egRec) const override final;
  StatusCode redoMatching(const EventContext &ctx, SG::WriteHandle<EgammaRecContainer> &newEgammaRecs) const override final;
  /**
   * @brief Search for secondary clusters
   *
   * @param i: index of the seed cluster
   * @param isUsed: booleand mask of already used cluster
   * @return vector of index of the satellites
   *
   * Only clusters that have not already been used are considered.
   * The satellite is added if:
   * - matches the seed in a narrow window OR
   * - is inside the window for additonal criteria AND matches the same track
   *
   * The first condition is evaluated with
   * egammaSuperClusterBuilderBase.matchesInWindow. The second condition is
   * evaluated using \ref electronSuperClusterBuilder.m_maxDelEtaCells
   * "MaxWindowDelEtaCells" and \ref electronSuperClusterBuilder.m_maxDelPhi
   * "MaxWindowDelPhiCells" and electronSuperClusterBuilder.matchSameTrack
   */
  std::vector<std::size_t> searchForSecondaryClusters(
    const size_t i,
    const EgammaRecContainer*,
    std::vector<bool>& isUsed) const;

  /** @brief Size of maximum search window in eta */
  Gaudi::Property<int> m_maxDelEtaCells{
    this,
    "MaxWindowDelEtaCells",
    5,
    "Size of maximum search window in eta"
  };
  /** @brief Size of maximum search window in phi */
  Gaudi::Property<int> m_maxDelPhiCells{
    this,
    "MaxWindowDelPhiCells",
    12,
    "Size of maximum search window in phi"
  };

  float m_maxDelEta;
  float m_maxDelPhi;
  Gaudi::Property<std::size_t> m_numberOfPixelHits{
    this,
    "NumberOfReqPixelHits",
    0,
    "Number of required pixel hits for electrons"
  };

  Gaudi::Property<std::size_t> m_numberOfSiHits{
    this,
    "NumberOfReqSiHits",
    7,
    "Number of required silicon hits for electrons"
  };

  /** @brief Tool to perform track matching*/
  ToolHandle<IEMTrackMatchBuilder> m_trackMatchBuilder{
    this,
    "TrackMatchBuilderTool",
    "EMTrackMatchBuilder",
    "Tool that matches tracks to egammaRecs"
  };

  /** @brief private member flag to do the track matching */
  Gaudi::Property<bool> m_doTrackMatching{ this,
                                           "doTrackMatching",
                                           true,
                                           "Boolean to do track matching" };
};

#endif
