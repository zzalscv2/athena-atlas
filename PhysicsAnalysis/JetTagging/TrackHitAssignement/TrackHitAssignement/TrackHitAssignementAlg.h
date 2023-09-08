/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKHITASSIGNEMENT_TRACKHITASSIGNEMENTALG_H
#define TRACKHITASSIGNEMENT_TRACKHITASSIGNEMENTALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "AthLinks/ElementLink.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "StoreGate/WriteDecorHandle.h"


class TrackHitAssignementAlg: public AthReentrantAlgorithm { 
 public: 
  TrackHitAssignementAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~TrackHitAssignementAlg(); 

                                        //IS EXECUTED:
  virtual StatusCode  initialize() override;     //once, before any input is loaded
  virtual StatusCode  execute(const EventContext& context) const override;        //per event
  
 private: 

  SG::ReadHandleKey<xAOD::TrackParticleContainer>   m_tracks{this,"TrackParticles", "InDetTrackParticles", "Track Particles"};
  SG::ReadHandleKey<TrackCollection>   m_TrackCollection{this,"TrackCollection", "CombinedInDetTracks", "InnerDetTracks"};

  SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer>   m_JetPixelCluster{this,"JetAssociatedPixelClusters", "JetAssociatedPixelClusters", "PixelClusters"};
  SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer>   m_JetSCTCluster{this,"JetAssociatedSCTClusters", "JetAssociatedSCTClusters", "SCTClusters"};

  SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_JetPixelClusterHits{this,"JetAssociatedPixelClustersHits", m_JetPixelCluster, "HitContainedInTrack"};
  SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_JetSCTClusterHits{this,"JetAssociatedSCTClustersHits", m_JetSCTCluster, "HitContainedInTrack"};
  SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_JetPixelClusterTrackAssocs{this,"JetAssociatedPixelClustersTrackAssocs", m_JetPixelCluster, "HitToTrackLinks"};
  SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_JetSCTClusterTrackAssocs{this,"JetAssociatedSCTClustersTrackAssocs", m_JetSCTCluster, "HitToTrackLinks"};

}; 

#endif //> !TRACKHITASSIGNEMENT_TRACKHITASSIGNEMENTALG_H
