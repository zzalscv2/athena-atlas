/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// G4HadIntProcessor.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Class Header
#include "ISF_FatrasToolsG4/G4HadIntProcessor.h"

// Fatras
#include "ISF_FatrasInterfaces/IPhysicsValidationTool.h"

// ISF
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Event/ParticleClipboard.h"
#include "ISF_Event/ParticleUserInformation.h"
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Geant4Tools/IG4RunManagerHelper.h"
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_Event/ISFTruthIncident.h"

#include <G4HadronElasticProcess.hh>

// Trk inlcude
#include "TrkGeometry/MaterialProperties.h"

// Geant4
#include "G4Material.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "QGSP_BERT.hh"
#include "QGSP_BIC.hh"
#include "FTFP_BERT.hh"
#include "G4UImanager.hh"
#include "G4NistManager.hh"
#include "G4VEnergyLossProcess.hh"
#include <G4MaterialCutsCouple.hh>
#include <G4HadronInelasticProcess.hh>

#include "globals.hh"
#include "G4CrossSectionDataStore.hh"
#include "G4Element.hh"
#include "G4ElementVector.hh"
#include "G4IsotopeVector.hh"
#include "G4Neutron.hh"
#include "G4ProductionCutsTable.hh"
#include "G4ios.hh"

// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "CLHEP/Random/RandExponential.h"
#include "CLHEP/Random/RandFlat.h"

// STD
#include <math.h>

// ROOT
#include "TTree.h"

namespace {
  /** projection factor for the non-parametric scattering */
  const double s_projectionFactor = sqrt(2.);
}

// constructor
iFatras::G4HadIntProcessor::G4HadIntProcessor(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p),
  m_rndGenSvc("AtDSFMTGenSvc", n),
  m_g4RunManagerHelper("iGeant4::G4RunManagerHelper/G4RunManagerHelper"),
  m_doElastic(false),
  m_hadIntProbScale(1.0),
  m_minMomentum(50.0),
  m_particleBroker("ISF_ParticleBrokerSvc", n),
  m_truthRecordSvc("ISF_ValidationTruthService", n),
  m_randomEngine(0),
  m_randomEngineName("FatrasRnd"),
  m_validationMode(false),
  m_validationTool(""),
  m_validationTreeName("FatrasMaterialEffects"),
  m_validationTreeDescription("Validation output from the McMaterialEffectsUpdator"),
  m_validationTreeFolder("/val/FatrasSimulationMaterial"),
  m_bremValidationTreeName("FatrasBremPhotons"),
  m_bremValidationTreeDescription("Validation output from the McMaterialEffectsUpdator"),
  m_bremValidationTreeFolder("/val/FatrasBremPhotons"),
  m_edValidationTreeName("FatrasEnergyInCaloDeposit"),
  m_edValidationTreeDescription("Validation output from the McMaterialEffectUpdator"),
  m_edValidationTreeFolder("/val/FatrasEnergyInCaloDeposit")
{
  // steering
  declareProperty("MomentumCut"                     , m_minMomentum                                                       );
  declareProperty("DoElasticInteractions"            , m_doElastic                                                       );
  declareProperty("HadronicInteractionScaleFactor"   , m_hadIntProbScale=1.0  , "Scale probability of HadrInteractions"  );
  // ISF Services and Tools
  declareProperty("ParticleBroker"                   , m_particleBroker       , "ISF Particle Broker"                    );
  declareProperty("TruthRecordSvc"                   , m_truthRecordSvc       , "ISF Particle Truth Svc"                 );
  // random number generator
  declareProperty("RandomNumberService"                 , m_rndGenSvc          , "Random number generator");
  declareProperty("RandomStreamName"                    , m_randomEngineName   , "Name of the random number stream");
  declareProperty("ValidationMode"                      , m_validationMode);
  declareProperty("PhysicsValidationTool"               , m_validationTool);
  declareProperty("G4RunManagerHelper"                  , m_g4RunManagerHelper);
}



// destructor
iFatras::G4HadIntProcessor::~G4HadIntProcessor()
{}

// Athena standard methods
// initialize
StatusCode iFatras::G4HadIntProcessor::initialize()
{
  // ISF Services
  if (m_particleBroker.retrieve().isFailure()){
    ATH_MSG_FATAL( "Could not retrieve ParticleBroker: " << m_particleBroker );
    return StatusCode::FAILURE;
  }
  if (m_truthRecordSvc.retrieve().isFailure()){
    ATH_MSG_FATAL( "Could not retrieve TruthRecordSvc: " << m_truthRecordSvc );
    return StatusCode::FAILURE;
  }

  if (m_validationMode){

    // retrieve the physics validation tool
    if (m_validationTool.retrieve().isFailure()){
      ATH_MSG_FATAL( "Could not retrieve " << m_validationTool );
      return StatusCode::FAILURE;
    } else
      ATH_MSG_VERBOSE( "Successfully retrieved " << m_validationTool );
  }

  // get the random generator serice
  if (m_rndGenSvc.retrieve().isFailure()){
    ATH_MSG_FATAL( "Could not retrieve " << m_rndGenSvc );
    return StatusCode::FAILURE;
  } else
    ATH_MSG_VERBOSE( "Successfully retrieved " << m_rndGenSvc );

  //Get own engine with own seeds:
  m_randomEngine = m_rndGenSvc->GetEngine(m_randomEngineName);
  if (!m_randomEngine) {
    ATH_MSG_FATAL( "Could not get random engine '" << m_randomEngineName << "'" );
    return StatusCode::FAILURE;
  }

  // all good
  ATH_MSG_INFO("initialize() successful");
  return StatusCode::SUCCESS;
}


// finalize
StatusCode iFatras::G4HadIntProcessor::finalize()
{
  ATH_MSG_INFO( " ---------- Statistics output -------------------------- " );
  //ATH_MSG_INFO( "                     Minimum energy cut for brem photons : " <<   m_minimumBremPhotonMomentum  );
  //ATH_MSG_INFO( "                     Brem photons (above cut, recorded)  : " <<   m_recordedBremPhotons        );

  ATH_MSG_INFO( "finalize() successful" );
  return StatusCode::SUCCESS;
}


std::map<int,G4VProcess*>::const_iterator iFatras::G4HadIntProcessor::initProcessPDG(int pdg)
{
  ATH_MSG_VERBOSE( "  [ g4sim ] Registering Geant4 processes for particles with pdg code " << pdg );

  // return value
  std::map<int,G4VProcess*>::const_iterator ret;


  G4ParticleDefinition *parDef = G4ParticleTable::GetParticleTable()->FindParticle( pdg);

  // check if everythin is set up properly
  if ( !parDef || !parDef->GetProcessManager() ) {
    ATH_MSG_WARNING( "  [ ---- ] Unable to register particle type with PDG code " << pdg );
    return m_g4HadrInelasticProcesses.end();

  }

  // get Geant4 processes
  G4ProcessVector *physIntVector = parDef->GetProcessManager()->GetPostStepProcessVector(typeGPIL);
  //G4ProcessVector *physIntVector = parDef->GetProcessManager()->GetProcessVector(idxAll);
  //  G4ProcessVector *physIntVector = parDef->GetProcessManager()->GetPostStepProcessVector(typeDoIt);
  if ( !physIntVector) {
    ATH_MSG_WARNING( "  [ ---- ] No Geant4 processes registered for PDG code " << pdg << " particles" );
    return m_g4HadrInelasticProcesses.end();
  }

  // loop over all processes of current particle type
  for( size_t np=0; np < physIntVector->size(); np++) {
    // get current process
    G4VProcess* curProc = (*physIntVector)(np);
    // NULL means the process is inactivated by a user on fly.
    if ( curProc == 0 ) continue;

    ATH_MSG_VERBOSE( "  [ g4sim ] Found Geant4 process " << curProc->GetProcessName());

    G4HadronInelasticProcess *hadInelastic = dynamic_cast<G4HadronInelasticProcess*>( curProc);
    G4HadronElasticProcess *hadElastic = dynamic_cast<G4HadronElasticProcess*>( curProc);
    ATH_MSG_DEBUG( "  hadronic process inelastic,elastic " << hadInelastic << ", " << hadElastic);
    if ( !hadInelastic && !hadElastic) {
      ATH_MSG_VERBOSE( "  [ g4sim ] Current process not an inelastic or elastic  hadronic process -> process not registered" );
      continue;
    }

    if (hadInelastic || hadElastic) {
      //Prepare and build the Physics table for primaries
      curProc->PreparePhysicsTable(*parDef);
      curProc->BuildPhysicsTable(*parDef);
    }

    if(hadInelastic){
      ret = m_g4HadrInelasticProcesses.insert( std::pair<int,G4VProcess*>( pdg, hadInelastic) ).first;
      ATH_MSG_DEBUG( "  [ g4sim ] Registered Geant4 hadronic interaction processes for particles with pdg code " << pdg );
    }
    if(m_doElastic && hadElastic ){
      ret = m_g4HadrElasticProcesses.insert( std::pair<int,G4VProcess*>( pdg, hadElastic) ).first;
      G4ProcessType pType = curProc->GetProcessType();
      ATH_MSG_DEBUG( "  [ g4sim ] Registered Geant4 ELASTIC hadronic interaction processes for particles with pdg code "
		     << pdg << "and process " <<  pType);
    }


  } // process loop

  // return iterator to insterted G4VProcess
  return ret;
}

bool iFatras::G4HadIntProcessor::hadronicInteraction(const Amg::Vector3D& position, const Amg::Vector3D& momentum,
						     double, double, double,
						     const Trk::MaterialProperties& mprop, double pathCorrection,
						     Trk::ParticleHypothesis particle) const
{
  // do not treat geantinos
  if (particle==Trk::geantino) return false;

  bool processSecondaries = true;

  // the layer material
  const Trk::MaterialProperties* ematprop  = dynamic_cast<const Trk::MaterialProperties*>(&(mprop));
  if (!ematprop) {
    ATH_MSG_WARNING("[ ---- ] Unable to cast MaterialProperties->MaterialProperties -> no material interactions for this particle");
    return false;
  }
  ATH_MSG_DEBUG("[ g4sim ] Material t/X0, t/L0 : " << pathCorrection*ematprop->thicknessInX0() << ", " << pathCorrection*ematprop->thicknessInL0() );

  // compute a random interaction- and radiation length,
  // according to the mean values
  double meanIntLength   = pathCorrection*ematprop->l0();
  double rndIntLength    = CLHEP::RandExponential::shoot(m_randomEngine, meanIntLength);
  double thickness       = pathCorrection*ematprop->thickness();

  // test for hadronic interactions
  if ( rndIntLength < thickness ) {
    ATH_MSG_DEBUG(" [ g4sim ] computing hadronic interaction on current particle in current material layer");
    return doHadronicInteraction( 0., position, momentum, &(ematprop->material()), particle, processSecondaries);
  }

  // hadronic interaction did not happen
  return false;
}


StatusCode iFatras::G4HadIntProcessor::initG4RunManager ATLAS_NOT_THREAD_SAFE () {

  ATH_MSG_DEBUG("[ g4sim ] Initializing G4RunManager");

  // Get the G4RunManagerHelper ( no initialization of G4RunManager done )
  ATH_CHECK( m_g4RunManagerHelper.retrieve() );

  G4RunManager* g4runManager = m_g4RunManagerHelper->fastG4RunManager();
  g4runManager->SetVerboseLevel(10);

  initProcessPDG( 211);
  initProcessPDG(-211);
  initProcessPDG(2212);
  initProcessPDG(-2212);
  initProcessPDG(2112);
  initProcessPDG( 130);
  initProcessPDG( 321);
  initProcessPDG( 111);
  initProcessPDG(-321);

  // define the available G4Material
  m_g4Material.clear();

  G4NistManager* G4Nist = G4NistManager::Instance();
  G4Material* air = G4Nist->G4NistManager::FindOrBuildMaterial("G4_AIR");
  if (air) {
    G4MaterialCutsCouple airCuts = G4MaterialCutsCouple(air);
    // airCuts->SetIndex(0);    // ?
    std::pair<G4Material*,G4MaterialCutsCouple> airMat(air,airCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(0.,airMat));
  }
  G4Material* h = G4Nist->G4NistManager::FindOrBuildMaterial("G4_H");
  if (h) {
    G4MaterialCutsCouple hCuts = G4MaterialCutsCouple(h);
    std::pair<G4Material*,G4MaterialCutsCouple> hMat(h,hCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(1.,hMat));
  }
  G4Material* al = G4Nist->G4NistManager::FindOrBuildMaterial("G4_Al");
  if (al) {
    G4MaterialCutsCouple alCuts = G4MaterialCutsCouple(al);
    std::pair<G4Material*,G4MaterialCutsCouple> alMat(al,alCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(13.,alMat));
  }
  G4Material* si = G4Nist->G4NistManager::FindOrBuildMaterial("G4_Si");
  if (si) {
    G4MaterialCutsCouple siCuts = G4MaterialCutsCouple(si);
    std::pair<G4Material*,G4MaterialCutsCouple> siMat(si,siCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(14.,siMat));
  }
  G4Material* ar = G4Nist->G4NistManager::FindOrBuildMaterial("G4_Ar");
  if (ar) {
    G4MaterialCutsCouple arCuts = G4MaterialCutsCouple(ar);
    std::pair<G4Material*,G4MaterialCutsCouple> arMat(ar,arCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(18.,arMat));
  }
  G4Material* fe = G4Nist->G4NistManager::FindOrBuildMaterial("G4_Fe");
  if (fe) {
    G4MaterialCutsCouple feCuts = G4MaterialCutsCouple(fe);
    std::pair<G4Material*,G4MaterialCutsCouple> feMat(fe,feCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(26.,feMat));
  }
  G4Material* pb = G4Nist->G4NistManager::FindOrBuildMaterial("G4_Pb");
  if (pb) {
    G4MaterialCutsCouple pbCuts = G4MaterialCutsCouple(pb);
    std::pair<G4Material*,G4MaterialCutsCouple> pbMat(pb,pbCuts);
    m_g4Material.push_back(std::pair<float,std::pair<G4Material*,G4MaterialCutsCouple> >(82.,pbMat));
  }

  ATH_MSG_INFO("material vector size for had interaction:"<< m_g4Material.size());

  //G4cout << *(G4Material::GetMaterialTable()) << std::endl;

  // Flush the G4 cout/cerr: ATLASSIM-5137
  G4cout << std::flush;
  G4cerr << std::flush;

  return StatusCode::SUCCESS;
}

ISF::ISFParticleVector iFatras::G4HadIntProcessor::getHadState(const ISF::ISFParticle* parent,
							       double time, const Amg::Vector3D& position, const Amg::Vector3D& momentum,
							       const Trk::Material *ematprop) const
{
  ISF::ISFParticleVector chDef(0);

  const int pdg = parent->pdgCode();

  // ions not handled at the moment
  if ( pdg>10000 ) return chDef;

  /*
   * This mutex needs to be locked when calling methods that may modify the process map
   * (e.g. initProcessPDG). We rely on the fact that std::map iterators remain valid even
   * when new elements are added. So we only lock on write-access.
   */
  static std::mutex processMapMutex;

  // initialize G4RunManager if not done already
  static const StatusCode g4RunManagerInit = [&]() {
    std::scoped_lock lock(processMapMutex);
    auto this_nc ATLAS_THREAD_SAFE = const_cast<iFatras::G4HadIntProcessor*>(this);
    StatusCode sc ATLAS_THREAD_SAFE = this_nc->initG4RunManager();
    return sc;
  }();
  if (g4RunManagerInit.isFailure()) return chDef;

  // find corresponding hadronic interaction process ----------------------------------------------
  //
  std::map<int, G4VProcess*>::const_iterator processIter_inelast = m_g4HadrInelasticProcesses.find(pdg);
  std::map<int, G4VProcess*>::const_iterator processIter_elast   = m_g4HadrElasticProcesses.find(pdg);

  if ( (processIter_inelast==m_g4HadrInelasticProcesses.end()) && (processIter_elast==m_g4HadrElasticProcesses.end()) ) {
    ATH_MSG_DEBUG ( " [ g4sim ] No hadronic interactions registered for current particle type (pdg=" << pdg << ")" );
    std::scoped_lock lock(processMapMutex);
    auto this_nc ATLAS_THREAD_SAFE = const_cast<iFatras::G4HadIntProcessor*>(this);
    this_nc->initProcessPDG(pdg);
    return chDef;       // this interaction aborted but next may go through
  }
  //if ( processIter_inelast==m_g4HadrInelasticProcesses.end()) return chDef;

  ATH_MSG_DEBUG ( " [ g4sim ] Found registered hadronic interactions for current particle type (pdg=" << pdg << ")" );

  // setup up G4Track ------------------------------------------------------------------------------
  //
  const G4ParticleDefinition *g4parDef = G4ParticleTable::GetParticleTable()->FindParticle(pdg);
  if ( g4parDef==0) {
    ATH_MSG_WARNING( "[ ---- ] Unable to find G4ParticleDefinition for particle with PID=" << pdg << " --> skipping hadronic interactions" );
    return chDef;
  }
  G4DynamicParticle* inputPar = new G4DynamicParticle();
  inputPar->SetDefinition( g4parDef);
  // input momentum - respect upper limits
  if ( momentum.mag()>1.e08 ) {
    ATH_MSG_WARNING( "input momentum beyond limit" << momentum.mag() << " --> skipping hadronic interaction" );
    return chDef;
  }
  const G4ThreeVector mom( momentum.x(), momentum.y(), momentum.z() );
  inputPar->SetMomentum( mom);
  // position and timing dummy
  G4Track g4track( inputPar, 0 /* time */, {0, 0, 0} /* position */);
  //G4TouchableHandle g4touchable(new G4TouchableHistory());     // TODO check memory handling here
  //g4track->SetTouchableHandle( g4touchable);

  // setup up G4Material ---------------------------------------------------------------------------
  unsigned int g4matInd  = retrieveG4MaterialIndex(ematprop);

  if (g4matInd >= m_g4Material.size()) {
    return chDef;
  }

  // further G4 initializations (G4Step, G4MaterialCutsCouple, ...)
  G4Step g4step;
  G4StepPoint* g4stepPoint = new G4StepPoint();
  g4step.SetPreStepPoint( g4stepPoint);  // now owned by g4step

  g4stepPoint->SetMaterial(m_g4Material[g4matInd].second.first);
  g4stepPoint->SetMaterialCutsCouple(&(m_g4Material[g4matInd].second.second));

  // preparing G4Step and G4Track
  g4track.SetStep( &g4step);

  // by default, the current process is the inelastic hadr. interaction
  G4VProcess *process = processIter_inelast!=m_g4HadrInelasticProcesses.end() ? processIter_inelast->second : 0;

  // if elastic interactions are enabled and there is a elastic process
  // in the m_g4HadrProcesses_Elastic std::map
  if( m_doElastic && (processIter_elast!=m_g4HadrElasticProcesses.end()) ) {
    double rand   = CLHEP::RandFlat::shoot(m_randomEngine, 0., 1.);

    // use a 50% chance to use either elastic or inelastic processes : TODO retrieve cross-section
    if( rand < 0.5) process = processIter_elast->second;
  }

  ATH_MSG_VERBOSE ( " [ g4sim ] Computing " << process->GetProcessName() << " process with current particle" );

  // do the G4VProcess (actually a G4HadronicProcess) ------------------------------------
  //process->SetVerboseLevel(10);
  //ATH_MSG_VERBOSE ( "Verbose Level is " << process->GetVerboseLevel() );

  G4VParticleChange* g4change = process->PostStepDoIt(g4track, g4step);
  if (!g4change) {
    ATH_MSG_WARNING( " [ ---- ] Geant4 did not return any hadronic interaction information of particle with pdg=" << pdg );
    return chDef;
  }

  // process the secondaries ------------------------------------------------------------------
  unsigned int numSecondaries = g4change->GetNumberOfSecondaries();
  ATH_MSG_DEBUG( "[ g4sim ] Material update created " <<  numSecondaries << " Geant4 particle (s)." );

  // green light for secondaries
  if ( numSecondaries ) {

    ISF::ISFParticleVector           children(numSecondaries);
    ISF::ISFParticleVector::iterator childrenIt = children.begin();
    unsigned short                numChildren = 0;
    for ( unsigned int i = 0; i < numSecondaries; i++ ){

      // get Geant4 created particle (G4Track)
      G4Track *trk = g4change->GetSecondary(i);
      const G4DynamicParticle *dynPar = trk->GetDynamicParticle();

      // drop if below energy threshold
      if ( dynPar->GetTotalMomentum() < m_minMomentum)
        continue;

      // get dynamic particle
      const G4ParticleDefinition *parDef = trk->GetParticleDefinition();

      // skip ions
      if (parDef->GetPDGEncoding()>1.e09) continue;

      //Prepare and build the physics table for secondaries
      //process->PreparePhysicsTable(*parDef);
      //process->BuildPhysicsTable(*parDef);

      ATH_MSG_VERBOSE( " [ g4sim ] Adding child particle to particle stack (pdg=" << parDef->GetPDGEncoding()
		       << " p=" << dynPar->GetTotalMomentum() );


      // create the particle to be put into ISF stack
      const G4ThreeVector &momG4 = dynPar->GetMomentum();
      Amg::Vector3D mom( momG4.x(), momG4.y(), momG4.z() );

      //Let's make sure the new ISFParticle get some valid TruthBinding and HepMcParticleLink objects
      ISF::TruthBinding* truthBinding = NULL;
      if (parent->getTruthBinding()) {
 	        ATH_MSG_VERBOSE("Could retrieve TruthBinding from original ISFParticle");
 	        truthBinding = new ISF::TruthBinding(*parent->getTruthBinding());
      }
      else
 	        ATH_MSG_WARNING("Could not retrieve TruthBinding from original ISFParticle, might cause issues later on.");
      ISF::ISFParticle* cParticle = new ISF::ISFParticle( position,
                                                          mom,
                                                          parDef->GetPDGMass(),
                                                          parDef->GetPDGCharge(),
                                                          parDef->GetPDGEncoding(),
                                                          1, //status
                                                          time,
                                                          *parent,
                                                          Barcode::fUndefinedBarcode,
                                                          truthBinding );
      cParticle->setNextGeoID( parent->nextGeoID() );
      cParticle->setNextSimID( parent->nextSimID() );
      // process sampling tool takes care of validation info
      *childrenIt = cParticle;
      ++childrenIt; numChildren++;
    }

    children.resize(numChildren);
    // truth info handled by process sampling tool

    // free up memory
    g4change->Clear();
    return children;

  }

  // free up memory
  g4change->Clear();
  return chDef;
}



bool iFatras::G4HadIntProcessor::doHadronicInteraction(double time, const Amg::Vector3D& position, const Amg::Vector3D& momentum,
						       const Trk::Material *ematprop,
						       Trk::ParticleHypothesis /*particle*/,
						       bool  processSecondaries) const
{
  // get parent particle
  // @TODO: replace by Fatras internal bookkeeping
  const ISF::ISFParticle *parent = ISF::ParticleClipboard::getInstance().getParticle();
  // something is seriously wrong if there is no parent particle
  assert(parent);

  ISF::ISFParticleVector ispVec=getHadState(parent, time, position, momentum, ematprop);

  if (!ispVec.size()) return false;

  // push onto ParticleStack

  if (processSecondaries) {
    for (unsigned int ic=0; ic<ispVec.size(); ic++) {
	//First let's make sure that new ISFParticles have valid truth info
	if (!ispVec[ic]->getTruthBinding()) {
		ispVec[ic]->setTruthBinding(new ISF::TruthBinding(*parent->getTruthBinding()));
	}
	m_particleBroker->push(ispVec[ic], parent);
    }
  }

  return true;

}

ISF::ISFParticleVector iFatras::G4HadIntProcessor::doHadIntOnLayer(const ISF::ISFParticle* parent, double time,
								   const Amg::Vector3D& position, const Amg::Vector3D& momentum,
								   const Trk::Material *emat,
								   Trk::ParticleHypothesis /*particle=Trk::pion*/) const
{

  return getHadState(parent, time, position, momentum, emat);

}

unsigned int iFatras::G4HadIntProcessor::retrieveG4MaterialIndex(const Trk::Material *ematprop) const {

  unsigned int nMaterials = m_g4Material.size();
  unsigned int invalidIndex = nMaterials + 1;
  
  if (0==nMaterials) {
    ATH_MSG_WARNING(" no predefined G4 material available for hadronic interaction " );
    return (invalidIndex);
  }

  // in the absence of detailed material composition, use average Z
  // if not available on input, take Al
  float  iZ = ematprop ? ematprop->averageZ() : 13 ;

  // choose from predefined materials
  unsigned int imat=0;

  while (imat < nMaterials && iZ > m_g4Material[imat].first ) imat++;

  unsigned int iSel=imat< nMaterials ? imat : nMaterials-1;

  if (iSel>0) {
    // pick randomly to reproduce the average Z
    //double rnd   = CLHEP::RandFlat::shoot(m_randomEngine, 0., 1.);
    //if (rnd < (iZ-m_g4Material[iSel-1].first)/(m_g4Material[iSel].first-m_g4Material[iSel-1].first)) iSel--;
    // weighted
    float dz2 =  -pow(m_g4Material[iSel-1].first,2)+pow(m_g4Material[iSel].first,2);
    double rnd   = CLHEP::RandFlat::shoot(m_randomEngine, 0., 1.);
    if (iZ*iZ+pow(m_g4Material[iSel-1].first,2) < rnd*dz2) iSel--;
  }

  return(iSel);

}
