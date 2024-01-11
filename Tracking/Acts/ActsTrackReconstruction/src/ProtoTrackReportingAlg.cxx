/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ProtoTrackReportingAlg.h"
#include "xAODEventInfo/EventInfo.h"


ActsTrk::ProtoTrackReportingAlg::ProtoTrackReportingAlg( const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){
}

StatusCode ActsTrk::ProtoTrackReportingAlg::initialize() {
  ATH_CHECK(m_EFTracks.initialize()); 
  ATH_CHECK(m_xAODTracks.initialize()); 
  

  return StatusCode::SUCCESS;
}


StatusCode ActsTrk::ProtoTrackReportingAlg::execute(const EventContext & ctx) const {  

  auto trackContainerHandle = SG::makeHandle(m_EFTracks, ctx);
  if (!trackContainerHandle.isValid())
  {
    ATH_MSG_FATAL("Failed to read EF track collection with key " << m_EFTracks.key());
    return StatusCode::FAILURE;
  } 

  // Print the numberr of candidates we found in the Trk and the xAOD containers, as well as the 
  // properties of the xAOD candidates. 
  ATH_MSG_INFO("I found the track collection, with "<<trackContainerHandle->size()<<" entries"); 


  auto xAODTrackHandle = SG::makeHandle(m_xAODTracks, ctx);
  if (!xAODTrackHandle.isValid())
  {
    ATH_MSG_FATAL("Failed to read xAOD EF track collection with key " << m_xAODTracks.key());
    return StatusCode::FAILURE;
  } 
  ATH_MSG_INFO("I found the xAOD track collection, with "<<xAODTrackHandle->size()<<" entries"); 
  for (const xAOD::TrackParticle* t : *xAODTrackHandle){
    ATH_MSG_INFO("-----------------------------"); 
    auto param = t->perigeeParameters();
    ATH_MSG_INFO("xAOD Track param: d0 "<<t->d0()<<" z0 "<<t->z0()<<" pt "<<t->pt()<<" eta "<<t->eta()<<" phi "<<t->phi()); 

  }


  return StatusCode::SUCCESS;
}


