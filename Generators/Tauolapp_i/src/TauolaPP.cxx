/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorObjects/McEventCollection.h"
#include "Tauolapp_i/TauolaPP.h"

// Tauola header files
#include "Tauola/Log.h"
#include "Tauola/Tauola.h"
#ifdef HEPMC3
#include "Tauola/TauolaHepMC3Event.h"
namespace Tauolapp
{
using TauolaHepMCEvent=TauolaHepMC3Event;
using TauolaHepMCParticle=TauolaHepMC3Particle;
}
#else
#include "Tauola/TauolaHepMCEvent.h"
#endif

#include "Tauola/f_Variables.h"

// for proper seeding
#include "CLHEP/Random/RandFlat.h"
#include "AthenaKernel/RNGWrapper.h"

// Pointer to random engine
CLHEP::HepRandomEngine*  TauolaPP::p_rndmEngine = nullptr;

double AthenaRandomGenerator ATLAS_NOT_THREAD_SAFE ()
{
  return CLHEP::RandFlat::shoot(TauolaPP::p_rndmEngine);
}

// Constructor
TauolaPP::TauolaPP(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
  //Key to HepMC record
  declareProperty("McEventKey", m_key="GEN_EVENT");

  //TAUOLA configurables
  //TAUOLA decay mode of particles with same/opposite charge as "decay_particle"
  declareProperty("decay_mode_same", m_decay_mode_same=1);
  declareProperty("decay_mode_opposite", m_decay_mode_opp=2);
  declareProperty("decay_particle",m_decay_particle=15);
  declareProperty("tau_mass",m_tau_mass=1.77684);
  declareProperty("spin_correlation",m_spin_correlation=true);
  declareProperty("setRadiation",m_setRadiation=true);
  declareProperty("setRadiationCutOff",m_setRadiationCutOff=0.01); 

}


StatusCode TauolaPP::initialize(){

  ATH_CHECK(m_rndmSvc.retrieve());
  p_rndmEngine = getRandomEngineDuringInitialize("TAUOLAPP_INIT", m_randomSeed, m_dsid);
  const long*  sip  =  p_rndmEngine->getSeeds();

  // Setup and intialise Tauola Interface
  using Tauolapp::Tauola;
  Tauola::setSameParticleDecayMode(m_decay_mode_same);
  Tauola::setOppositeParticleDecayMode(m_decay_mode_opp);
  // etc.... see Tauola.h for the full list of configurables
  // Note: some need to be set before (or after) calling Tauola::initialize();

  // Tauola::setHiggsScalarPseudoscalarMixingAngle(atof(argv[5]));
  // Tauola::setHiggsScalarPseudoscalarPDG(25);

  Tauola::initialize();
 
  Tauola::setEtaK0sPi(1,0,1); // switches to decay eta K0_S and pi0 1/0 on/off.
  Tauola::spin_correlation.setAll(m_spin_correlation);
  Tauola::setRadiation(m_setRadiation);
  Tauola::setRadiationCutOff(m_setRadiationCutOff);

  //call RanLux generator for ++ part of Tauola
  Tauola::setRandomGenerator(AthenaRandomGenerator);

  //seeding tauola-fortran generator
  // See tauola.f: the first parameter should be positive int <900000000
  Tauola::setSeed(int(std::abs(sip[0])%(900000000)),0,0);

  //setting tau mass
  Tauolapp::parmas_.amtau=m_tau_mass;

  return StatusCode::SUCCESS;
}


void TauolaPP::reseedRandomEngine(const std::string& streamName, const EventContext& ctx)
{
  long seeds[7];
  ATHRNG::calculateSeedsMC21(seeds, streamName,  ctx.eventID().event_number(), m_dsid, m_randomSeed);
  p_rndmEngine->setSeeds(seeds, 0); // NOT THREAD-SAFE
}

CLHEP::HepRandomEngine* TauolaPP::getRandomEngine(const std::string& streamName, unsigned long int randomSeedOffset,
                                                             const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  rngWrapper->setSeed( streamName, ctx.slot(), randomSeedOffset, ctx.eventID().run_number() );
  return rngWrapper->getEngine(ctx);
}


CLHEP::HepRandomEngine* TauolaPP::getRandomEngineDuringInitialize(const std::string& streamName, unsigned long int randomSeedOffset, unsigned int conditionsRun, unsigned int lbn) const
{
  const size_t slot=0;
  EventContext ctx;
  ctx.setSlot( slot );
  ctx.setEventID (EventIDBase (conditionsRun,
               EventIDBase::UNDEFEVT,  // event
               EventIDBase::UNDEFNUM,  // timestamp
               EventIDBase::UNDEFNUM,  // timestamp ns
               lbn));
  Atlas::setExtendedEventContext(ctx,
                                 Atlas::ExtendedEventContext( evtStore()->hiveProxyDict(),
                                                              conditionsRun) );
  return getRandomEngine(streamName, randomSeedOffset, ctx);
}


StatusCode TauolaPP::execute() {

  //Re-seed the random number stream
  const EventContext& ctx = Gaudi::Hive::currentContext();
  reseedRandomEngine("TAUOLAPP", ctx);

  // Load HepMC info
  // FIXME should be using Read/WriteHandles here
  const McEventCollection* mcCollptr_const;
  ATH_CHECK( evtStore()->retrieve(mcCollptr_const, m_key) );
  // Const_cast to make an event possible to update
  McEventCollection* mcCollptr =  const_cast<McEventCollection*>(mcCollptr_const);

  // Loop over all events in McEventCollection
  for (HepMC::GenEvent* evt : *mcCollptr) {
    // Convert event record to format readable by tauola interface
    auto t_event = new Tauolapp::TauolaHepMCEvent(evt);

#ifdef HEPMC3
//move to GeV
//comment out for this version, as it causes problems (we may need it for a new official version)
//    for (auto p: t_event->getEvent()->particles()) {
//        p->set_momentum(p->momentum()*1.0/1000);
//        p->set_generated_mass(1.0/1000* p->generated_mass());}
    // remove tau decays first
      t_event->undecayTaus();
    // decay taus
      t_event->decayTaus();
// move back to MeV
//    for (auto p: t_event->getEvent()->particles()) {
//        p->set_momentum(p->momentum()*1000);
//        p->set_generated_mass(1000* p->generated_mass());}

// for event listing uncomment the line below
//    HepMC3::Print::listing(std::cout, *(t_event->getEvent()));

#else

    // remove tau decays first
      t_event->undecayTaus();
    // decay taus
      t_event->decayTaus();
    // t_event->getEvent()->print();
#endif

  }

  return StatusCode::SUCCESS;
}
