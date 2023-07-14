/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GenParticleFinalStateFilter.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// class header include
#include "GenParticleFinalStateFilter.h"

// TruthUtils include
#include "TruthUtils/HepMCHelpers.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"


/** Constructor **/
ISF::GenParticleFinalStateFilter::GenParticleFinalStateFilter( const std::string& t,
                                                               const std::string& n,
                                                               const IInterface* p )
  : base_class(t,n,p),
    m_checkGenSimStable(true),
    m_checkGenInteracting(true)
{
    // different options
    declareProperty("CheckGenSimStable",        m_checkGenSimStable);
    declareProperty("CheckGenInteracting",      m_checkGenInteracting);
}


// Athena algtool's Hooks
StatusCode  ISF::GenParticleFinalStateFilter::initialize()
{
    ATH_MSG_VERBOSE("Initializing ...");
    return StatusCode::SUCCESS;
}


/** returns true if the the particle is considered stable */
#ifdef HEPMC3
bool ISF::GenParticleFinalStateFilter::pass(const HepMC::ConstGenParticlePtr& particle) const
{
  bool passFilter = true;
  passFilter &= MC::isFinalState(particle);
  passFilter &= (!m_checkGenSimStable)   || MC::isSimStable(particle);
  passFilter &= (!m_checkGenInteracting) || MC::isSimInteracting(particle);
  return passFilter;
}

#else
bool ISF::GenParticleFinalStateFilter::pass(const HepMC::GenParticle& particle) const
{
  bool passFilter = true;
  passFilter &= MC::isFinalState(&particle);
  passFilter &= (!m_checkGenSimStable)   || MC::isSimStable(&particle);
  passFilter &= (!m_checkGenInteracting) || MC::isSimInteracting(&particle);

  return passFilter;
}
#endif


StatusCode  ISF::GenParticleFinalStateFilter::finalize()
{
    ATH_MSG_VERBOSE("Finalizing ...");
    return StatusCode::SUCCESS;
}

