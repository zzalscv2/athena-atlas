/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// ----------------------------------------------------------------------
// Generators/QGSJet.cxx
//
// Description: Allows the user to generate crmc (model qgsjet) events and store the result
// in the Transient Store.
//
// AuthorList:
//   Sami tyKama:       Initial code.
//   Sebastian Piec:  Adaptation for Epos 1.99.crmc.r2790.
// ----------------------------------------------------------------------

#include "GaudiKernel/MsgStream.h"
#include "CLHEP/Random/RandFlat.h"
#include "AthenaKernel/RNGWrapper.h"

#include "AtlasHepMC/IO_HEPEVT.h"


#include "QGSJet_i/QGSJet.h"



namespace{
  static std::string qgsjet_rndm_stream = "QGSJET_INIT";
  static CLHEP::HepRandomEngine* p_rndmEngine{};
}

extern "C" double atl_qgsjet_rndm_( int* )
{
  return CLHEP::RandFlat::shoot(p_rndmEngine);
}

// ----------------------------------------------------------------------
// QGSJet Fortran bindings.
// ----------------------------------------------------------------------
extern "C"
{
    // generator initialization
 void crmc_set_f_(int &nEvents,int &iSeed,double &beamMomentum, double &targetMomentum, int &primaryParticle, int &targetParticle, int &model, int &itab, int &ilheout, const char *paramFile);
  void crmc_init_f_();
  //  void crmc_init_f_( int &iSeed, double &beamMomentum, double &targetMomentum, int &primaryParticle, int &targetParticle, int &model, const char *paramFile );
    // event generation
  void crmc_f_( int &iout, int &ievent, int &nParticles, double &impactParam, int &partPdg,
                double &partPx, double &partPy, double &partPz, double &partEnergy, double &partMass, int &outstat );

    // cross section info
  void crmc_xsection_f_(double &xsigtot, double &xsigine, double &xsigela, double &xsigdd,
      double &xsigsd, double &xsloela, double &xsigtotaa, double &xsigineaa, double &xsigelaaa);

#ifdef HEPMC3
extern struct eposhepevt
{
    int        nevhep;
    int        nhep;
    int        isthep[HEPEVT_EntriesAllocation];
    int        idhep [HEPEVT_EntriesAllocation];
    int        jmohep[HEPEVT_EntriesAllocation][2];
    int        jdahep[HEPEVT_EntriesAllocation][2];
    double     phep  [HEPEVT_EntriesAllocation][5];
    double     vhep  [HEPEVT_EntriesAllocation][4];
} hepevt_;
struct hepmc3hepevt
{
    int        nevhep;
    int        nhep;
    int        isthep[10000];
    int        idhep [10000];
    int        jmohep[10000][2];
    int        jdahep[10000][2];
    double     phep  [10000][5];
    double     vhep  [10000][4];
} localhepevt_;
#endif

}
extern "C"
{
  extern struct
  {
    float sigtot;
    float sigcut;
    float sigela;
    float sloela;
    float sigsd;
    float sigine;
    float sigdif;
    float sigineaa;
    float sigtotaa;
    float sigelaaa;
    float sigcutaa;
  } hadr5_; //crmc-aaa.f
}

extern "C"
{
  extern struct
  {
    // nevt .......... error code. 1=valid event, 0=invalid event
    // bimevt ........ absolute value of impact parameter
    // phievt ........ angle of impact parameter
    // kolevt ........ number of collisions
    // koievt ........ number of inelastic collisions
    // pmxevt ........ reference momentum
    // egyevt ........ pp cm energy (hadron) or string energy (lepton)
    // npjevt ........ number of primary projectile participants
    // ntgevt ........ number of primary target participants
    // npnevt ........ number of primary projectile neutron spectators
    // nppevt ........ number of primary projectile proton spectators
    // ntnevt ........ number of primary target neutron spectators
    // ntpevt ........ number of primary target proton spectators
    // jpnevt ........ number of absolute projectile neutron spectators
    // jppevt ........ number of absolute projectile proton spectators
    // jtnevt ........ number of absolute target neutron spectators
    // jtpevt ........ number of absolute target proton spectators
    // xbjevt ........ bjorken x for dis
    // qsqevt ........ q**2 for dis
    // sigtot ........ total cross section
    // nglevt ........ number of collisions acc to  Glauber
    // zppevt ........ average Z-parton-proj
    // zptevt ........ average Z-parton-targ
    // ng1evt ........ number of Glauber participants with at least one IAs
    // ng2evt ........ number of Glauber participants with at least two IAs
    // ikoevt ........ number of elementary parton-parton scatterings
    // typevt ........ type of event (1=Non Diff, 2=Double Diff, 3=Single Diff
    float phievt;
    int nevt;
    float bimevt;
    int kolevt;
    int koievt;
    float pmxevt;
    float egyevt;
    int npjevt;
    int ntgevt;
    int npnevt;
    int nppevt;
    int ntnevt;
    int ntpevt;
    int jpnevt;
    int jppevt;
    int jtnevt;
    int jtpevt;
    float xbjevt;
    float qsqevt;
    int nglevt;
    float zppevt;
    float zptevt;
    int minfra;
    int maxfra;
    int kohevt;
  } cevt_; //qgsjet.inc
}

extern "C"
{
  extern struct
  {
    int ng1evt;
    int ng2evt;
    float rglevt;
    float sglevt;
    float eglevt;
    float fglevt;
    int ikoevt;
    float typevt;
  } c2evt_; //qgsjet.inc
}


// ----------------------------------------------------------------------
QGSJet::QGSJet( const std::string &name, ISvcLocator *pSvcLocator ):
  GenModule( name, pSvcLocator )
{
  qgsjet_rndm_stream = "QGSJET_INIT";

  // initialize internally used arrays
  m_partID.resize (kMaxParticles);
  m_partPx.resize (kMaxParticles);
  m_partPy.resize (kMaxParticles);
  m_partPz.resize (kMaxParticles);
  m_partEnergy.resize (kMaxParticles);
  m_partMass.resize (kMaxParticles);
  m_partStat.resize (kMaxParticles);
  for (size_t i = 0; i < kMaxParticles; ++i) {
    m_partID[i] = 0;
    m_partPx[i] = m_partPy[i] = m_partPz[i] = m_partEnergy[i] = m_partMass[i] = 0.0;
    m_partStat[i] = 0;
  }

}

// ----------------------------------------------------------------------
StatusCode QGSJet::genInitialize()
{
  ATH_MSG_INFO( " CRMC INITIALISING.\n" );

  p_rndmEngine = getRandomEngineDuringInitialize(qgsjet_rndm_stream, m_randomSeed, m_dsid); // NOT THREAD-SAFE
  const long *sip = p_rndmEngine->getSeeds();
  long int si1 = sip[0];
  long int si2 = sip[1];

  // eA

  std::cout << "eA seed: " << si1 << " " << si2 << std::endl;

  int iSeed = si1%1000000000;     // FIXME ?

  // set up initial values

  std::cout << "parameters " << m_nEvents << " " << iSeed << " " << m_beamMomentum << " " << m_targetMomentum << " " << m_primaryParticle << " " << m_targetParticle << " " << m_model << " " << m_itab << " " << m_ilheout << " " <<  m_lheout.value().c_str()<< " " <<  m_paramFile.value().c_str() << std::endl;

//  crmc_set_f_(m_nEvents, iSeed, m_beamMomentum, m_targetMomentum, m_primaryParticle, m_targetParticle, m_model, m_itab, m_ilheout,  m_paramFile.c_str() );

  crmc_set_f_(m_nEvents.value(), iSeed, m_beamMomentum.value(), m_targetMomentum.value(), m_primaryParticle.value(), m_targetParticle.value(), m_model.value(), m_itab.value(), m_ilheout.value(), (m_paramFile.value() + " ").c_str() );

    // initialize QGSJet
  //  crmc_init_f_( iSeed, m_beamMomentum, m_targetMomentum, m_primaryParticle, m_targetParticle, m_model, m_paramFile.c_str() );
  crmc_init_f_();

  qgsjet_rndm_stream = "QGSJet";

    // setup HepMC
#ifdef HEPMC3
    /// Inlined
    HepMC::HEPEVT_Wrapper::set_hepevt_address((char*)(&localhepevt_));
#else
    HepMC::HEPEVT_Wrapper::set_sizeof_int(sizeof( int ));
    HepMC::HEPEVT_Wrapper::set_sizeof_real( 8 );
    HepMC::HEPEVT_Wrapper::set_max_number_entries(kMaxParticles);
#endif

  m_events = 0;


 return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode QGSJet::callGenerator()
{
  // ATH_MSG_INFO( " QGSJet Generating." );

  //Re-seed the random number stream
  long seeds[7];
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ATHRNG::calculateSeedsMC21(seeds, qgsjet_rndm_stream, ctx.eventID().event_number(), m_dsid, m_randomSeed);
  p_rndmEngine->setSeeds(seeds, 0); // NOT THREAD-SAFE

  // save the random number seeds in the event
  const long *s = p_rndmEngine->getSeeds();

  std:: cout << "eA seed s : " << s[0] << " " << s[1] << std::endl;
  m_seeds.clear();
  m_seeds.push_back(s[0]);
  m_seeds.push_back(s[1]);

   ++m_events;

    // as in crmchep.h
  int nParticles = 0;
  double impactParameter = -1.0;

    // generate event
  crmc_f_( m_iout, m_ievent ,nParticles, impactParameter, m_partID[0], m_partPx[0], m_partPy[0], m_partPz[0],
           m_partEnergy[0], m_partMass[0], m_partStat[0]  );

  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode QGSJet::genFinalize()
{
  ATH_MSG_INFO("QGSJet finalizing.");

  std::cout << "MetaData: generator = QGSJet " << std::endl;

    // retrieve information about the total cross-section from QGSJet
  double xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa;
  xsigtot = xsigine = xsigela = xsigdd = xsigsd = xsloela = xsigtotaa = xsigineaa = xsigelaaa = 0.0;

  crmc_xsection_f_(xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa);

  xsigtot *= 1000000;         // [mb] to [nb] conversion
  std::cout << "MetaData: cross-section (nb) = " << xsigtot << std::endl;
  xsigine *= 1000000;        //[mb] to [nb] conversion
  std::cout << "MetaData: cross-section inelastic (cut + projectile diffraction)[nb] = " << xsigine << std::endl;
     xsigela *= 1000000;         // [mb] to [nb] conversion
  std::cout << "MetaData: cross-section elastic (includes target diffraction)[nb] = " << xsigela << std::endl;
  xsigdd *= 1000000;         // [mb] to [nb] conversion
  std::cout << "MetaData: cross-section dd (nb) = " << xsigdd << std::endl;
  xsigsd *= 1000000;         // [mb] to [nb] conversion
  std::cout << "MetaData: cross-section sd (nb) = " << xsigsd << std::endl;

  //  m_qgsjetEventInfo.close();

  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode QGSJet::fillEvt( HepMC::GenEvent* evt )
{
  //  ATH_MSG_INFO( " QGSJet Filling.\n" );

    // debug printout


  HepMC::HEPEVT_Wrapper::set_event_number(m_events);
#ifdef HEPMC3
  ///If HepMC3 has been compiled with different block size than is used in the interface,
  /// only the inlined functions can be used without restrictions.
  /// The convert functions are compiled and should operate on the block of matching size.
  /// The best solution would be to define a single block sze for all Athena.
  localhepevt_.nevhep = m_events;
  localhepevt_.nhep = std::min(10000, hepevt_.nhep);
   for (int i = 0; i < localhepevt_.nhep; i++ ) {
    localhepevt_.isthep[i] = hepevt_.isthep[i];
    localhepevt_.idhep [i] = hepevt_.idhep [i];
    for (int k = 0; k < 2; k++) localhepevt_.jmohep[i][k] = hepevt_.jmohep[i][k];
    for (int k = 0; k < 2; k++) localhepevt_.jdahep[i][k] = hepevt_.jdahep[i][k];
    for (int k = 0; k < 5; k++) localhepevt_.phep  [i][k] = hepevt_.phep  [i][k];
    for (int k = 0; k < 4; k++) localhepevt_.vhep  [i][k] = hepevt_.vhep  [i][k];
    localhepevt_.jmohep[i][1] = std::max(localhepevt_.jmohep[i][0],localhepevt_.jmohep[i][1]);
    localhepevt_.jdahep[i][1] = std::max(localhepevt_.jdahep[i][0],localhepevt_.jdahep[i][1]);
    /// For some interesting reason EPOS marks beam particle parents as -1 -1
    if (localhepevt_.jmohep[i][0] <= 0 && localhepevt_.jmohep[i][1] <= 0 )
    {
      localhepevt_.jmohep[i][0] = 0;
      localhepevt_.jmohep[i][1] = 0;
      localhepevt_.isthep[i] = 4;
    }
   }
  /// Compiled!
  HepMC::HEPEVT_Wrapper::HEPEVT_to_GenEvent(evt);
#else
  HepMC::IO_HEPEVT hepio;


  hepio.set_trust_mothers_before_daughters(0);
  hepio.set_print_inconsistency_errors(0);
  hepio.fill_next_event(evt);
#endif
  HepMC::set_random_states(evt, m_seeds );

  evt->weights().push_back(1.0);

//correct units
//uncomment to list HepMC events
#ifdef HEPMC3
evt->set_units(HepMC3::Units::MEV, HepMC3::Units::MM);
//    std::cout << " print::listing QGSJet " << std::endl;
//    HepMC3::Print::listing(std::cout, *evt);
#else
GeVToMeV(evt);
//    std::cout << " print::printing QGSJet " << std::endl;
//    evt->print();
#endif

  std::vector<HepMC::GenParticlePtr> beams;

  for (auto p: *evt) {
    if (p->status() == 4) {
      beams.push_back(p);
    }
  }

 if (beams.size()>=2){
  evt->set_beam_particles(beams[0], beams[1]);
 }

   int sig_id = -1;
   switch (int(c2evt_.typevt))
    {
    case  1: sig_id = 101; break;
    case -1: sig_id = 101; break;
    case  2: sig_id = 105; break;
    case -2: sig_id = 105; break;
    case  3: sig_id = 102; break;
    case -3: sig_id = 102; break;
    case  4: sig_id = 103; break;
    case -4: sig_id = 104; break;
    default: ATH_MSG_INFO( "Signal ID not recognised for setting HEPEVT \n");
    }

  HepMC::set_signal_process_id(evt,sig_id);

  double xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa;
  xsigtot = xsigine = xsigela = xsigdd = xsigsd = xsloela = xsigtotaa = xsigineaa = xsigelaaa = 0.0;
  crmc_xsection_f_(xsigtot, xsigine, xsigela, xsigdd, xsigsd, xsloela, xsigtotaa, xsigineaa, xsigelaaa);
  xsigtot *= 1000000;         // [mb] to [nb] conversion
#ifdef HEPMC3
  std::shared_ptr<HepMC3::GenCrossSection> xsec = std::make_shared<HepMC3::GenCrossSection>();
  xsec->set_cross_section(xsigine, 0.0);
#else
  HepMC::GenCrossSection xsec;
  xsec.set_cross_section(xsigine, 0.0);
#endif
  evt->set_cross_section(xsec);

 return StatusCode::SUCCESS;
}
