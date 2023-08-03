/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrkExtrapolator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// class header include
#include "TrkExtrapolator.h"


#include "TrkExInterfaces/IExtrapolator.h"

#include "TrkGeometry/TrackingGeometry.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"


/** Constructor **/
ISF::TrkExtrapolator::TrkExtrapolator(const std::string& t, const std::string& n, const IInterface* p)
  : base_class(t,n,p),
    m_extrapolator("Trk::Extrapolator/AtlasExtrapolator"),
    m_trackingVolumeName(""),
    m_trackingVolume(0),
    m_pdgToParticleHypothesis(new Trk::PdgToParticleHypothesis())
{
   declareProperty( "TrackingVolumeName",
         m_trackingVolumeName,
         "Name of the TrackingVolume within which the extrapolation is to be carried out");
   declareProperty( "Extrapolator", 
         m_extrapolator,
         "Extrapolator used for track extrapolation" );
}

/** Destructor **/
ISF::TrkExtrapolator::~TrkExtrapolator()
{
  delete m_pdgToParticleHypothesis;
}


/** Athena AlgTool initialization **/
StatusCode  ISF::TrkExtrapolator::initialize()
{
  ATH_MSG_VERBOSE("initialize() begin");

  StatusCode sc = m_extrapolator.retrieve();
  if (sc.isFailure()){
    ATH_MSG_FATAL( "Could not retrieve " << m_extrapolator );
    return StatusCode::FAILURE;
  }
  // initialize the TrackingGeometryReadKey
  ATH_CHECK(m_trackingGeometryReadKey.initialize());

  ATH_MSG_VERBOSE("initialize() successful");
  return StatusCode::SUCCESS;
}

/** Athena AlgTool finalization **/
StatusCode  ISF::TrkExtrapolator::finalize()
{
  ATH_MSG_VERBOSE("finalize() begin");

  ATH_MSG_VERBOSE("finalize() successful");
  return StatusCode::SUCCESS;
}

/** Extrapolate the given ISFParticle to the given TrackingVolume name */
ISF::ISFParticle* ISF::TrkExtrapolator::extrapolate( const ISF::ISFParticle &particle ) const {

  const EventContext& ctx = Gaudi::Hive::currentContext();
  if( m_extrapolator.empty() ) {
    ATH_MSG_ERROR( "Problem with extrapolator!" );
    return 0;
  }

  if ( !m_trackingVolume.get() ) {
    // ------------------------------- get the trackingGeometry at first place
    SG::ReadCondHandle<Trk::TrackingGeometry> readHandle{m_trackingGeometryReadKey};
    if (!readHandle.isValid() || *readHandle == nullptr) {
      ATH_MSG_ERROR("Problem with TrackingGeometry '" << m_trackingGeometryReadKey.fullKey() << "' from CondStore.");
      return 0;
    }

    const Trk::TrackingGeometry* trackingGeometry = *readHandle;
    const Trk::TrackingVolume* trackingVolume = trackingGeometry->trackingVolume( m_trackingVolumeName);
    if (!trackingVolume) {
      ATH_MSG_FATAL("Failed to retrieve TrackingVolume: " << m_trackingVolumeName);
      return 0;
    }
    m_trackingVolume.set(trackingVolume);
  }
  
  // create objects from ISFParticle needed for extrapolation
  Trk::CurvilinearParameters par(particle.position(),particle.momentum(),particle.charge());
  
  int absPdg   = abs(particle.pdgCode());
  //bool photon   = (absPdg == 22);
  //bool geantino = (absPdg == 999);
  //bool charged  = photon || geantino ? false : (particle.charge()*particle.charge() > 0) ;
  
  Trk::ParticleHypothesis particleHypo = 
       m_pdgToParticleHypothesis->convert(particle.pdgCode(),particle.charge());
  if ( absPdg == 999 ) particleHypo = Trk::geantino;
  
  // extrapolate to calorimeter entry
  const Trk::TrackParameters* extrapolatedPars = m_extrapolator->extrapolateToVolume(ctx,
                                                                                     par,
                                                                                     *m_trackingVolume.get(),
                                                                                     Trk::alongMomentum,
                                                                                     particleHypo).release();
  
  // create a new ISF particle representing the given particle at the extrapolated position
  ISFParticle *extrapolatedParticle = new ISFParticle( extrapolatedPars->position(),
                                                       extrapolatedPars->momentum(),
                                                       particle.mass(),
                                                       particle.charge(),
                                                       particle.pdgCode(),
                                                       particle.status(),
                                                       particle.timeStamp(),
                                                       particle );
  
  // cleanup
  delete extrapolatedPars;

  return extrapolatedParticle;
}

