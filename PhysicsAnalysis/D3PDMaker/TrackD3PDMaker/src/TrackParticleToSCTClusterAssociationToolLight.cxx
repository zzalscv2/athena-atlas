/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackParticleToSCTClusterAssociationToolLight.h"

#include <Particle/TrackParticle.h>
#include "TrkTrack/Track.h"
#include <TrkTrack/TrackStateOnSurface.h>
#include "AthenaKernel/errorcheck.h"
#include "Identifier/Identifier.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "AtlasDetDescr/AtlasDetectorID.h"


namespace D3PD {


TrackParticleToSCTClusterAssociationToolLight::TrackParticleToSCTClusterAssociationToolLight
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : Base (type, name, parent),
      m_idHelper(0)
{
}

StatusCode TrackParticleToSCTClusterAssociationToolLight::initialize(){

  CHECK( Base::initialize() );

  // need Atlas id-helper to identify sub-detectors, take it from detStore
  if (detStore()->retrieve(m_idHelper, "AtlasID").isFailure()) {
    REPORT_MESSAGE (MSG::ERROR) << "Could not get AtlasID helper";
    return StatusCode::FAILURE;
  }   

  return StatusCode::SUCCESS;
}

/**
 * @brief Start the iteration for a new association.
 * @param p The object from which to associate.
 */
StatusCode TrackParticleToSCTClusterAssociationToolLight::reset (const Rec::TrackParticle& p)
{

  sctClusterForAssociation.clear();

  const Trk::Track *track = p.originalTrack();
  if(!track){
    m_clusItr = m_clusEnd;
    //REPORT_MESSAGE (MSG::DEBUG) << "Could not access original track";
    return StatusCode::SUCCESS;
  }

  const DataVector<const Trk::TrackStateOnSurface> *TSOs = 0;


  TSOs = track->trackStateOnSurfaces();
  

  if(!TSOs){
    m_clusItr = m_clusEnd;
    REPORT_MESSAGE (MSG::WARNING) << "Could not access track state on surfaces";
    return StatusCode::SUCCESS;
  }

  DataVector<const Trk::TrackStateOnSurface>::const_iterator it = TSOs->begin();
  for( ; it!=TSOs->end(); it++){

    const InDet::SiClusterOnTrack *clus = dynamic_cast<const InDet::SiClusterOnTrack*>((*it)->measurementOnTrack());

    if (clus){
  
      // Get Pointer to prepRawDataObject	
      const InDet::SCT_Cluster *RawDataClus = dynamic_cast<const InDet::SCT_Cluster*>(clus->prepRawData());

      if (RawDataClus!=0){
	if (RawDataClus->detectorElement()->isSCT()){
	
	  sctClusterForAssociation.push_back(*it);

	}
      }
    } // if (clus)
  }
 
  m_clusItr = sctClusterForAssociation.begin(); //sct::vector<cont InDet::SCT_Cluster>::const_iterator 
  m_clusEnd = sctClusterForAssociation.end(); 

  return StatusCode::SUCCESS;
}

/**
 * @brief Return a pointer to the next element in the association.
 *
 * Return 0 when the association has been exhausted.
 */
const Trk::TrackStateOnSurface* TrackParticleToSCTClusterAssociationToolLight::next()
{

  // reached the end
  if(m_clusItr == m_clusEnd) return 0;

  const Trk::TrackStateOnSurface *TSO = (*m_clusItr);
  m_clusItr++;

  return TSO;
}



} // namespace D3PD
