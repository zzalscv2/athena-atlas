/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// TrackHitAssignement includes
#include "TrackHitAssignement/TrackHitAssignementAlg.h"


TrackHitAssignementAlg::TrackHitAssignementAlg( const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){
}


TrackHitAssignementAlg::~TrackHitAssignementAlg() {}


StatusCode TrackHitAssignementAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK(m_tracks.initialize());
  ATH_CHECK(m_JetPixelCluster.initialize());
  ATH_CHECK(m_JetSCTCluster.initialize());
  ATH_CHECK(m_TrackCollection.initialize());
  ATH_CHECK(m_JetPixelClusterHits.initialize());
  ATH_CHECK(m_JetSCTClusterHits.initialize());
  ATH_CHECK(m_JetPixelClusterTrackAssocs.initialize());
  ATH_CHECK(m_JetSCTClusterTrackAssocs.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TrackHitAssignementAlg::execute(const EventContext& context) const {  
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  //Read needed Containers to decorate
  //and check if valid
  SG::ReadHandle<xAOD::TrackParticleContainer>   tracks{m_tracks, context};
  SG::ReadHandle<xAOD::TrackMeasurementValidationContainer>   JetPixelCluster{m_JetPixelCluster, context};
  SG::ReadHandle<xAOD::TrackMeasurementValidationContainer>   JetSCTCluster{m_JetSCTCluster, context};

  SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> JetPixelClusterHits{m_JetPixelClusterHits, context};
  SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> JetSCTClusterHits{m_JetSCTClusterHits, context};
  SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>>> JetPixelClusterTrackAssocs{m_JetPixelClusterTrackAssocs, context};
  SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>>> JetSCTClusterTrackAssocs{m_JetSCTClusterTrackAssocs, context};

  if(!JetPixelCluster.isValid()){
    ATH_MSG_ERROR ("Couldn't find Pixel");
    return StatusCode::FAILURE;
  }
  if(!JetSCTCluster.isValid()){
    ATH_MSG_ERROR ("Couldn't find SCT");
    return StatusCode::FAILURE;
  }
  if(!tracks.isValid()){
    ATH_MSG_ERROR ("Couldn't find tracks");
    return StatusCode::FAILURE;
  }

  std::multimap<uint64_t, const xAOD::TrackParticle*> track_hits;

  //go from tracks to hits and save the hits used for reconstruction in a map
  for(const auto trk:*tracks){
    auto trkTrk_link = trk->trackLink();
    if (trkTrk_link.isValid()){
      auto trkTrk = *trkTrk_link;
      auto states = trkTrk->trackStateOnSurfaces();
      for(const auto state:*states){
        if (!state->type(Trk::TrackStateOnSurface::Measurement)){
          continue;
        }
        auto hitOnTrack = state->measurementOnTrack();
        if(hitOnTrack->type(Trk::MeasurementBaseType::RIO_OnTrack)){
          const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>( hitOnTrack ) ;
          const Trk::PrepRawData* prd = hit->prepRawData() ;
          Identifier ident_hit = prd->identify();
          track_hits.emplace(ident_hit.get_compact(), trk);
        }
      }
    }
  }


  // run through SCT/Pixel Clusters and check if hit is in map 
  // if yes, add also ElementLink
  typedef std::multimap<uint64_t, const xAOD::TrackParticle*>::iterator MMAPIterator;

  for(const xAOD::TrackMeasurementValidation* hit: *JetSCTCluster){
    std::pair<MMAPIterator, MMAPIterator> result = track_hits.equal_range(hit->identifier());
    int count = std::distance(result.first, result.second);
    std::vector<ElementLink<xAOD::TrackParticleContainer>> vec_ElemLink_SCT;
    if(count == 0){
      JetSCTClusterHits(*hit)=0;
    }
    else{
      JetSCTClusterHits(*hit)=1;
      for (MMAPIterator it = result.first; it != result.second; it++){
        vec_ElemLink_SCT.push_back(ElementLink<xAOD::TrackParticleContainer> ((it->second),*tracks, context));
      }
    }    
    JetSCTClusterTrackAssocs(*hit)=vec_ElemLink_SCT;
  }
  
  for(const xAOD::TrackMeasurementValidation* hit: *JetPixelCluster){
    std::pair<MMAPIterator, MMAPIterator> result = track_hits.equal_range(hit->identifier());
    int count = std::distance(result.first, result.second);
    std::vector<ElementLink<xAOD::TrackParticleContainer>> vec_ElemLink_pix;
    if(count == 0){
      JetPixelClusterHits(*hit)=0;
    }
    else{
      JetPixelClusterHits(*hit)=1;
      for (MMAPIterator it = result.first; it != result.second; it++){
        vec_ElemLink_pix.push_back(ElementLink<xAOD::TrackParticleContainer> ((it->second),*tracks, context));
      }
    }    
    JetPixelClusterTrackAssocs(*hit)=vec_ElemLink_pix; 
  }
  return StatusCode::SUCCESS;
}
