/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
//
// MmDigitizationTool
// ------------
// Authors: Nektarios Chr. Benekos <nectarios.benekos@cern.ch>
//          Konstantinos Karakostas <Konstantinos.Karakostas@cern.ch>
////////////////////////////////////////////////////////////////////////////////


//Inputs
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"
 
//Outputs
#include "MuonDigitContainer/MmDigitContainer.h"

//MM digitization includes
#include "MM_Digitization/MmDigitizationTool.h"
#include "MM_Digitization/IMM_DigitizationTool.h"
#include "MM_Digitization/MmDigitToolInput.h"
#include "MuonSimEvent/MM_SimIdToOfflineId.h"

#include "MuonReadoutGeometry/NSWenumeration.h"
#include "MuonReadoutGeometry/NSWgeometry.h"
 
//Gaudi - Core
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "StoreGate/StoreGateSvc.h"
#include "PathResolver/PathResolver.h"
#include "AIDA/IHistogram1D.h"
#include "EventInfo/TagInfo.h"
#include "EventInfoMgt/ITagInfoMgr.h"

//Geometry
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonSimEvent/MicromegasHitIdHelper.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkSurfaces/Surface.h"

//Pile-up
#include "PileUpTools/PileUpMergeSvc.h"

//Truth
#include "CLHEP/Units/PhysicalConstants.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "HepMC/GenParticle.h"

//Random Numbers
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGamma.h"
#include "CLHEP/Random/RandPoisson.h"
#include "AtlasCLHEP_RandomGenerators/RandGaussZiggurat.h"
#include "CLHEP/Random/RandExponential.h"

//ROOT
#include "TH1.h"
#include "TTree.h"
#include "TFile.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace MuonGM;

/*******************************************************************************/
MmDigitizationTool::MmDigitizationTool(const std::string& type, const std::string& name, const IInterface* parent)
  : PileUpToolBase(type, name, parent)
  , m_sgSvc("StoreGateSvc", name)
  , m_digitContainer(NULL)
  , m_sdoContainer(NULL)
  , m_digitTool("MM_Response_DigitTool", this)
  , m_validationSetup(false)
  , m_energyThreshold(50.)
  , m_saveInternalHistos(false)
  , m_thpcMM(0)
  , m_Polya(0.0)
  , m_sprob(0)
  , m_amplification(0.0)
  , m_StripsResponse(0)
  , m_qThreshold(0.001) 
  , m_diffusSigma(0.036)
  , m_LogitundinalDiffusSigma(0.019)
  , m_driftVelocity(0.047)
  , m_crossTalk1(0.)
  , m_crossTalk2(0.)
  , m_qThresholdForTrigger(1.)
  , m_ElectronicsResponse(0)
  , m_file(0)
  , m_ntuple(0)
  , m_AngleDistr(0)
  , m_AbsAngleDistr(0), m_ClusterLength2D(0), m_ClusterLength(0)
  , m_mergeSvc(0)
  , m_inputObjectName("")
  , m_outputObjectName("")
  , m_rndmSvc("AtRndmGenSvc", name )
  , m_rndmEngine(0)
  , m_rndmEngineName("MuonDigitization")
{

  declareInterface<IMuonDigitizationTool>(this);

  declareProperty("MaskMultiplet", m_maskMultiplet = 0,  "0: all, 1: first, 2: second, 3: both"  );
  
  //Random numbers service               
  declareProperty("RndmSvc",             m_rndmSvc,            "Random Number Service used in Muon digitization");
  declareProperty("RndmEngine",          m_rndmEngineName,     "Random engine name");
  declareProperty("DigitizationTool",    m_digitTool,          "Tool which handle the digitization process");
  declareProperty("MCStore",             m_sgSvc,              "help");

  //Object names                         
  declareProperty("InputObjectName",     m_inputObjectName     =  "MicromegasSensitiveDetector");
  declareProperty("OutputObjectName",    m_outputObjectName    =  "MM_DIGITS");
  declareProperty("OutputSDOName",       m_outputSDOName       =  "MM_SDO");

  //Configurations
  declareProperty("CheckSimHits",        m_checkMMSimHits      =  true,       "Control on the hit validity");
 
  //Timing scheme                         
  declareProperty("UseTimeWindow",       m_useTimeWindow  =  true);
  declareProperty("WindowLowerOffset",   m_timeWindowLowerOffset = -50.);
  declareProperty("WindowUpperOffset",   m_timeWindowUpperOffset = +50.);
  declareProperty("DiffMagSecondMuonHit",m_DiffMagSecondMuonHit = 0.1);
 
  // electronics
  declareProperty("ns2TDC",              m_ns2TDC = 0.78125, "Conversion factor TDC/ns");
  declareProperty("ResolutionTDC",       m_resTDC = 0.5, "TDC resolution");

  // Constants vars for the StripsResponse class
  // qThreshold=2e, we accept a good strip if the charge is >=2e
  declareProperty("qThreshold",              m_qThreshold = 0.001); 
  // transverse diffusion (350 μm per 1cm ) for 93:7 @ 600 V/cm, according to garfield      
  declareProperty("DiffusSigma",             m_diffusSigma = 0.036);    
  declareProperty("LogitundinalDiffusSigma", m_LogitundinalDiffusSigma = 0.019);
  // 0.050 drift velocity in [mm/ns], driftGap=5 mm +0.128 mm (the amplification gap)
  declareProperty("driftGap",                m_driftGap = 5.128);
  declareProperty("DriftVelocity",           m_driftVelocity = 0.047);  
  // crosstalk of neighbor strips, it's 15%
  declareProperty("crossTalk1",		     m_crossTalk1 = 0.0);        
  // crosstalk of neighbor strips, it's 6%
  declareProperty("crossTalk2",		     m_crossTalk2 = 0.0); 
  declareProperty("qThresholdForTrigger",    m_qThresholdForTrigger = 1.0); 
  
  // Constants vars for the ElectronicsResponse
  declareProperty("alpha",                   m_alpha = 2.5);
  declareProperty("RC",                      m_RC = 20.);
  declareProperty("electronicsThreshold",    m_electronicsThreshold = 0.000811174);
   
  declareProperty("SaveInternalHistos",  m_saveInternalHistos = false   );
  declareProperty("ValidationSetup",     m_validationSetup = false   );
  declareProperty("EnergyThreshold",     m_energyThreshold = 50., "Minimal energy to produce a PRD"  );

}
/*******************************************************************************/
// member function implementation
//--------------------------------------------
StatusCode MmDigitizationTool::initialize() {

  StatusCode status(StatusCode::SUCCESS);
  ATH_MSG_DEBUG ("MmDigitizationTool:: in initialize()") ;
  ATH_MSG_DEBUG ( "Configuration  MmDigitizationTool " );
  ATH_MSG_DEBUG ( "RndmSvc                " << m_rndmSvc             );
  ATH_MSG_DEBUG ( "RndmEngine             " << m_rndmEngineName      );
  ATH_MSG_DEBUG ( "MCStore                " << m_sgSvc               );
  ATH_MSG_DEBUG ( "DigitizationTool       " << m_digitTool           );
  ATH_MSG_DEBUG ( "InputObjectName        " << m_inputObjectName     );
  ATH_MSG_DEBUG ( "OutputObjectName       " << m_outputObjectName    );
  ATH_MSG_DEBUG ( "OutputSDOName          " << m_outputSDOName       );
  ATH_MSG_DEBUG ( "UseTimeWindow          " << m_useTimeWindow       );
  ATH_MSG_DEBUG ( "CheckSimHits           " << m_checkMMSimHits      );
  ATH_MSG_DEBUG ( "ns2TDC                 " << m_ns2TDC              );
  ATH_MSG_DEBUG ( "ResolutionTDC          " << m_resTDC              );
  ATH_MSG_DEBUG ( "Threshold              " << m_qThreshold          ); 
  ATH_MSG_DEBUG ( "DiffusSigma            " << m_diffusSigma 	     );    
  ATH_MSG_DEBUG ( "LogitundinalDiffusSigma" << m_LogitundinalDiffusSigma );
  ATH_MSG_DEBUG ( "DriftVelocity          " << m_driftVelocity       );  
  ATH_MSG_DEBUG ( "crossTalk1             " << m_crossTalk1 	     );        
  ATH_MSG_DEBUG ( "crossTalk2             " << m_crossTalk2 	     ); 
  ATH_MSG_INFO ( "ValidationSetup         " << m_validationSetup     );
  ATH_MSG_INFO ( "EnergyThreshold         " << m_energyThreshold     );
  
  // initialize random number generators
  // then initialize the CSC identifier helper
  // This method must be called before looping over the hits
  // to digitize them using the method digitize_hit below
  m_bunchTime = 0.0;

  // initialize random number generators
  //  double average_int = 30;  // average interactions per cm
  //  m_FlatDist = CLHEP::RandFlat::shoot(m_rndmEngine, 0.0,1.0);
  //  m_GaussDist = CLHEP::RandGauss::shoot(m_rndmEngine,0.0,1.0); 
  //  m_GammaDist = CLHEP::RandGamma::shoot(m_rndmEngine, (1.0+m_Polya), 1.0);
  //  m_PoissonDist = CLHEP::RandPoisson::shoot(m_rndmEngine, average_int);


  // initialize transient event store
  if (m_sgSvc.retrieve().isFailure()) {
    ATH_MSG_FATAL ( "Could not retrieve StoreGateSvc!" );
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG ( "Retrieved StoreGateSvc." );

  status = service("ActiveStoreSvc", m_activeStore);
  if ( !status.isSuccess() ) {
    ATH_MSG_FATAL ( "Could not get active store service" );
    return status;
  }

  // initialize transient detector store and MuonGeoModel OR MuonDetDescrManager 
  StoreGateSvc* detStore=0;
  m_MuonGeoMgr=0;
  status = serviceLocator()->service("DetectorStore", detStore);
  if (status.isSuccess()) {
    if(detStore->contains<MuonGM::MuonDetectorManager>( "Muon" )){
       
      status = detStore->retrieve(m_MuonGeoMgr);
      if (status.isFailure()) {
	ATH_MSG_FATAL ( "Could not retrieve MuonGeoModelDetectorManager!" );
	return status;
      }
      else {
	ATH_MSG_DEBUG ( "Retrieved MuonGeoModelDetectorManager from StoreGate" );
	//initialize the MmIdHelper
	m_idHelper  = m_MuonGeoMgr->mmIdHelper();
	if(!m_idHelper) return status;
	ATH_MSG_DEBUG ( "Retrieved MmIdHelper " << m_idHelper );
      }
    }
  }
  else {
    ATH_MSG_FATAL ( "Could not retrieve DetectorStore!" );
    return status;
  }
   
  // check the input object name
  if (m_inputObjectName=="") {
    ATH_MSG_FATAL ( "Property InputObjectName not set !" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "Input objects: '" << m_inputObjectName << "'" );
  }
   
  // check the output object name
  if (m_outputObjectName=="") {
    ATH_MSG_FATAL ( "Property OutputObjectName not set !" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "Output digits: '" << m_outputObjectName << "'" );
  }
   
  //initialize the digit container 
  try{
    m_digitContainer = new MmDigitContainer(m_idHelper->detectorElement_hash_max());
  }
  catch(std::bad_alloc){
    ATH_MSG_FATAL ( "Could not create a new MicroMegas DigitContainer!" );
    return StatusCode::FAILURE;
  }
  m_digitContainer->addRef();
   
  //simulation identifier helper       
  muonHelper = MicromegasHitIdHelper::GetHelper();
   
  //get the r->t conversion tool
  status  = m_digitTool.retrieve();
  if( status.isFailure() ) {
    ATH_MSG_FATAL("Could not retrieve digitization tool! " << m_digitTool);
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG("Retrieved digitization tool!" << m_digitTool);
  }
  
  // initialize inverse lightSpeed (c_light in m/s)
  m_inv_c_light = 1./(CLHEP::c_light);
   
  if (!m_rndmSvc.retrieve().isSuccess()) {
    ATH_MSG_ERROR("Could not initialize Random Number Service");
  }     
   
  // getting our random numbers stream
  ATH_MSG_DEBUG ( "Getting random number engine : <" << m_rndmEngineName << ">" );
  if (!m_rndmSvc.retrieve().isSuccess()){
    ATH_MSG_ERROR("Could not initialize Random Number Service");
  }
  m_rndmEngine = m_rndmSvc->GetEngine(m_rndmEngineName);
  if (m_rndmEngine==0) {
    ATH_MSG_ERROR("Could not find RndmEngine : " << m_rndmEngineName );
    return StatusCode::FAILURE;
  }
  
  //locate the PileUpMergeSvc and initialize our local ptr
  const bool CREATEIF(true);
  if (!(service("PileUpMergeSvc", m_mergeSvc, CREATEIF)).isSuccess() || 0 == m_mergeSvc) {
    ATH_MSG_ERROR ( "Could not find PileUpMergeSvc" );
    return StatusCode::FAILURE;
  }

  // StripsResponse Creation
  m_StripsResponse = new StripsResponse();
  m_StripsResponse->set_qThreshold(m_qThreshold);
  m_StripsResponse->set_diffusSigma(m_diffusSigma);
  m_StripsResponse->set_LogitundinalDiffusSigma(m_LogitundinalDiffusSigma);
  m_StripsResponse->set_driftGap(m_driftGap);
  m_StripsResponse->set_driftVelocity(m_driftVelocity); 
  m_StripsResponse->set_crossTalk1(m_crossTalk1);
  m_StripsResponse->set_crossTalk2(m_crossTalk2);
 
  m_ElectronicsResponse = new ElectronicsResponse(); 
  m_ElectronicsResponse->set_alpha(m_alpha);
  m_ElectronicsResponse->set_RC(m_RC);
  m_ElectronicsResponse->set_electronicsThreshold(m_electronicsThreshold);
  
  //ROOT Staff for internal validation
  if (m_saveInternalHistos) m_file = new TFile("MM_fullSimu_plots.root","RECREATE");
  m_ntuple = new TTree("fullSim","fullSim");

  m_ntuple->Branch("exitcode",&exitcode);
  m_ntuple->Branch("Station_side",&m_n_Station_side);
  m_ntuple->Branch("Station_eta",&m_n_Station_eta);
  m_ntuple->Branch("Station_phi",&m_n_Station_phi);
  m_ntuple->Branch("Station_multilayer",&m_n_Station_multilayer);
  m_ntuple->Branch("Station_layer",&m_n_Station_layer);
  
  m_ntuple->Branch("hitPDGId",&m_n_hitPDGId);
  m_ntuple->Branch("hitKineticEnergy",&m_n_hitKineticEnergy);
  m_ntuple->Branch("hitDepositEnergy",&m_n_hitDepositEnergy);
  m_ntuple->Branch("hitOnSurface_x",&m_n_hitOnSurface_x);
  m_ntuple->Branch("hitOnSurface_y",&m_n_hitOnSurface_y);
  m_ntuple->Branch("hitStripID",&m_n_hitStripID);
  m_ntuple->Branch("hitDistToChannel",&m_n_hitDistToChannel);
  m_ntuple->Branch("hitIncomingAngle",&m_n_hitIncomingAngle);
  m_ntuple->Branch("hitIncomingAngleRads",&m_n_hitIncomingAngleRads);
  
  m_ntuple->Branch("StrRespID",&m_n_StrRespID);
  m_ntuple->Branch("StrRespCharge",&m_n_StrRespCharge);
  m_ntuple->Branch("StrRespTime",&m_n_StrRespTime);
  m_ntuple->Branch("StrRespTrg_ID",&m_n_StrRespTrg_ID);
  m_ntuple->Branch("StrRespTrg_Time",&m_n_StrRespTrg_Time);
  m_ntuple->Branch("Strip_Multiplicity_byDiffer",&m_n_strip_multiplicity);
  m_ntuple->Branch("Strip_Multiplicity_2",&m_n_strip_multiplicity_2);
  m_ntuple->Branch("tofCorrection",&tofCorrection);
  m_ntuple->Branch("bunchTime",&bunchTime);
  m_ntuple->Branch("globalHitTime",&globalHitTime);
  
  m_AngleDistr = new TH1I("m_AngleDistr", "m_AngleDistr", 360, -180., 180.);
  m_AbsAngleDistr = new TH1I("m_AbsAngleDistr", "m_AbsAngleDistr", 180, 0., 180.);
  m_ClusterLength2D = new TH1I("m_ClusterLength2D", "m_ClusterLength2D", 100, 0., 100.);
  m_ClusterLength = new TH1I("m_ClusterLength", "m_ClusterLength", 100, 0., 100.);
  m_gasGap = new TH1I("m_gasGap", "m_gasGap", 100, 0., 100.);
  m_gasGapDir = new TH1I("m_gasGapDir", "m_gasGapDir", 20, -10., 10.);
 
  // m_thpcMM->reserve(20000);
 
 
  return status;
}
/*******************************************************************************/ 
//----------------------------------------------------------------------
// PrepareEvent method:
//----------------------------------------------------------------------
StatusCode MmDigitizationTool::prepareEvent(unsigned int nInputEvents) {

  ATH_MSG_DEBUG("MmDigitizationTool::prepareEvent() called for " << nInputEvents << " input events" );
  //m_digitContainer->cleanup();
  m_MMHitCollList.clear();
  // if (0 == m_thpcMM) { /*m_thpcMM->reserve(20000);*/  m_thpcMM = new TimedHitCollection<GenericMuonSimHit>();   }
  //m_thpcMM = new TimedHitCollection<GenericMuonSimHit>();   
  
  if(!m_thpcMM) { 
        m_thpcMM = new TimedHitCollection<GenericMuonSimHit>();
  }else{
	ATH_MSG_ERROR ( "m_thpcMM is not null" );
	return StatusCode::FAILURE;	
  }
  
  //m_MMHitCollList.push_back(NULL);
  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode MmDigitizationTool::processBunchXing(int bunchXing,
						PileUpEventInfo::SubEvent::const_iterator bSubEvents,
						PileUpEventInfo::SubEvent::const_iterator eSubEvents) {

  ATH_MSG_DEBUG ( "MmDigitizationTool::in processBunchXing()"  << bunchXing );
  
  PileUpEventInfo::SubEvent::const_iterator iEvt = bSubEvents;
 
  //loop on event and sub-events for the current bunch Xing
  for (; iEvt!=eSubEvents; ++iEvt) {
    StoreGateSvc& seStore = *iEvt->pSubEvtSG;
    const EventInfo* pEI(0);
    if (seStore.retrieve(pEI).isSuccess()) {
      ATH_MSG_INFO( "SubEvt EventInfo from StoreGate " << seStore.name() << " :"
		    << " bunch crossing : " << bunchXing
		    << " time offset : " << iEvt->time()
		    << " event number : " << pEI->event_ID()->event_number()
		    << " run number : " << pEI->event_ID()->run_number() );
    }
 
    PileUpTimeEventIndex thisEventIndex = PileUpTimeEventIndex(static_cast<int>(iEvt->time()),iEvt->index());
      
    const GenericMuonSimHitCollection* seHitColl = 0;
    if (!seStore.retrieve(seHitColl,m_inputObjectName).isSuccess()) {
      ATH_MSG_ERROR ( "SubEvent MicroMegas SimHitCollection not found in StoreGate " << seStore.name() );
      return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE ( "MicroMegas SimHitCollection found with " << seHitColl->size() << " hits" );
    
    const double timeOfBCID(static_cast<double>(iEvt->time()));
    ATH_MSG_DEBUG ( "timeOfBCID " << timeOfBCID );

    //Copy hit Collection
    GenericMuonSimHitCollection* MMHitColl = new GenericMuonSimHitCollection(m_inputObjectName);
    
    GenericMuonSimHitCollection::const_iterator i = seHitColl->begin();
    GenericMuonSimHitCollection::const_iterator e = seHitColl->end();
 
    // Read hits from this collection
    for (; i!=e; ++i){ 
      MMHitColl->Emplace(*i);
    }
    m_thpcMM->insert(thisEventIndex, MMHitColl);
    
    //store these for deletion at the end of mergeEvent
    m_MMHitCollList.push_back(MMHitColl);
 
  }//  while (iEvt != eSubEvents) {
  return StatusCode::SUCCESS;
}

/*******************************************************************************/
StatusCode MmDigitizationTool::getNextEvent() {

  // Get next event and extract collection of hit collections:
  // This is applicable to non-PileUp Event... 

  ATH_MSG_DEBUG ( "MmDigitizationTool::getNextEvent()" );
   
  if (!m_mergeSvc) {
    const bool CREATEIF(true);
    if (!(service("PileUpMergeSvc", m_mergeSvc, CREATEIF)).isSuccess() ||
	0 == m_mergeSvc) {
      ATH_MSG_ERROR ("Could not find PileUpMergeSvc" );
      return StatusCode::FAILURE;
    }
  }

  // initialize pointer
  //m_thpcMM = 0;
 
  //  get the container(s)
  typedef PileUpMergeSvc::TimedList<GenericMuonSimHitCollection>::type TimedHitCollList;
 
  //this is a list<info<time_t, DataLink<GenericMuonSimHitCollection> > >
  TimedHitCollList hitCollList;
    
  if (!(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList).isSuccess()) ) {
    ATH_MSG_ERROR ( "Could not fill TimedHitCollList" );
    return StatusCode::FAILURE;
  }  
  if (hitCollList.size()==0) {
    ATH_MSG_ERROR ( "TimedHitCollList has size 0" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( hitCollList.size() << " MicroMegas SimHitCollections with key " << m_inputObjectName << " found" );
  } 
   
  // create a new hits collection - Define Hit Collection
  // m_thpcMM = new TimedHitCollection<GenericMuonSimHit>() ;
  if(!m_thpcMM) { 
        m_thpcMM = new TimedHitCollection<GenericMuonSimHit>();
  }else{
	ATH_MSG_ERROR ( "m_thpcMM is not null" );
	return StatusCode::FAILURE;	
  }
 
  //now merge all collections into one
  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());
 
  // loop on the hit collections
  while(iColl != endColl) {
    const GenericMuonSimHitCollection* tmpColl(iColl->second);  
    m_thpcMM->insert(iColl->first, tmpColl);
    ATH_MSG_DEBUG ( "MMSimHitCollection found with " << tmpColl->size() << " hits"  );
    ++iColl; 
  } 
  return StatusCode::SUCCESS;
} 
/*******************************************************************************/
StatusCode MmDigitizationTool::mergeEvent() {

  StatusCode status = StatusCode::SUCCESS;

  // ATH_MSG_DEBUG ( "MmDigitizationTool::in mergeEvent()" );

  // Cleanup and record the Digit container in StoreGate
  status = recordDigitAndSdoContainers();
  if(!status.isSuccess()) {
    ATH_MSG_FATAL("MMDigitizationTool::recordDigitAndSdoContainers failed.");
    return StatusCode::FAILURE; 
  }
   
  status = doDigitization();
  if (status.isFailure()) { ATH_MSG_ERROR ( "doDigitization Failed" );  return StatusCode::FAILURE; }
 
  // reset the pointer (delete null pointer should be safe)
  if (m_thpcMM){
    delete m_thpcMM;
    m_thpcMM = 0;
  }
  
  // remove cloned one in processBunchXing...... 
  std::list<GenericMuonSimHitCollection*>::iterator MMHitColl = m_MMHitCollList.begin();
  std::list<GenericMuonSimHitCollection*>::iterator MMHitCollEnd = m_MMHitCollList.end();
  while(MMHitColl!=MMHitCollEnd) {
    delete (*MMHitColl);
    ++MMHitColl;
  }
  m_MMHitCollList.clear(); 
  return status;
}
/*******************************************************************************/
StatusCode MmDigitizationTool::digitize() {
  return this->processAllSubEvents(); 
}
/*******************************************************************************/
StatusCode MmDigitizationTool::processAllSubEvents() {
 
  ATH_MSG_DEBUG ("MmDigitizationTool::processAllSubEvents()");

  StatusCode status = recordDigitAndSdoContainers();
  //if(!status.isSuccess()) {
  //  ATH_MSG_FATAL("MMDigitizationTool::recordDigitAndSdoContainers failed.");
  //  return StatusCode::FAILURE; // there are no hits in this event
  //}

  //ATH_MSG_DEBUG ( "MicroMegas SDO collection (MmSDOCollection) recorded in StoreGate." );

  //merging of the hit collection in getNextEvent method   

  if (0 == m_thpcMM ) {
    status = getNextEvent();
    if(status.isFailure()) {
      ATH_MSG_FATAL( "There are no MicroMegas hits in this event" );
      return StatusCode::FAILURE;
    }
  }
  
  status = doDigitization();
  if(!status.isSuccess()) { 
    ATH_MSG_ERROR( "MmDigitizationTool :: doDigitization() Failed" );  
    return StatusCode::FAILURE; 
  }
  
  // reset the pointer (delete null pointer should be safe)
  if (m_thpcMM){
    delete m_thpcMM;
    m_thpcMM = 0;
  }
  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode MmDigitizationTool::finalize() {
  //ATH_MSG_DEBUG("MMDigitizationTool::finalize() ---- m_digitContainer->digit_size() = "<<m_digitContainer->digit_size() );
  m_digitContainer->release();
  
  if (m_saveInternalHistos) {
    m_ntuple->Write();
    m_AngleDistr->Write();
    m_AbsAngleDistr->Write();
    m_ClusterLength2D->Write();
    m_ClusterLength->Write();
    m_gasGap->Write();
    m_gasGapDir->Write();
    m_file->Close();
  }
  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode MmDigitizationTool::recordDigitAndSdoContainers() { 
 
  // cleanup digit container
  m_digitContainer->cleanup();
  
  // record the digit container in StoreGate
  m_activeStore->setStore(&*m_sgSvc);
  StatusCode status = m_sgSvc->record(m_digitContainer, m_outputObjectName);
  if(status.isFailure()) {
    ATH_MSG_FATAL("Unable to record Micromegas digit container in StoreGate");
    return status;
  }else { ATH_MSG_DEBUG("MmDigitContainer recorded in StoreGate.");}
  
  // create and record the SDO container in StoreGate
  m_sdoContainer = new MuonSimDataCollection();
  status = m_sgSvc->record(m_sdoContainer, m_outputSDOName);
  if(status.isFailure()) {
    ATH_MSG_FATAL("Unable to record MM SDO collection in StoreGate");
    return status;
  } else { ATH_MSG_DEBUG("MMSDOCollection recorded in StoreGate."); }
  
  return status;
}

/*******************************************************************************/
StatusCode MmDigitizationTool::doDigitization() {

  
  GenericMuonSimHitCollection* inputSimHitColl=NULL;
 
  if (m_validationSetup)
    {  
      inputSimHitColl = new GenericMuonSimHitCollection("MicromegasSensitiveDetector");
      StatusCode status = m_sgSvc->record(inputSimHitColl,"InputMicroMegasHits");
      if (status.isFailure())  {
	ATH_MSG_ERROR ( "Unable to record Input MicromegasSensitiveDetector HIT collection in StoreGate"  );
	return status;
      }
    }
        
  if( m_maskMultiplet == 3 ) {
   
    return StatusCode::SUCCESS;
  }
 
  // Perform null check on m_thpcCSC
  //   if(!m_thpcMM) {
  //     ATH_MSG_ERROR ( "m_thpcMM is null" );
  //     return StatusCode::FAILURE;
  //   }

  //iterate over hits and fill id-keyed drift time map
  TimedHitCollection< GenericMuonSimHit >::const_iterator i, e;
  const GenericMuonSimHit* previousHit = 0;
   
  // ATH_MSG_DEBUG("create PRD container of size " << m_idHelper->detectorElement_hash_max()); 
  std::map<Identifier,int> hitsPerChannel;
  int nhits = 0;

  // nextDetectorElement-->sets an iterator range with the hits of current detector element , returns a bool when done
  while( m_thpcMM->nextDetectorElement(i, e) ) {
        
    // Loop over the hits:
    while (i != e) {

      TimedHitPtr<GenericMuonSimHit> phit = *i++;
      const GenericMuonSimHit& hit(*phit);
      
      // SimHits without energy loss are not recorded.
      // not needed because of already done in sensitive detector
      // https://svnweb.cern.ch/trac/atlasoff/browser/MuonSpectrometer/MuonG4/MuonG4SD/trunk/src/MicromegasSensitiveDetector.cxx?rev=542333#L65
      // if(hit.depositEnergy()==0.) continue;

      if( previousHit && abs(hit.particleEncoding())==13 && abs(previousHit->particleEncoding())==13 ) {
	Amg::Vector3D diff = previousHit->localPosition() - hit.localPrePosition();
	ATH_MSG_DEBUG("second hit from a muon: prev " <<  previousHit->localPosition() << " current " << hit.localPrePosition() << " diff " << diff );
	if( diff.mag() < m_DiffMagSecondMuonHit ) continue;
      }
      m_n_hitPDGId = hit.particleEncoding();
      m_n_hitDepositEnergy = hit.depositEnergy();
      m_n_hitKineticEnergy = hit.kineticEnergy();

      globalHitTime = hit.globalpreTime();
      tofCorrection = hit.globalPosition().mag()/CLHEP::c_light;
      bunchTime = globalHitTime - tofCorrection;
      const float stripPropagationTime = 0.;
      static const float jitter = 0.;
      float MMDigitTime = bunchTime + jitter + stripPropagationTime;

      //     ATH_MSG_DEBUG ( "MMDigitTime " << timeOfBCID );

      const float timeWindowStrip = 120.; //driftvelocity gap;
      if (MMDigitTime < -timeWindowStrip || MMDigitTime > timeWindowStrip) {
	exitcode = 4; m_ntuple->Fill();
	continue;
      }
  
      // SimHits without energy loss are not recorded.
      ///if(hit.depositEnergy()==0. && abs(hit.particleEncoding())!=13) continue;

      m_n_Station_side=-111; m_n_Station_eta=-111; m_n_Station_phi=-111; m_n_Station_multilayer=-111; m_n_Station_layer=-111; m_n_hitStripID=-111; m_n_StrRespTrg_ID=-111;
      m_n_hitOnSurface_x=-999.; m_n_hitOnSurface_y=-999.; m_n_hitDistToChannel=-999.; m_n_hitIncomingAngle=-999.; m_n_StrRespTrg_Time=-999.;
      m_n_strip_multiplicity = -999.;
      exitcode = 0;
      m_n_StrRespID.clear();
      m_n_StrRespCharge.clear(); m_n_StrRespTime.clear();

      const int idHit = hit.GenericId();
      // the G4 time or TOF from IP
      double G4Time(hit.globalTime());
      // see what are the members of GenericMuonSimHit
      const Amg::Vector3D globPos = hit.globalPosition();
   
      // convert sim id helper to offline id  
      muonHelper = MicromegasHitIdHelper::GetHelper();  
      MM_SimIdToOfflineId simToOffline(*m_idHelper);
      
      //get the hit Identifier and info
      int simId=hit.GenericId();
      Identifier layid = simToOffline.convert(simId);
   
      // Read the information about the Micro Megas hit
      ATH_MSG_DEBUG ( "> idHit  " << idHit << " Hit bunch time  " << bunchTime << " tot " << globalHitTime << " tof/G4 time " << hit.globalTime() << " globalPosition " << globPos 
		      << "hit: r " << hit.globalPosition().perp() << " z " << hit.globalPosition().z() << " mclink " << hit.particleLink() << " station eta " << m_idHelper->stationEta(layid) << " station phi " << m_idHelper->stationPhi(layid) << " multiplet " << m_idHelper->multilayer(layid) ); 

      if (m_validationSetup){
	GenericMuonSimHit* copyHit = new GenericMuonSimHit(idHit, hit.globalpreTime(), hit.globalTime(), hit.globalPosition(), hit.localPosition(), hit.globalPrePosition(), hit.localPrePosition(), hit.particleEncoding(), hit.kineticEnergy(), hit.globalDirection(), hit.depositEnergy(), hit.StepLength(), hit.trackNumber() ); 
	ATH_MSG_INFO("Validation:  globalHitTime, G4Time, BCtime = "<<globalHitTime<<" "<<G4Time<<" "<<bunchTime << "\n: " << copyHit->print() );
        inputSimHitColl->Insert(*copyHit);
      }
      // Important checks for hits (global time, position along strip, charge, masked chambers etc..) DO NOT SET THIS CHECK TO FALSE IF YOU DON'T KNOW WHAT YOU'RE DOING !
      if(m_checkMMSimHits) { if(checkMMSimHit(hit) == false) {exitcode = 8; m_ntuple->Fill();continue;} }
  
 
      // sanity checks
      if( !m_idHelper->is_mm(layid) ){
	ATH_MSG_WARNING("MM id is not a mm id! " << m_idHelper->stationNameString(m_idHelper->stationName(layid)) );
      }		
				
      std::string stName = m_idHelper->stationNameString(m_idHelper->stationName(layid));
      int isSmall = stName[2] == 'S';
 
      if( m_idHelper->is_mdt(layid)|| m_idHelper->is_rpc(layid)|| m_idHelper->is_tgc(layid)|| m_idHelper->is_csc(layid)|| m_idHelper->is_stgc(layid) ){
	ATH_MSG_WARNING("MM id has wrong technology type! " << m_idHelper->is_mdt(layid) << " " << m_idHelper->is_rpc(layid) << " " << m_idHelper->is_tgc(layid) << " " << m_idHelper->is_csc(layid) << " " << m_idHelper->is_stgc(layid) );
	exitcode = 9; m_ntuple->Fill();
      }

      if( m_idHelper->stationPhi(layid) == 0 ){
	ATH_MSG_WARNING("unexpected phi range " << m_idHelper->stationPhi(layid) );
	exitcode = 9; m_ntuple->Fill();
	continue;
      }


      // remove hits in masked multiplet
      if( m_maskMultiplet == m_idHelper->multilayer(layid) ) continue;
	

      // get readout element
      const MuonGM::MMReadoutElement* detEl = m_MuonGeoMgr->getMMReadoutElement(layid);
      if( !detEl ){
	ATH_MSG_WARNING( "Failed to retrieve detector element for: isSmall " << isSmall << " eta " << m_idHelper->stationEta(layid) << " phi " << m_idHelper->stationPhi(layid) << " ml " << m_idHelper->multilayer(layid) );
	exitcode = 10; m_ntuple->Fill();
	continue;
      }
 
      // surface
      const Trk::PlaneSurface& surf = detEl->surface(layid);

      // calculate the inclination angle 
      //Angle
      const Amg::Vector3D GloDire(hit.globalDirection().x(), hit.globalDirection().y(), hit.globalDirection().z());
      Trk::LocalDirection locDire;
      surf.globalToLocalDirection(GloDire, locDire);
      float inAngle_XZ = fabs( locDire.angleXZ() / CLHEP::degree);
      inAngle_XZ = 90. - inAngle_XZ ;
      //      inAngle_XZ = MM_READOUT [muonHelper->GetLayer(simId)-1]*inAngle_XZ ;
      ATH_MSG_DEBUG(" At eta " << m_idHelper->stationEta(layid) << " phi " << m_idHelper->stationPhi(layid) <<  "\n IncomingAngle: " <<  locDire.angleXZ() / CLHEP::degree << "\n inAngle_XZ, " << inAngle_XZ << " , " << inAngle_XZ * CLHEP::degree << "   ..   "  << CLHEP::degree );
 
      // compute hit position within the detector element/surfaces
      // Amg::Transform3D globalToLocal = detEl->absTransform().inverse();
      Amg::Vector3D hpos(hit.globalPosition().x(),hit.globalPosition().y(),hit.globalPosition().z());
      // Amg::Vector3D lpos = globalToLocal*hpos;
      // compute the hit position on the readout plane (same as in MuonFastDigitization)
      Amg::Vector3D slpos = surf.transform().inverse()*hpos;
      Amg::Vector2D posOnSurfUnProjected(slpos.x(),slpos.y());
      
      // double gasGapThickness = detEl->getDesign(layid)->gasGapThickness();
      
      Amg::Vector3D locdir(0., 0., 0.);
      if (MM_READOUT[m_idHelper->gasGap(layid)-1]==1) locdir = surf.transform().inverse().linear()*Amg::Vector3D(hit.globalDirection().x(), hit.globalDirection().y(), hit.globalDirection().z());
      else locdir = surf.transform().inverse().linear()*Amg::Vector3D(hit.globalDirection().x(), hit.globalDirection().y(), -hit.globalDirection().z());
	
      double scale, scaletop;
      double gasgap = 5.;
	   
      scale = -slpos.z()/locdir.z();
      scaletop = (gasgap+slpos.z())/locdir.z();
      // scaletop = (fabs(gasGapThickness) + slpos.z())/locdir.z();
      	
      Amg::Vector3D hitOnSurface = slpos + scale*locdir;
      Amg::Vector3D hitOnTopSurface = slpos + scaletop*locdir;
      Amg::Vector2D posOnSurf (hitOnSurface.x(), hitOnSurface.y());
      Amg::Vector2D posOnTopSurf (hitOnTopSurface.x(),hitOnTopSurface.y());

      // account for time offset
      double shiftTimeOffset = MMDigitTime* m_driftVelocity;
      Amg::Vector3D hitAfterTimeShift(hitOnSurface.x(),hitOnSurface.y(),shiftTimeOffset);
      Amg::Vector3D hitAfterTimeShiftOnSurface = hitAfterTimeShift - (shiftTimeOffset/locdir.z())*locdir;
	
      ATH_MSG_DEBUG("slpos.z " << slpos.z() << ", locdir " << locdir.z() << ", scale " << scale << ", hitOnSurface.z " << hitOnSurface.z() << ", hitOnTopSurface.z " << hitOnTopSurface.z() );

      if( fabs(hitOnSurface.z()) > 0.1 ) ATH_MSG_WARNING("bad propagation to surface " << hitOnSurface );
      if( fabs(hitAfterTimeShiftOnSurface.z()) > 0.1 ) ATH_MSG_WARNING("bad propagation to surface after time shift " << hitAfterTimeShiftOnSurface );
      
      m_n_Station_side = muonHelper->GetSide(simId);
      m_n_Station_eta = muonHelper->GetZSector(simId); 
      m_n_Station_phi = muonHelper->GetPhiSector(simId);
      m_n_Station_multilayer = muonHelper->GetMultiLayer(simId);
      m_n_Station_layer = muonHelper->GetLayer(simId); 
 
      if(hit.kineticEnergy()<m_energyThreshold && abs(hit.particleEncoding())==11) {
      	exitcode = 5;
      	m_ntuple->Fill();
      	continue;
      }
    
      // perform bound check
      if( !surf.insideBounds(posOnSurf) ){
	exitcode = 1;
      	m_ntuple->Fill();
      	continue;
      }
  		
      m_gasGap->Fill(m_n_Station_layer);
      m_gasGapDir->Fill(MM_READOUT [m_n_Station_layer-1]);
      
      int stripNumber = detEl->stripNumber(posOnSurf,layid);
      int LastStripNumber = detEl->stripNumber(posOnTopSurf, layid);
      if( stripNumber == -1 ){
	ATH_MSG_WARNING("!!! Failed to obtain strip number " << m_idHelper->print_to_string(layid) <<  "\n\t\t with pos " << posOnSurf 
			<< " z " << slpos.z() << " eKin: " << hit.kineticEnergy() << " eDep: " << hit.depositEnergy() << " unprojectedStrip: " << detEl->stripNumber(posOnSurfUnProjected, layid));
	exitcode = 2; m_ntuple->Fill();
	continue;
      }
      m_n_strip_multiplicity = LastStripNumber - stripNumber;
            
      
      // re-definition of ID
      Identifier parentID = m_idHelper->parentID(layid);
      Identifier DigitId = m_idHelper->channelID(parentID, m_idHelper->multilayer(layid), m_idHelper->gasGap(layid), stripNumber);
      // ATH_MSG_DEBUG(" re-definition of ID; " << m_idHelper->print_to_string(id) );
			
      int& counts = hitsPerChannel[DigitId];
      ++counts;
      if( counts > 1 ) {exitcode = 11; m_ntuple->Fill(); continue;}
      ++nhits;


      IdentifierHash hash;
      // contain (name, eta, phi, multiPlet)
      m_idHelper->get_detectorElement_hash(layid, hash);
      // ATH_MSG_DEBUG(" looking up collection using hash " << (int)hash << " " << m_idHelper->print_to_string(layid) );
      
      const MuonGM::MuonChannelDesign* mmChannelDes = detEl->getDesign(DigitId); 
      double distToChannel_withStripID = mmChannelDes->distanceToChannel(posOnSurf, stripNumber);
      double distToChannel = mmChannelDes->distanceToChannel(posOnSurf);
      ATH_MSG_DEBUG(" looking up collection using hash " << (int)hash << " " << m_idHelper->print_to_string(layid) << " DigitId: " << m_idHelper->print_to_string(DigitId));
  
      if ( fabs(distToChannel_withStripID - distToChannel) > mmChannelDes->channelWidth(posOnSurf)) {
	ATH_MSG_WARNING( "Found: distToChannel_withStripID: " << distToChannel_withStripID << " != distToChannel: " << distToChannel  );
	exitcode = 12; m_ntuple->Fill();
	continue;
      }
  
      // ATH_MSG_DEBUG( "Found: distToChannel_withStripID: " << distToChannel_withStripID << " ? distToChannel: " << distToChannel  );
 
      //starting the digitization
      const Identifier elemId = m_idHelper -> elementID(DigitId); //
      // ATH_MSG_DEBUG( "executeDigi() - element identifier is: " << m_idHelper->show_to_string(elemId) ); 

      //store local hit position + sign
      // ATH_MSG_DEBUG( " MmDigitToolInput create... " );
      const MmDigitToolInput StripdigitInput(stripNumber, distToChannel, inAngle_XZ, 0., detEl->numberOfStrips(layid)); 
      m_AngleDistr->Fill(inAngle_XZ);
      m_AbsAngleDistr->Fill(fabs(inAngle_XZ));
  
  
      // digitize input for strip response
      m_StripsResponse->set_stripWidth(mmChannelDes->channelWidth(posOnSurf));
      MmDigitToolOutput StripdigitOutput( m_StripsResponse->GetResponceFrom(StripdigitInput) );
      if(!StripdigitOutput.isValid()) continue;
      
      // digitize input for strip response
      m_StripsResponse->set_stripWidth(mmChannelDes->channelWidth(posOnSurf));
             
      // simulate strip response, check if strip fired
      const std::vector<int> & StripsResponse_stripPos = StripdigitOutput.stripPos();
      const std::vector<float> & StripsResponse_stripTime = StripdigitOutput.stripTime();
      const std::vector<float> & StripsResponse_stripCharge = StripdigitOutput.stripCharge();
      
      //ATH_MSG_DEBUG("posOnSurf       " << posOnSurf.x() << " " << posOnSurf.y() );
      //ATH_MSG_DEBUG( " Incoming at: " << stripNumber << " channelWidth: " << mmChannelDes->channelWidth(posOnSurf) << " ? " <<  m_StripsResponse->get_stripWidth());
      //float maxChargeFound(0.);
      float firstTimeFound(100000.);
      float lastTimeFound(0.);
      int posForTrigger(0), posForLast(0);
      for (int i=0; i<(int)StripsResponse_stripPos.size(); i++) {
	if ( (firstTimeFound > StripsResponse_stripTime[i]) && (StripsResponse_stripCharge[i] > m_qThresholdForTrigger)) {firstTimeFound = StripsResponse_stripTime[i]; posForTrigger = i;}
	if ( (lastTimeFound < StripsResponse_stripTime[i]) && (StripsResponse_stripCharge[i] > m_qThresholdForTrigger)) {lastTimeFound = StripsResponse_stripTime[i]; posForLast = i;}
	Amg::Vector2D tmpPos (posOnSurf.x()+ (StripsResponse_stripPos[i] - stripNumber )*0.425 , posOnSurf.y());
	//ATH_MSG_DEBUG( " For: " << i<< " we have :" << StripsResponse_stripPos[i] << "   ?   "<< mmChannelDes->channelNumber(tmpPos) << " with charge: " << StripsResponse_stripCharge[i] << " time: " << StripsResponse_stripTime[i] << " timeForTrig: " << firstTimeFound);
      }
      m_n_strip_multiplicity_2 = MM_READOUT [muonHelper->GetLayer(simId)-1]*StripsResponse_stripPos[StripsResponse_stripPos.size()-1] - (-MM_READOUT [muonHelper->GetLayer(simId)-1])*StripsResponse_stripPos[0];
			
      //ATH_MSG_DEBUG("Final: MM_READOUT ["<< m_n_Station_layer-1 <<"] "<< MM_READOUT [m_n_Station_layer-1] << "  ??  " << m_n_strip_multiplicity << " from " <<  stripNumber << " to " << LastStripNumber << " while: " << StripsResponse_stripPos[0] << " to " << StripsResponse_stripPos[StripsResponse_stripPos.size()-1]);
      
      StripdigitOutput.set_StripForTrigger(StripsResponse_stripPos[posForTrigger]);
      StripdigitOutput.set_StripTimeForTrigger(StripsResponse_stripTime[posForTrigger]);
      m_ClusterLength->Fill(StripsResponse_stripPos[StripsResponse_stripPos.size()-1] - StripsResponse_stripPos[0] );
      m_ClusterLength2D->Fill(StripsResponse_stripPos[posForLast] - StripsResponse_stripPos[posForTrigger]);
			
      m_n_hitStripID=stripNumber;
      m_n_StrRespTrg_ID=StripsResponse_stripPos[posForTrigger];
      m_n_hitDistToChannel=distToChannel; 
      m_n_hitIncomingAngle=inAngle_XZ; 
      m_n_hitIncomingAngleRads = inAngle_XZ * CLHEP::degree;
      m_n_StrRespTrg_Time=StripsResponse_stripTime[posForTrigger];
      m_n_hitOnSurface_x=posOnSurf.x();
      m_n_hitOnSurface_y = posOnSurf.y();
  		
      m_n_StrRespID = StripsResponse_stripPos;
      m_n_StrRespCharge = StripsResponse_stripCharge; 
      m_n_StrRespTime = StripsResponse_stripTime;
  		
      m_ntuple->Fill();
  		
      //ATH_MSG_DEBUG( " Cluster length: " << StripsResponse_stripPos[StripsResponse_stripPos.size()-1] - StripsResponse_stripPos[0] );
			
      const MmElectronicsToolInput ElectronicDigitInput( StripdigitOutput.stripPos(), StripdigitOutput.stripCharge(), StripdigitOutput.stripTime() );       
      MmDigitToolOutput ElectronicOutput( m_ElectronicsResponse->GetResponceFrom(ElectronicDigitInput) );

      if(!ElectronicOutput.isValid()) ATH_MSG_WARNING ( "MmDigitizationTool::doDigitization() -- there is no electronics response even though there is a strip response." );

      /* unused variables to prevent compilation warnings */
      /*

	const std::vector<int> & chipResponsePos = ElectronicOutput.stripPos();
	const std::vector<float> & chipResponseTime = ElectronicOutput.stripTime();
	const std::vector<float> & chipResponseCharge = ElectronicOutput.stripCharge(); 
      */
      
      // The collections should use the detector element hashes not the module hashes to be consistent with the PRD granularity. 
      IdentifierHash detIdhash ;
      // set RE hash id 
      int gethash_code = m_idHelper->get_detectorElement_hash(elemId, detIdhash);
      if (gethash_code != 0) {
	ATH_MSG_ERROR ( "MmDigitizationTool --  collection hash Id NOT computed for id = " << m_idHelper->show_to_string(elemId) ); 
	// continue;
      } 
      
      MmDigit*  newDigit = new MmDigit(DigitId, StripdigitOutput.stripTime(), StripdigitOutput.stripPos(), StripdigitOutput.stripCharge(), 
				       ElectronicOutput.stripTime(),ElectronicOutput.stripPos(),ElectronicOutput.stripCharge(),
				       StripdigitOutput.stripForTrigger(), StripdigitOutput.stripTimeForTrigger()  );
      
      //ATH_MSG_DEBUG ( "strip and chip response digit created" );

      MmDigitCollection* digitCollection = 0;
 	
      // put new collection in storegate
      // Get the messaging service, print where you are
      m_activeStore->setStore( &*m_sgSvc );
      MmDigitContainer::const_iterator it_coll = m_digitContainer->indexFind(detIdhash);
      if (m_digitContainer->end() ==  it_coll) {
	digitCollection = new MmDigitCollection(elemId, detIdhash);
	digitCollection->push_back(newDigit);
	m_activeStore->setStore( &*m_sgSvc );
	StatusCode status = m_digitContainer->addCollection(digitCollection, detIdhash);
	// ATH_MSG_DEBUG ( "MM digitCollection status: " << status );
	if (status.isFailure()) {
	  ATH_MSG_ERROR ( "Couldn't record MicroMegas DigitCollection with key=" << detIdhash  << " in StoreGate!" );
	  // delete  digitCollection;
	  // digitCollection    = 0 ;
	  // return StatusCode::RECOVERABLE; // consistent with ERROR message above. 
	}
	//else ATH_MSG_DEBUG ( "New MicroMegas DigitCollection with key=" << detIdhash << " recorded in StoreGate." );
      }
      else {
	digitCollection = const_cast<MmDigitCollection*>( it_coll->cptr() );
	digitCollection->push_back(newDigit);
      }
        
      // fill the SDO collection in StoreGate
      // create here deposit for MuonSimData, link and tof
      ///MuonSimData::Deposit deposit(HepMcParticleLink(phit->trackNumber(), phit.eventId()), MuonMCData(G4Time, 0));
      MuonSimData::Deposit deposit(hit.particleLink(), MuonMCData(hitOnSurface.x(), hitOnSurface.y()));
      std::vector<MuonSimData::Deposit> deposits;
      deposits.push_back(deposit);
      m_sdoContainer->insert(std::make_pair(DigitId, MuonSimData(deposits, inAngle_XZ * CLHEP::degree*1000)));
         
      // ATH_MSG_DEBUG( " MmDigitToolInput outsideWindow: " << outsideWindow(m_bunchTime) ); 
      // it should be decided by bunchtime. -400 to +200
      // if (outsideWindow(m_bunchTime)) continue;

      //digitCollection = getDigitCollection(elemId);
      //ATH_MSG_DEBUG ( "Finished MmDigitizationTool::MM digit and SDO stored in the DigitCollection" );
      previousHit = &hit;
    }//while(i != e)
  }//while(m_thpcMM->nextDetectorElement(i, e))
  // reset the pointer if it is not null

  ATH_MSG_DEBUG ( "MmDigitization Done!"  );

  if (m_thpcMM){
    delete m_thpcMM;
    m_thpcMM = 0;
  }
	     
  return StatusCode::SUCCESS;
} 
/*******************************************************************************/
bool MmDigitizationTool::checkMMSimHit( const GenericMuonSimHit& /*hit*/ ) const {
  // ATH_MSG_DEBUG ("MmDigitizationTool::checkMMSimHit()");
  /*
  //get the hit Identifier and info
  const int simId = hit.GenericId();  
  std::string stationName = muonHelper->GetStationName(simId);
  int stationEta = muonHelper->GetZSector(simId);
  int stationPhi  = muonHelper->GetPhiSector(simId);
  int multilayer = muonHelper->GetMultiLayer(simId); // mmMultiplet in MmIdHelpers
  int layer = muonHelper->GetLayer(simId); // mmGasGap in MmIdHelpers
  int side = muonHelper->GetSide(simId);
         
  //  ATH_MSG_VERBOSE("Micromegas hit: r " << hit.globalPosition().perp() << " z " << hit.globalPosition().z() << " mclink " << hit.particleLink()
  //		  << " stName " << stationName << " eta " << stationEta << " phi " << stationPhi << " ml " << multilayer << " layer " << layer << " side " << side );
  
  // the G4 time or TOF from IP
  double MM_Hit_globalTime = hit.globalTime() ;
  double MM_Hit_globalPreTime = hit.globalpreTime() ;

  double timeOfFlight = 0.5* ( MM_Hit_globalPreTime - MM_Hit_globalTime );

  // extract the distance to the origin of the module to Time of flight 
  // Use the coordinate of the center of the module to calculate the time of flight  
  Amg::Vector3D distance = 0.5* ( hit.globalPrePosition() + hit.globalPosition() );
  double m_time = timeOfFlight/CLHEP::ns - distance.mag()/CLHEP::c_light/CLHEP::ns;
 
  ATH_MSG_VERBOSE(     "Micromegas hit  "
  << " MM_Hit_globalPreTime ["  << MM_Hit_globalPreTime  << "]"
  << " MM_Hit_globalTime ["  << MM_Hit_globalTime  << "]" 
  << " timeOfFlight ["   << timeOfFlight  << "]"
  << " point  ["        << distance       << "]"
  << " m_time ["         << m_time     << "]" );
  */
  
  return true;
}     
/*******************************************************************************/
int MmDigitizationTool::digitizeTime(double time) const {

  int    tdcCount;
  double tmpCount = time/m_ns2TDC;
  double rand = CLHEP::RandGaussZiggurat::shoot(m_rndmEngine, tmpCount, m_resTDC);
  tdcCount = static_cast<long>(rand);
   
  if (tdcCount < 0 || tdcCount > 4096){
    //ATH_MSG_DEBUG( " Count outside TDC window: " << tdcCount );
  }
   
  return tdcCount;
}
/*******************************************************************************/
