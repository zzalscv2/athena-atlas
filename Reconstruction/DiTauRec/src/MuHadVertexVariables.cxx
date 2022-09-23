/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "Particle/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "TrkVertexFitters/AdaptiveVertexFitter.h"
#include "TrkVxEdmCnv/IVxCandidateXAODVertex.h"
#include "TrkSurfaces/PerigeeSurface.h"

#include "tauRecTools/TauEventData.h"
#include "DiTauRec/MuHadVertexVariables.h"


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

const float MuHadVertexVariables::DEFAULT = -1111.;

MuHadVertexVariables::MuHadVertexVariables(const std::string &name ) :
  TauRecToolBase(name),
  m_fitTool("Trk::AdaptiveVertexFitter"),
  m_SeedFinder("Trk::CrossDistancesSeedFinder"),
  m_xaodConverter("Trk::VxCandidateXAODVertex"),
  m_pSecVtxContainer(nullptr),
  m_pSecVtxAuxContainer(nullptr){
  declareProperty("TrackToVertexIPEstimator", m_trackToVertexIPEstimator);
  declareProperty("VertexFitter", m_fitTool);
  declareProperty("SeedFinder", m_SeedFinder);
  declareProperty("XAODConverter",m_xaodConverter);
}

MuHadVertexVariables::~MuHadVertexVariables() {}

//-----------------------------------------------------------------------------
// Initializer
//-----------------------------------------------------------------------------

StatusCode MuHadVertexVariables::initialize() {
  CHECK( m_trackToVertexIPEstimator.retrieve() );
  CHECK( m_fitTool.retrieve() );
  CHECK( m_SeedFinder.retrieve() );

  return StatusCode::SUCCESS;
}

StatusCode MuHadVertexVariables::eventInitialize() {

      // Secondary Vertex Container for tau decay vertex
  m_pSecVtxContainer = new xAOD::VertexContainer();
  m_pSecVtxAuxContainer = new xAOD::VertexAuxContainer();
  m_pSecVtxContainer->setStore( m_pSecVtxAuxContainer );
      
  CHECK( evtStore()->record( m_pSecVtxContainer, "MuRmTauSecondaryVertices" ) );
  CHECK( evtStore()->record( m_pSecVtxAuxContainer, "MuRmTauSecondaryVerticesAux." ) );

  return StatusCode::SUCCESS;

}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------
StatusCode MuHadVertexVariables::execute(xAOD::TauJet& pTau) {

  ATH_MSG_DEBUG("executing MuHadVertexVariables");

  pTau.setDetail(xAOD::TauJetParameters::ipSigLeadTrk,  DEFAULT ) ;
  pTau.setDetail(xAOD::TauJetParameters::ipZ0SinThetaSigLeadTrk, DEFAULT  );
  pTau.setDetail(xAOD::TauJetParameters::trFlightPathSig, DEFAULT ) ;

  // for tau trigger
  bool inTrigger = tauEventData()->inTrigger();
  if (inTrigger) ATH_MSG_DEBUG("We're in the Trigger");
	
  // impact parameter variables for standard tracks
  if (pTau.nTracks() > 0) 
  {

    const Trk::ImpactParametersAndSigma* myIPandSigma = nullptr ;
    const xAOD::Vertex* vxcand = nullptr;
    float d0 = DEFAULT , sd0 = 1. , z0 = DEFAULT, szst = 1.;  
    if (inTrigger) {
	  
      StatusCode scBeam = StatusCode::FAILURE;
	  
      if (tauEventData()->hasObject("Beamspot")) scBeam = tauEventData()->getObject("Beamspot", vxcand);

      if(scBeam){
	myIPandSigma = m_trackToVertexIPEstimator->estimate( pTau.track(0)->track(), vxcand);
      } else {
	ATH_MSG_DEBUG("No Beamspot object in tau candidate");
      }
	  
    }  else 
    { 
      if (pTau.vertexLink()) vxcand = *(pTau.vertexLink()) ;

      //check if vertex has a valid type (skip if vertex has type NoVtx)
      if (vxcand->vertexType() > 0)
	myIPandSigma = m_trackToVertexIPEstimator->estimate(pTau.track(0)->track(), vxcand);
    }

    if (myIPandSigma != 0) {
      d0 = myIPandSigma->IPd0 ;
      sd0 = myIPandSigma->sigmad0 ;
      z0 = myIPandSigma->IPz0SinTheta ;
      szst = myIPandSigma->sigmaz0SinTheta ;
      delete myIPandSigma;
    }  

    pTau.setDetail( xAOD::TauJetParameters::ipSigLeadTrk, static_cast<float>( d0/sd0 ) );
    pTau.setDetail(xAOD::TauJetParameters::ipZ0SinThetaSigLeadTrk, static_cast<float>( z0/szst ) );
  } else 
    ATH_MSG_DEBUG("Tau has no tracks");


  //try to find secondary vertex
  //look for secondary vertex if more than 1 track
  if (pTau.nTracks() < 2) {
    return StatusCode::SUCCESS;
  }
  // get xAOD TrackParticles and Trk::Tracks
  std::vector<const xAOD::TrackParticle*> xaodTracks;
  std::vector<const Trk::TrackParameters*> perigeeList;
  std::vector<const Trk::Track*> origTracks;

  for ( const xAOD::TauTrack* ttrk : pTau.tracks()  ) 
  {
    const xAOD::TrackParticle* xaodTP = ttrk->track() ;

    if ( xaodTP == nullptr ) { continue ; }

    xaodTracks.push_back( xaodTP ) ;
    const Trk::Perigee * tmpMeasPer= &( xaodTP->perigeeParameters() );

    if ( tmpMeasPer != nullptr ) 
    {
      perigeeList.push_back( tmpMeasPer ) ;
    }
    ATH_MSG_VERBOSE("xAOD::TrackParticle " << xaodTP->pt() <<" "<< xaodTP->eta() <<" "<< xaodTP->phi() );
    if ( xaodTP ) 
    {
      // only if running on ESD 
      const Trk::Track* esd = xaodTP->track() ;
      if ( esd != nullptr ) origTracks.push_back( esd ) ;
    }
  }

  Amg::Vector3D seedPoint = origTracks.size() > 0 ? m_SeedFinder->findSeed( origTracks ) : m_SeedFinder->findSeed( perigeeList ) ;

  ATH_MSG_DEBUG("seedPoint x/y/perp=" << seedPoint.x() <<  " " << seedPoint.y() << " "<< seedPoint.z() << " " << TMath::Sqrt(seedPoint.x()*seedPoint.x()+seedPoint.y()+seedPoint.y()));

  // fitting the vertex itself
  xAOD::Vertex* xAODvertex = nullptr ;

  xAODvertex = m_fitTool->fit( xaodTracks, seedPoint);
  if ( xAODvertex ) {
    ATH_MSG_VERBOSE("using new xAOD API: Secondary Vertex found and recorded! x="<<xAODvertex->position().x()<< ", y="<<xAODvertex->position().y()<<", perp="<<xAODvertex->position().perp());
    m_pSecVtxContainer->push_back(xAODvertex);
    xAODvertex->setVertexType(xAOD::VxType::NotSpecified);
    pTau.setSecondaryVertex( m_pSecVtxContainer, xAODvertex ); 		// set the link to the vertex
  } else 
  {
    ATH_MSG_WARNING("no secondary vertex found!");
    return StatusCode::SUCCESS;
  }

  // get the transverse flight path significance
  float trFlightPS = trFlightPathSig(pTau, *xAODvertex);
  pTau.setDetail(xAOD::TauJetParameters::trFlightPathSig, static_cast<float>(trFlightPS));
  ATH_MSG_VERBOSE("transverse flight path significance="<<trFlightPS);

  return StatusCode::SUCCESS;
}

//-------------------------------------------------------------------------
// calculate the transverse flight path significance
//-------------------------------------------------------------------------
double MuHadVertexVariables::trFlightPathSig(const xAOD::TauJet& pTau, const xAOD::Vertex& secVertex) {

  const xAOD::Vertex* pVertex = nullptr ;
  if (pTau.vertexLink()) pVertex = *pTau.vertexLink();
  if (!pVertex) {
    ATH_MSG_WARNING("No primary vertex information for calculation of transverse flight path significance");
    return DEFAULT ;
  }

  double fpt = (secVertex.position() - pVertex->position()).perp();

  if ( fpt == 0. ) {
    ATH_MSG_WARNING("delta pt of (secVtx - priVtx) is 0!");
    return DEFAULT ;
  }

  double fpx = secVertex.position().x() - pVertex->position().x();
  double fpy = secVertex.position().y() - pVertex->position().y();

  double sigma_fpt2 = (fpx * fpx * secVertex.covariancePosition()(Trk::x, Trk::x) +
		       fpx * fpy * secVertex.covariancePosition()(Trk::x, Trk::y) +
		       fpy * fpx * secVertex.covariancePosition()(Trk::y, Trk::x) +
		       fpy * fpy * secVertex.covariancePosition()(Trk::y, Trk::y)) / (fpt * fpt);

  if (sigma_fpt2 <= 0) {
    ATH_MSG_WARNING("sigma delta pt of (secVtx - priVtx) is 0!");
    return DEFAULT ;
  }

  double sigma_fpt = std::sqrt(sigma_fpt2);  
  double sign = 0;

  if (fpx * pTau.p4().Px() + fpy * pTau.p4().Py() > 0.) sign = 1.;
  else sign = -1.;

  return sign * fpt / sigma_fpt;

}

#endif

