/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKPARTICLEASSOCIATIONALGS_TRACKPARTICLECELLASSOCIATIONALG_H
#define TRACKPARTICLEASSOCIATIONALGS_TRACKPARTICLECELLASSOCIATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODAssociations/TrackParticleClusterAssociation.h"
#include "xAODAssociations/TrackParticleClusterAssociationContainer.h"
#include "xAODAssociations/TrackParticleClusterAssociationAuxContainer.h"


#include <string>
namespace Rec {
  class IParticleCaloCellAssociationTool;
}

class TrackParticleCellAssociationAlg : public AthReentrantAlgorithm
{
 public:
  TrackParticleCellAssociationAlg(const std::string& name, ISvcLocator* pSvcLocator);

  ~TrackParticleCellAssociationAlg();

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;

 private:

  ToolHandle<Rec::IParticleCaloCellAssociationTool> m_caloCellAssociationTool;

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleCollectionName{this,"TrackParticleContainerName", 
      "InDetTrackParticles","SG Key of track particle container"};

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_clusterContainerName{this,"ClusterContainerName", 
      "InDetTrackParticlesAssociatedClusters","SG Key of output cluster container"};

  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_clusterCellLinkName{this,"CaloClusterCellLinkName",
      "InDetTrackParticlesAssociatedClusters_links","SG Key of out CaloCluserCellLInkContainer"};


  SG::WriteHandleKey< xAOD::TrackParticleClusterAssociationContainer> m_associationContainerName{this,"AssociationContainerName",
      "InDetTrackParticlesClusterAssociations","SG Key of association container"};

  double m_ptCut;
};


#endif
