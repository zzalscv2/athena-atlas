/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** R.Goncalo - 21/10/2007 - add tests for TrigDecisionTool:
TrigDecisionChecker based on TrigDecisionMaker/TrigDecisionTest */

#include "TrigValAlgs/TrigDecisionChecker.h"
//#include "TrigSteering/Chain.h"

// EDM
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "TrigParticle/TrigTauContainer.h"
#include "TrigCaloEvent/TrigTauClusterContainer.h"
#include "TrigInDetEvent/TrigTauTracksInfoCollection.h"
#include "TrigMuonEvent/CombinedMuonFeature.h"

// include these tau navigation check
#include "TrigParticle/TrigTau.h"
//#include "TrigCaloEvent/TrigTauCluster.h"
#include "tauEvent/TauJet.h"
#include "tauEvent/TauJetContainer.h"
#include "tauEvent/TauDetailsContainer.h"
#include "tauEvent/Tau1P3PDetails.h"
#include "tauEvent/TauRecDetails.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTrigBphys/TrigBphysContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODJet/JetContainer.h"

#include "TrigParticle/TrigEFBjetContainer.h"
#include "Particle/TrackParticleContainer.h"
#include "TrigInDetEvent/TrigVertexCollection.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "VxSecVertex/VxSecVKalVertexInfo.h"

#include "TrigConfigSvc/DSConfigSvc.h"

// Gaudi inlcudes
//#include "AthenaBaseComps/AthAlgorithm.h"
//#include "GaudiKernel/ServiceHandle.h"
#include "SGTools/crc64.h"

//#include "AthenaKernel/IUserDataSvc.h"
//#include "AthenaKernel/IUserDataSvc.h"

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"


/*#include "xAODTrigMinBias/TrigSpacePointCountsContainer.h"
/#include "xAODTrigMinBias/TrigT2MbtsBitsContainer.h"
/#include "xAODTrigMinBias/TrigVertexCountsContainer.h"
/#include "xAODTrigMinBias/TrigTrackCountsContainer.h"*/
#include "xAODTrigMinBias/TrigSpacePointCounts.h"
#include "xAODTrigMinBias/TrigT2MbtsBits.h"
#include "xAODTrigMinBias/TrigVertexCounts.h"
#include "xAODTrigMinBias/TrigTrackCounts.h"

// tools
#include "MuonCombinedToolInterfaces/IMuonPrintingTool.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

TrigDecisionChecker::TrigDecisionChecker(const std::string &name, ISvcLocator *pSvcLocator)
  : AthAlgorithm(name, pSvcLocator),
    m_trigDec("Trig::TrigDecisionTool/TrigDecisionTool"),
    m_storeGate("StoreGateSvc", name),
    m_configSvc( "TrigConf::TrigConfigSvc/TrigConfigSvc", name ),
    m_dsSvc( "TrigConf::DSConfigSvc/DSConfigSvc", name ),
    m_muonPrinter("Rec::MuonPrintingTool/MuonPrintingTool")
{
  // default for muon chains
  m_muonItems.push_back("HLT_mu26_imedium");

    // dc14 bphysics menu items - can be moved into JobOptions if required
    m_bphysItems.push_back("HLT_2mu10_bBmumu");
    m_bphysItems.push_back("HLT_2mu10_bBmumux_BcmumuDsloose");
    m_bphysItems.push_back("HLT_2mu10_bBmumuxv2");
    m_bphysItems.push_back("HLT_2mu10_bJpsimumu");
    m_bphysItems.push_back("HLT_2mu10_bUpsimumu");
    m_bphysItems.push_back("HLT_2mu6_bBmumu");
    m_bphysItems.push_back("HLT_2mu6_bBmumux_BcmumuDsloose");
    m_bphysItems.push_back("HLT_2mu6_bBmumuxv2");
    m_bphysItems.push_back("HLT_2mu6_bDimu");
    m_bphysItems.push_back("HLT_2mu6_bDimu_novtx_noos");
    m_bphysItems.push_back("HLT_2mu6_bJpsimumu");
    m_bphysItems.push_back("HLT_2mu6_bUpsimumu");
    m_bphysItems.push_back("HLT_3mu6_bDimu");
    m_bphysItems.push_back("HLT_3mu6_bJpsi");
    m_bphysItems.push_back("HLT_3mu6_bTau");
    m_bphysItems.push_back("HLT_mu13_mu13_idperf_Zmumu");
    m_bphysItems.push_back("HLT_mu4_iloose_mu4_7invm9_noos");
    m_bphysItems.push_back("HLT_mu4_mu4_idperf_bJpsimumu_noid");
    m_bphysItems.push_back("HLT_mu6_bJpsi_Trkloose");

    // dc14 tau menu items - can be moved into JobOptions if required      
    m_TauItems.push_back("HLT_e18_loose1_tau25_medium1_calo");
    m_TauItems.push_back("HLT_tau25_medium1_mvonly");
    m_TauItems.push_back("HLT_tau125_r1perf");
    m_TauItems.push_back("HLT_tau80_medium1_calo");
    m_TauItems.push_back("HLT_tau20_r1perf_idperf");
    m_TauItems.push_back("HLT_e18_loose1_tau80_medium1_ptonly");
    m_TauItems.push_back("HLT_e18_loose1_tau80_medium1_calo");
    m_TauItems.push_back("HLT_e18_lhloose_tau25_medium1_ptonly");
    m_TauItems.push_back("HLT_mu14_tau25_medium1_ptonly");
    m_TauItems.push_back("HLT_tau25_medium1_mvonly_L1TAU6");
    m_TauItems.push_back("HLT_tau80_medium1_track");
    m_TauItems.push_back("HLT_tau125_medium1_track");
    m_TauItems.push_back("HLT_tau25_medium1_trackonly");
    m_TauItems.push_back("HLT_tau29_r1perf");
    m_TauItems.push_back("HLT_mu14_tau35_medium1_ptonly");
    m_TauItems.push_back("HLT_tau25_medium1_ptonly");
    m_TauItems.push_back("HLT_tau35_r1perf");
    m_TauItems.push_back("HLT_mu14_tau25_medium1_calo");
    m_TauItems.push_back("HLT_tau125_r1medium1");
    m_TauItems.push_back("HLT_tau35_medium1_calo_tau25_medium1_calo");
    m_TauItems.push_back("HLT_e18_lhloose_tau80_medium1_calo");
    m_TauItems.push_back("HLT_tau25_medium1_track");
    m_TauItems.push_back("HLT_tau35_medium1_ptonly");
    m_TauItems.push_back("HLT_tau25_r1perf_L1TAU6");
    m_TauItems.push_back("HLT_e18_lhloose_tau80_medium1_ptonly");
    m_TauItems.push_back("HLT_tau125_medium1_calo");
    m_TauItems.push_back("HLT_tau35_medium1_ptonly_tau25_medium1_ptonly_xe50");
    m_TauItems.push_back("HLT_e18_loose1_tau25_medium1_ptonly");
    m_TauItems.push_back("HLT_mu14_tau35_medium1_calo");
    m_TauItems.push_back("HLT_tau35_medium1_calo_tau25_medium1_calo_xe50");
    m_TauItems.push_back("HLT_tau25_medium1_caloonly");
    m_TauItems.push_back("HLT_tau25_medium1_calo");
    m_TauItems.push_back("HLT_tau35_medium1_ptonly_tau25_medium1_ptonly");
    m_TauItems.push_back("HLT_tau20_r1medium1");
    m_TauItems.push_back("HLT_e18_lhloose_tau25_medium1_calo");
    m_TauItems.push_back("HLT_tau35_medium1_calo");
    m_TauItems.push_back("HLT_tau35_medium1_ptonly_xe70_L1XE45");
    m_TauItems.push_back("HLT_tau25_r1perf");
    m_TauItems.push_back("HLT_tau20_r1perf");
    m_TauItems.push_back("HLT_tau29_r1medium1");
    m_TauItems.push_back("HLT_tau35_medium1_track");
    m_TauItems.push_back("HLT_tau35_medium1_calo_xe70_L1XE45");


  declareProperty("TrigDecisionKey",   m_trigDecisionKey = "TrigDecision");
  declareProperty("TrigDecisionTool",  m_trigDec, "The tool to access TrigDecision");
  declareProperty("MonitoredChains",   m_count_sig_names );
  //declareProperty("MonitoredTauChain", m_tauItem = "tau20_r1medium1");
  declareProperty("MonitoringBlock",   m_monitoring_block_size = 100);
  declareProperty("WriteEventDecision",m_event_decision_printout = true);
  declareProperty("WriteOutCounts",    m_printout_counts   = false);
  declareProperty("WriteOutFilename",  m_printout_filename = "trigger_counts.log");
  declareProperty("EventInfoName",     m_eventInfoName="", "The name of the EventInfo container" );
  declareProperty("SMK",               m_smKey=0, "The super master key to use" );
  declareProperty("MuonItems",         m_muonItems, "Muon triggers to test");
  declareProperty("BjetItems",         m_bjetItems, "Bjet triggers to test");
  declareProperty("BphysicsItems",         m_bphysItems, "Bphysics triggers to test");
  declareProperty("ElectronItems",     m_electronItems, "Electron triggers to test");
  declareProperty("PhotonItems",       m_photonItems, "Photon triggers to test");
  declareProperty("TauItems",       m_TauItems, "Tau triggers to test");
  declareProperty("MinBiasItems",      m_minBiasItems, "MinBias triggers to test");
  declareProperty("JetItems",       m_jetItems, "Jet triggers to test");

}


TrigDecisionChecker::~TrigDecisionChecker() {}


StatusCode TrigDecisionChecker::initialize()
{
  // reset event counters
  m_first_event=true;
  m_event_number=0;
  m_mu_sum=0.0;

  // Reset keys
  m_smk=0;
  m_l1psk=0;
  m_hltpsk=0;
  
  // print out properties
  msg(MSG::INFO) << "Initializing..." << endreq;
  msg(MSG::INFO) << "TrigDecisionKey:    " << m_trigDecisionKey << endreq;
  msg(MSG::INFO) << "MonitoredChains:    " << m_count_sig_names << endreq;
  //msg(MSG::INFO) << "MonitoredTauItem:   " << m_tauItem << endreq;
  msg(MSG::INFO) << "MonitoringBlock:    " << m_monitoring_block_size<< endreq;
  msg(MSG::INFO) << "PrintOutCounts:     " << m_printout_counts << endreq;
  if (m_printout_counts) {
    msg(MSG::INFO) << "PrintOutFilename:   " << m_printout_filename << endreq;
  }
  msg(MSG::INFO) << "SuperMasterKey:     " << m_smKey << endreq;
    
  // get handle to StoreGate service
  StatusCode sc = m_storeGate.retrieve();
  if ( sc.isFailure() ) {
    msg(MSG::ERROR) << "Could not retrieve StoreGateSvc!" << endreq;
    return sc;
  }
    
  // Retrieve the trigger configuration service
  // get handle to trigger configuration
  sc = m_configSvc.retrieve();
  if ( sc.isFailure() ) {
    msg(MSG::ERROR) << "Could not retrieve Trigger configuration!" << endreq;
    return sc;
  }

  // get handle to TrigDecisionTool
  sc = m_trigDec.retrieve();
  if ( sc.isFailure() ) {
    msg(MSG::ERROR) << "Could not retrieve TrigDecisionTool!" << endreq;
    return sc;
  }
  m_trigDec->ExperimentalAndExpertMethods()->enable();

  // reserve space for vectors
  m_summary.reserve(700);
  m_summary_chain_passraw.reserve(700);
  m_summary_chain_passraw.reserve(700);
  m_summary_chain_PT.reserve(700);
  m_summary_chain_PS.reserve(700);
  m_summary_passraw.reserve(700);
  m_summary_pass.reserve(700);
  m_summary_passphys.reserve(700);
  m_chain_prescales.reserve(700);
  m_chain_prescales_calculated.reserve(700);
  m_chain_passthrough.reserve(700);
  m_chain_passthrough_calculated.reserve(700);
  m_lower_chain_accept.reserve(700);

  // initialize vectors for chains to monitor
  msg(MSG::INFO) << "Monitoring number of events passing these chains (blocks of "
	   << m_monitoring_block_size << " events):"<< endreq;
  for (unsigned int i=0; i < m_count_sig_names.size(); ++i) {
    msg(MSG::INFO) << "Initializing monitoring counters for " << m_count_sig_names[i] << endreq;
    m_count_sigs.push_back(new std::vector<int>());
    m_run_count_sigs.push_back(0);
  }
 
  // retrieve muon printing tool
  sc = m_muonPrinter.retrieve();
  if(sc.isFailure()) {
    ATH_MSG_ERROR("Could not retrieve MuonPrinter tool");
    return sc;
  }
 
  msg(MSG::INFO) << "Initialization successful" << endreq;


  return StatusCode::SUCCESS;
}


StatusCode TrigDecisionChecker::finalize()
{

  // print summary of trigger decisions for each level
  msg(MSG::INFO) << "==========================================================" << endreq;
  msg(MSG::INFO) << "TrigDecisionTool summary:" << endreq;
  msg(MSG::INFO) << "==========================================================" << endreq;

  // LVL1
  std::map<std::string,int>::iterator itL1,itL1End=m_L1_summary.end();
  for ( itL1=m_L1_summary.begin(); itL1!=itL1End; ++itL1 ) {
    msg(MSG::INFO) << "REGTEST item " << (*itL1).first << " accepted events=" << (*itL1).second << endreq;
  }

  // HLT
  for ( unsigned int i=0; i<m_summary.size(); ++i) {
    msg(MSG::INFO) << "REGTEST chain " << m_summary[i] << " accepted events= " << m_summary_passphys[i]
	     <<" ( PS: " << m_summary_chain_PS[i] << " , PT: " << m_summary_chain_PT[i] << ")" << endreq;
  }

  // print out nr. of events passed in blocks of N events for specific chains (configurable)
  msg(MSG::INFO) << " " << endreq;
  msg(MSG::INFO) << "TrigDecisionTool tests: monitored chain efficiency" << endreq;
  msg(MSG::INFO) << "==================================================" << endreq;
  msg(MSG::INFO) << "REGTEST  Nr.events: ";
  for (unsigned int k=0; k<m_count_event.size();++k) {
    msg() << m_count_event[k] << " ";
  }
  msg() << endreq;
  for (unsigned int i=0; i<m_count_sig_names.size();++i) {
    msg(MSG::INFO) << "REGTEST " << m_count_sig_names[i] << " : ";
    for (unsigned int j=0; j<(m_count_sigs[i])->size();++j) {
      msg() << (*m_count_sigs[i])[j] << " ";
    }
    msg() << endreq;
  }

  // compare prescale and passthrough fractions from configuration and by counting events
  msg(MSG::INFO) << " " << endreq;
  msg(MSG::INFO) << "TrigDecisionTool tests: chain prescale/passthrough " << endreq;
  msg(MSG::INFO) << "==================================================" << endreq;
  // HLT
  msg(MSG::INFO) << "REGTEST Chain (passed raw) PS(conf) PS(counts) PT(conf) PT(counts) (Lower chain)";
  for ( unsigned int i=0; i<m_summary.size(); ++i) {
    if (m_lower_chain_accept[i]!=0) {
      msg(MSG::INFO) << "REGTEST " << m_summary[i] << " (" << m_summary_chain_PT[i] << ") \t"
	       << " \t" << m_chain_prescales[i] << " \t" << m_chain_prescales_calculated[i] ///m_lower_chain_accept[i]
	       << " \t" << m_chain_passthrough[i] << " \t" << m_chain_passthrough_calculated[i] ///m_lower_chain_accept[i]
	       << " (" << m_lower_chain_accept[i] << ")" << endreq;
    } else {
      msg(MSG::INFO) << "REGTEST " << m_summary[i] << " (" << m_summary_chain_PT[i] << ") \t"
	       << " \t" << m_chain_prescales[i] << "   ---   "
	       << " \t" << m_chain_passthrough[i] << "   ---   "
	       << " (lower chain passed no events)" << endreq;
    }
  }
  msg(MSG::INFO) << "REGTEST ran on " << m_event_number << " events" << endreq;
  msg(MSG::INFO) << "Average mu value " << m_mu_sum/float(m_event_number) << endreq;

  // open output file and write out counts
  if (m_printout_counts) {
    msg(MSG::INFO) << "==================================================" << endreq;
    msg(MSG::INFO) << "Opening " << m_printout_filename << " for writing trigger counts" << endreq;
    m_printout_file.open(m_printout_filename.c_str());
    
    /*
      m_L1_summary - isPassed for L1
      m_summary_chain_passraw  - chainPassedRaw for HLT     
      m_summary_pass     - chain Passed for HLT     
      m_summary_passphys - chain Physics Passed for HLT     
      m_summary_chain_PS - chainPassedRaw && !isPrescaled for HLT     
      m_summary_chain_PT - (chainPassedRaw && !isPrescaled) || isPassedThrough for HLT     
    */

    m_printout_file  << std::setiosflags(std::ios::left) << std::setw(25) << "* L1 item"
		     << std::setw(15) << " #Passed " << std::endl;
    std::map<std::string,int>::iterator l1,l1End=m_L1_summary.end();
    for ( l1=m_L1_summary.begin(); l1!=l1End; ++l1 ) {
      m_printout_file << std::setiosflags(std::ios::left) << (*l1).first 
		      << std::setw(10) << (*l1).second << std::endl;
    }

    m_printout_file  << std::setiosflags(std::ios::left) << std::setw(25) << "* Chain name"
		     << std::setw(15) << " #Passed Raw "
		     << std::setw(15) << " #passed  "
		     << std::setw(15) << " #Physics Passed "<< std::endl;
    for ( unsigned int i=0; i<m_summary.size(); ++i) {
      m_printout_file << std::setiosflags(std::ios::left) << std::setw(25) << m_summary[i] 
		      << std::setw(15) << m_summary_passraw[i] 
		      << std::setw(15) << m_summary_pass[i] 
		      << std::setw(15) << m_summary_passphys[i] << std::endl;
      
      //    std::cout << std::setiosflags(std::ios::dec) << std::setw(6) << i << "|"
      // 	      << std::setiosflags(std::ios::dec) << std::setw(4) << (*(m_elink_vec[i]))->algorithmId() << "|"
      // 	      << std::setw(11) << *(m_elink_vec[i]) << "|";
    }
    msg(MSG::INFO) << "Closing output file " << m_printout_filename << endreq;
    m_printout_file.close();
  }
  
  // cleanup newed vectors
  msg(MSG::INFO) << "==================================================" << endreq;
  msg(MSG::INFO) << "Cleaning up counters..." << endreq;
  for (unsigned int i=0; i<m_count_sigs.size(); ++i) {
    delete m_count_sigs[i];
  }
  msg(MSG::INFO) << "Finalised successfully" << endreq;
  
  return StatusCode::SUCCESS;
}

uint32_t TrigDecisionChecker_old_smk=0;

StatusCode TrigDecisionChecker::execute()
{
  printf("TDC::execute called!\n");
  // Fill the variables:
  m_smk = m_configSvc->masterKey();
  m_l1psk = m_configSvc->lvl1PrescaleKey();
  m_hltpsk = m_configSvc->hltPrescaleKey();
    
  // If the keys returned by the configuration service don't seem to make sense,
  // use something else as the SMK. (Needed mostly for MC test jobs.)
  if( ( ( m_smk == 0 ) && ( m_l1psk == 0 ) && ( m_hltpsk == 0 ) ) ||
     ( static_cast< int >( m_smk )    < 0 ) ||
     ( static_cast< int >( m_l1psk )  < 0 ) ||
     ( static_cast< int >( m_hltpsk ) < 0 ) ) {

    // See if we are reading an AOD:
    if( ! m_dsSvc ) {
      msg() <<  MSG::FATAL
        << "The trigger configuration keys don't seem to make sense, and we're not using "
        << "TrigConf::DSConfigSvc...";
      return StatusCode::FAILURE;
    }
    TrigConf::DSConfigSvc* dsSvc = dynamic_cast< TrigConf::DSConfigSvc* >( m_dsSvc.operator->() );
    if( ! dsSvc ) {
      msg() <<  MSG::FATAL
        << "The trigger configuration keys don't seem to make sense, and we're not using "
        << "TrigConf::DSConfigSvc...";
      return StatusCode::FAILURE;
    }
    
    // Turn the configuration source name (probably an XML file in this case) into an
    // imaginary Super Master Key:
    m_smk = SG::crc64( dsSvc->configurationSource() ) & 0xffff;
    m_l1psk = 0;
    m_hltpsk = 0;
  }
  
  if(TrigDecisionChecker_old_smk!=m_smk) {
    printf("New SMK found = %d\n",m_smk);
    TrigDecisionChecker_old_smk=m_smk;
  }
  printf("SMK = %d\n",m_smk);

  // Check to see whether this is an event which we should process
  if(m_smk!=m_smKey && m_smKey!=0) {
    // We do not have a matching super master key so skip the event and return success
    return StatusCode::SUCCESS;
  }
  
  m_event_number++;
  
  // check mu value
  const EventInfo* eventInfo;
  //#sc = evtStore()->retrieve(eventInfo, m_eventInfoName);
  StatusCode sc;
  if (m_eventInfoName == "") {
    sc=m_storeGate->retrieve(eventInfo);
  } else {
    sc=m_storeGate->retrieve(eventInfo ,m_eventInfoName);
  }
  if ( sc.isFailure() || !eventInfo )
     {
         msg(MSG::INFO)  << "Container '" << m_eventInfoName
                                    << "' could not be retrieved from StoreGate !" << endreq;
         sc = StatusCode::SUCCESS;
         return sc;
  } else {
          msg(MSG::DEBUG) << "Container '" << m_eventInfoName
                               << "' retrieved from StoreGate" << endreq;
  }

  if ( eventInfo ) {
       float mu = eventInfo->actualInteractionsPerCrossing();
       float muave =  eventInfo->averageInteractionsPerCrossing(); 
       msg(MSG::INFO) << "run number  " << eventInfo->event_ID()->run_number() << " event number " << eventInfo->event_ID()->event_number() << 
                                " lumi block " << eventInfo->event_ID()->lumi_block() << endreq;
       msg(MSG::INFO) << "mu value " << mu << " average mu value " << muave <<  " event number " << m_event_number <<  endreq;
       m_mu_sum = m_mu_sum + eventInfo->actualInteractionsPerCrossing();
  }

 
   for(auto tauItem : m_TauItems) {
    ATH_MSG_INFO("Check tau items " << tauItem);
    sc = checkTauEDM(tauItem);
    if ( sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not finish checkTauEDM test for chain " <<tauItem << endreq;
      return sc;
    }
  }
  
   for(auto muonItem : m_muonItems) {
    sc = checkMuonEDM(muonItem);
    if ( sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not finish checkMuonEDM test for chain " << muonItem << endreq;
      return sc;
    }
  }

  for(auto bjetItem : m_bjetItems) {
    sc = checkBjetEDM(bjetItem);
    if ( sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not finish checkBjetEDM test for chain " << bjetItem << endreq;
      return sc;
    }
  }

    for(auto bphysItem : m_bphysItems) {
        sc = checkBphysEDM(bphysItem);
        if ( sc.isFailure() ) {
            msg(MSG::ERROR) << "Could not finish checkBphysEDM test for chain " << bphysItem << endreq;
            // return sc; try not to return for other tests to run
        }
    }

  for(auto electronItem : m_electronItems) {
      ATH_MSG_INFO("Check Electron items " << electronItem);
    sc = checkElectronEDM(electronItem);
    if ( sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not finish checkElectronEDM test for chain " << electronItem << endreq;
      return sc;
    }
  }

  for(auto photonItem : m_photonItems) {
    sc = checkPhotonEDM(photonItem);
    if ( sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not finish checkPhotonEDM test for chain " << photonItem << endreq;
      return sc;
    }
  }

  for(auto minBiasItem : m_minBiasItems) {
    sc = checkMinBiasEDM(minBiasItem);
    if ( sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not finish checkMinBiasEDM test for chain " << minBiasItem << endreq;
      return sc;
    }
  }
   
  ATH_MSG_INFO("REGTEST ==========START of Jet EDM/Navigation check===========");
  for(auto jetItem : m_jetItems) {
      sc = checkJetEDM(jetItem);
      if ( sc.isFailure() ) {
          ATH_MSG_INFO("REGTEST Could not finish checkJetEDM test for chain " << jetItem);
          return sc;
      }
  }
  ATH_MSG_INFO("REGTEST ==========END of Jet EDM/Navigatinon check===========");
    
    
  if (m_event_decision_printout) {
    msg(MSG::INFO) << "TrigDecisionChecker::execute" << endreq;

    msg(MSG::INFO) << "Pass state     = " << m_trigDec->isPassed("EF_.*") << endreq;
    msg(MSG::INFO) << "Pass state L1  = " << m_trigDec->isPassed("L1_.*") << endreq;
    msg(MSG::INFO) << "Pass state L2  = " << m_trigDec->isPassed("L2_.*") << endreq;
    msg(MSG::INFO) << "Pass state EF  = " << m_trigDec->isPassed("EF_.*") << endreq;
    msg(MSG::INFO) << "Pass state HLT = " << m_trigDec->isPassed("HLT_.*") << endreq;
  }

  Trig::ExpertMethods* em = m_trigDec->ExperimentalAndExpertMethods();

  // L1
  std::vector<std::string> allItems = m_trigDec->getListOfTriggers("L1_.*");
  if (!allItems.empty()) {
    if (m_event_decision_printout) msg(MSG::INFO) << "Items : " << allItems.size() << endreq;
    
    for (std::vector<std::string>::const_iterator itemIt = allItems.begin();
         itemIt != allItems.end(); ++itemIt) {
      
      const LVL1CTP::Lvl1Item* aItem = em->getItemDetails(*itemIt);

      if (!aItem) continue;
      if (aItem->name()=="") continue;

      if (m_event_decision_printout) msg(MSG::INFO) << "Item " << aItem->name() << " : Item ID " << aItem->hashId() << " : " << aItem->isPassed() << endreq;

      // fill bookkeeping map with zeros if first event
      std::string item_name = aItem->name();
      if (m_first_event) m_L1_summary[item_name] = 0;

      // increment counter for L1 summary
      if (aItem->isPassed()) m_L1_summary[item_name] = (m_L1_summary[item_name]+1);
      int count = (m_L1_summary.find(item_name))->second;
      if (m_event_decision_printout) msg(MSG::INFO) << "L1_map[" << item_name << "] = " << count << endreq;


    }
  } else {
      msg(MSG::WARNING) << "Could not retrieve L1 Items !!" << endreq;
  }


  // HLT Chains

  // Get all configured chain names from config
  std::vector<std::string> confChains = m_trigDec->getListOfTriggers("L2_.*, EF_.*, HLT_.*");
  msg(MSG::INFO) << "Configuring for " << confChains.size() << " HLT chain counters" << endreq;

  // resize & initialise counters in first event
  if (m_first_event) {
    m_summary.resize(0); // vector for chain names
    m_summary_passraw.resize(confChains.size(),0);
    m_summary_pass.resize(confChains.size(),0);
    m_summary_passphys.resize(confChains.size(),0);
    m_summary_chain_passraw.resize(confChains.size(),0);
    m_summary_chain_pass.resize(confChains.size(),0);
    m_summary_chain_passphys.resize(confChains.size(),0);
    m_summary_chain_PT.resize(confChains.size(),0);
    m_summary_chain_PS.resize(confChains.size(),0);
    m_chain_prescales.resize(confChains.size(),0);
    m_chain_prescales_calculated.resize(confChains.size(),0);
    m_chain_passthrough.resize(confChains.size(),0);
    m_chain_passthrough_calculated.resize(confChains.size(),0);
    m_lower_chain_accept.resize(confChains.size(),0);

    // make vector of names and sort alphabetically
    std::map<std::string,float> t_pt_map;
    std::map<std::string,float> t_ps_map;

    for(std::vector<std::string>::iterator chIter = confChains.begin(); chIter != confChains.end(); ++chIter) {
      const TrigConf::HLTChain * ch = em->getChainConfigurationDetails(*chIter);
      std::string name = *chIter;
      m_summary.push_back(name);
      t_pt_map[name] = ch->pass_through();
      t_ps_map[name] = ch->prescale();
      m_lower_chain_map[name] = ch->lower_chain_name();
      msg(MSG::DEBUG) << "Configured chain: " << name
               << "; prescale=" << ch->prescale()
               << "; passthrough=" << ch->pass_through()
               << "; lower chain=" << ch->lower_chain_name() << endreq;
    }
    // sort by chain names to group together L2 and EF chains: do this for
    // first event only and use ordering in m_summary for later processing
    std::sort(m_summary.begin(),m_summary.end());

    // store prescaled factor from *configuration* chain
    for (unsigned int k=0; k<m_summary.size(); ++k) {
      // prescales
      std::map<std::string,float>::iterator psIt=t_ps_map.find(m_summary[k]);
      float ps = -1;
      if (psIt!=t_ps_map.end()) ps = (*psIt).second;
      m_chain_prescales[k]=ps;
      // passthrough
      std::map<std::string,float>::iterator ptIt=t_pt_map.find(m_summary[k]);
      float pt = -1;
      if (ptIt!=t_pt_map.end()) pt = (*ptIt).second;
      m_chain_passthrough[k]=pt;
    }
  }

  // all events: loop over names of configured chains, find their status,
  //   increment counters
  for (unsigned int i=0; i<m_summary.size(); ++i){
    std::string name = m_summary[i];
    msg(MSG::DEBUG) << "Testing chain: " << name << endreq;

    const HLT::Chain* aChain = em->getChainDetails(name);
    if (! aChain) { // inactive chain
      continue;
    }

    // use TrigDecisionTool methods directly
    if ( m_trigDec->isPassed(name, TrigDefs::allowResurrectedDecision | TrigDefs::requireDecision) ) {
      msg(MSG::INFO) << "chain: " << name << " Passed RAW" << endreq;
      ++m_summary_passraw[i];
    }
    if ( m_trigDec->isPassed(name) ) {
      msg(MSG::INFO) << "chain: " << name << " Passed" << endreq;
      ++m_summary_pass[i];
    }
    if ( m_trigDec->isPassed(name) ) {
      msg(MSG::INFO) << "chain: " << name << " Passed PHYSICS" << endreq;
      ++m_summary_passphys[i];
    }


    // Reproduce the definitions of raw, with PS, with PT, as used in monitoring:
    // http://alxr.usatlas.bnl.gov/lxr/source/atlas/Trigger/TrigMonitoring/TrigSteerMonitor/src/TrigChainMoni.cxx#380
    // chainPassed == (chainPassedRaw() && !aChain->isPrescaled()) || isPassedThrough
    if (aChain->chainPassedRaw()){
      ++m_summary_chain_passraw[i];
    }
    if (aChain->chainPassedRaw() && !aChain->isPrescaled()){
      ++m_summary_chain_PS[i];
    }
    if (aChain->chainPassed()){
      ++m_summary_chain_PT[i];
    }
    if (!(aChain->isPrescaled())){ // get prescale fraction by counting events
      ++m_chain_prescales_calculated[i];
    }
    if (aChain->isPassedThrough()){ // get passthrough fraction by counting events
      ++m_chain_passthrough_calculated[i];
    }
    // events accepted by the lower chain
    std::map<std::string,std::string>::iterator lcIt=m_lower_chain_map.find(m_summary[i]);
    if (lcIt!=m_lower_chain_map.end()) {
      if ( !m_trigDec->getListOfTriggers((*lcIt).second).empty() &&m_trigDec->isPassed((*lcIt).second) ) {
        ++m_lower_chain_accept[i];
      }
    }
    // print info for each event
    if (m_event_decision_printout){
      msg(MSG::INFO) << "chain " << name << " = "
	       << m_summary_chain_passraw[i] << endreq;
    }
  }

  // TrigDecisionTool tests on a few specific sigs:
  for (unsigned int i=0; i < m_count_sig_names.size(); ++i) {
    msg(MSG::DEBUG) << "Monitoring " << m_count_sig_names[i] << endreq;
    if ( !m_trigDec->getListOfTriggers(m_count_sig_names[i]).empty() ) {
      if ( m_trigDec->isPassed(m_count_sig_names[i]) )
	m_run_count_sigs[i] = m_run_count_sigs[i]+1;
    } else {
      msg(MSG::WARNING) << m_count_sig_names[i] << " not configured!" << endreq;
    }
  }

  if (m_event_number%m_monitoring_block_size == 0) {
    m_count_event.push_back(m_event_number);

    for (unsigned int i=0; i < m_count_sig_names.size(); ++i) {
      (m_count_sigs[i])->push_back(m_run_count_sigs[i]);
      m_run_count_sigs[i]=0;
    }
  }

 // reset first event flag
  if (m_first_event) m_first_event = false;

  return StatusCode::SUCCESS;
}

StatusCode TrigDecisionChecker::checkBjetEDM(std::string trigItem){

  msg(MSG::INFO) << "REGTEST ==========START of muon EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed(trigItem));

  Trig::FeatureContainer fc = m_trigDec->features(trigItem);

  // bjets
  const std::vector< Trig::Feature<TrigEFBjetContainer> > vec_bjetcont = fc.get<TrigEFBjetContainer>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<TrigEFBjetContainer> > = " << vec_bjetcont.size());
  for( auto bjetcont : vec_bjetcont ) {
    ATH_MSG_INFO("REGTEST Got bjet container, size = " << bjetcont.cptr()->size());
    for(unsigned int i=0; i<bjetcont.cptr()->size(); i++) {
      ATH_MSG_INFO("REGTEST Got bjet with IP3D/SV/COMB weights = " << (*(bjetcont.cptr()))[i]->xIP3D() << "/" << (*(bjetcont.cptr()))[i]->xSV() << "/" << (*(bjetcont.cptr()))[i]->xComb());
    }
  }// loop over bjet features

  // tracks
  const std::vector< Trig::Feature<Rec::TrackParticleContainer> > vec_trkcont = fc.get<Rec::TrackParticleContainer>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<Rec::TrackParticleContainer> > = " << vec_trkcont.size());
  for( auto trkcont : vec_trkcont ) {
    ATH_MSG_INFO("REGTEST Got track container, size = " << trkcont.cptr()->size());
  }// loop over track features
  
  // primary vertex
  const std::vector< Trig::Feature<TrigVertexCollection> > vec_pvcont = fc.get<TrigVertexCollection>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<TrigVertexCollection> > = " << vec_pvcont.size());
  for( auto pvcont : vec_pvcont ) {
    if(pvcont.cptr()->size())
      ATH_MSG_INFO("REGTEST Got primary vertex container, size/algoid/z = " << pvcont.cptr()->size() << "/" << pvcont.cptr()->front()->algorithmId() << "/" << pvcont.cptr()->front()->z());
    else
      ATH_MSG_INFO("REGTEST Got primary vertex container, size = " << pvcont.cptr()->size());
  }// loop over primary vertex features
  
  // secondary vertex
  const std::vector< Trig::Feature<Trk::VxSecVertexInfoContainer> > vec_svcont = fc.get<Trk::VxSecVertexInfoContainer>();
  ATH_MSG_INFO("Size of vector< Trk::VxSecVertexInfoContainer> > = " << vec_svcont.size());
  for( auto svcont : vec_svcont ) {
    if(svcont.cptr()->size())
      if(svcont.cptr()->front()->vertices().size())
	ATH_MSG_INFO("REGTEST Got secondary vertex container, size/size/type/x/y/z = " << svcont.cptr()->size() << "/" << svcont.cptr()->front()->vertices().size() << "/" 
		     << svcont.cptr()->front()->vertices().front()->vertexType() << "/"
		     << svcont.cptr()->front()->vertices().front()->position().x() << "/"
		     << svcont.cptr()->front()->vertices().front()->position().y() << "/"
		     << svcont.cptr()->front()->vertices().front()->position().z());
      else
	ATH_MSG_INFO("REGTEST Got secondary vertex container, size/size = " << svcont.cptr()->size() << "/" << svcont.cptr()->front()->vertices().size()); 
    else
      ATH_MSG_INFO("REGTEST Got secondary vertex container, size = " << svcont.cptr()->size());
  }// loop over secondary vertex features
  
  msg(MSG::INFO) << "REGTEST ==========END of bjet EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  return StatusCode::SUCCESS;
}//checkBjetEDM

StatusCode TrigDecisionChecker::checkMuonEDM(std::string trigItem){

  msg(MSG::INFO) << "REGTEST ==========START of muon EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed(trigItem));

  Trig::FeatureContainer fc = m_trigDec->features(trigItem);

  const std::vector< Trig::Feature<xAOD::MuonContainer> > vec_muons = fc.get<xAOD::MuonContainer>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::MuonContainer> > = " << vec_muons.size());

  for( auto mufeat : vec_muons ) {
    ATH_MSG_INFO("REGTEST Got muon container, size = " << mufeat.cptr()->size());
    std::string output = m_muonPrinter->print( *(mufeat.cptr()) );
    ATH_MSG_INFO(output);
  }// loop over muon features

  const std::vector< Trig::Feature<CombinedMuonFeature> > vec_cbmufeats = fc.get<CombinedMuonFeature>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<CombinedMuonFeature> > = " << vec_cbmufeats.size());

  for( auto cbmufeat : vec_cbmufeats) {
    ATH_MSG_INFO("REGTEST CombinedMuonFeature with pt, eta, phi = " << cbmufeat.cptr()->pt() << ", " << cbmufeat.cptr()->eta() << ", " << cbmufeat.cptr()->phi());
  }

  msg(MSG::INFO) << "REGTEST ==========END of muon EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  return StatusCode::SUCCESS;
}//checkMuonEDM

StatusCode TrigDecisionChecker::checkTauEDM(std::string trigItem){
  msg(MSG::INFO)<< "REGTEST ==========START of tau EDM/Navigation check for chain " << trigItem<< "===========" << endreq;
  Trig::FeatureContainer fc = m_trigDec->features(trigItem);
  const std::vector< Trig::Feature<xAOD::TauJetContainer> > vec_tauHLTClust = fc.get<xAOD::TauJetContainer>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::TauJetContainer> > = " << vec_tauHLTClust.size());
  for(auto cont_tau : vec_tauHLTClust) {
      ATH_MSG_INFO("REGTEST Got Tau container, size = " << cont_tau.cptr()->size());
    
      for(auto tauItr : *(cont_tau.cptr())) {
      
	msg(MSG::INFO) << "REGTEST "<<" HLT tau number of tracks: "<<tauItr->nTracks()<<endreq;
 	msg(MSG::INFO)  << "REGTEST "<<" HLT tau pt : "<<tauItr->pt()<<endreq;
        msg(MSG::INFO)  << "REGTEST "<<" HLT tau phi : "<<tauItr->phi()<<endreq;
        msg(MSG::INFO)  << "REGTEST "<<" HLT tau eta : "<<tauItr->eta()<<endreq;
        if( !tauItr->jetLink().isValid() ) {
          ATH_MSG_WARNING("tau does not have jet seed");
          return StatusCode::SUCCESS;
       }
        
        const xAOD::Jet* pJetSeed = *(tauItr->jetLink());   
        xAOD::JetConstituentVector::const_iterator clusItr  = pJetSeed->getConstituents().begin();
        xAOD::JetConstituentVector::const_iterator clusItrE = pJetSeed->getConstituents().end();   
        for (int clusCount = 0; clusItr != clusItrE; ++clusItr, ++clusCount) {     
        	ATH_MSG_INFO( "REGTEST Tau Cluster " << clusCount << " pt = " << (*clusItr)->pt()<< " eta = " << (*clusItr)->eta()<< " phi = " << (*clusItr)->phi() );     
        }  
        for (unsigned int trackNum = 0;  trackNum < tauItr->nTracks(); ++trackNum) {
        
	  const xAOD::TrackParticle *linkTrack = tauItr->track(trackNum);
	  if (!linkTrack) {
	  ATH_MSG_WARNING("can't get tau linked track");
	  return StatusCode::SUCCESS;
	   } else {
	  ATH_MSG_DEBUG("Got the tau linked track");
	  }     
	  ATH_MSG_INFO( "REGTEST Tau linked track " << trackNum << " pt = " << linkTrack->pt()<< " eta = " << linkTrack->eta() << " phi = " << linkTrack->phi() );
        }
      
      }
    }
    
  msg(MSG::INFO) << "REGTEST ==========END of Tau EDM/Navigation check ===========" << endreq;
  return StatusCode::SUCCESS;        
}



StatusCode TrigDecisionChecker::checkBphysEDM(std::string trigItem){
    
    msg(MSG::INFO) << "REGTEST ==========START of Bphysics EDM/Navigation check for chain " << trigItem << " ===========" << endreq;
    
    ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed(trigItem));
    
    Trig::FeatureContainer fc = m_trigDec->features(trigItem);
    
    const std::vector< Trig::Feature<xAOD::TrigBphysContainer> > fc_bphys = fc.get<xAOD::TrigBphysContainer>();
    ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::TrigBphysContainer> > = " << fc_bphys.size());
    
    for( auto cont_bphys : fc_bphys ) {
        ATH_MSG_INFO("REGTEST Got Bphysics container, size = " << cont_bphys.cptr()->size());
        for ( auto bphys:  *(cont_bphys.cptr()) )  {
            ATH_MSG_INFO("REGTEST  Bphysics Item mass, fitmass, secVx, nTP: "
                         << bphys->mass()*0.001 << " , " << bphys->fitmass() * 0.001 << " , "
                         << bphys->secondaryDecay() << " , " << bphys->nTrackParticles()
                         );
        } // for
        
    }// loop over bphys features
    
    //    const std::vector< Trig::Feature<CombinedMuonFeature> > vec_cbmufeats = fc.get<CombinedMuonFeature>();
    //    ATH_MSG_INFO("Size of vector< Trig::Feature<CombinedMuonFeature> > = " << vec_cbmufeats.size());
    //
    //    for( auto cbmufeat : vec_cbmufeats) {
    //        ATH_MSG_INFO("REGTEST CombinedMuonFeature with pt, eta, phi = " << cbmufeat.cptr()->pt() << ", " << cbmufeat.cptr()->eta() << ", " << cbmufeat.cptr()->phi());
    //    }
    //    
    msg(MSG::INFO) << "REGTEST ==========END of Bphysics EDM/Navigation check for chain " << trigItem << " ===========" << endreq;
    
    return StatusCode::SUCCESS;
}//checkBphysEDM

StatusCode TrigDecisionChecker::checkElectronEDM(std::string trigItem){
  msg(MSG::INFO) << "REGTEST ==========START of Electron EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed("HLT_"+trigItem));

  Trig::FeatureContainer fc = m_trigDec->features("HLT_"+trigItem);
  const std::vector< Trig::Feature<xAOD::ElectronContainer> > vec_el = fc.get<xAOD::ElectronContainer>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::ElectronContainer> > = " << vec_el.size());
  float val_float=-99.;
  for(auto elfeat : vec_el){
      ATH_MSG_INFO("REGTEST: Got electron container, size = " << elfeat.cptr()->size());
      const xAOD::ElectronContainer *elCont = elfeat.cptr();

      for(const auto& eg : *elCont){
          if (eg) {
              ATH_MSG_INFO(" REGTEST: egamma energy: " << eg->e() );
              ATH_MSG_INFO(" REGTEST: egamma eta: " << eg->eta() );
              ATH_MSG_INFO(" REGTEST: egamma phi: " << eg->phi() );
          } else{
              ATH_MSG_INFO(" REGTEST: problems with egamma pointer" );
              return StatusCode::SUCCESS;
          }
          ATH_MSG_INFO(" REGTEST: caloCluster variables ");
          if (eg->caloCluster()) {
              ATH_MSG_INFO(" REGTEST: egamma cluster transverse energy: " << eg->caloCluster()->et() );
              ATH_MSG_INFO(" REGTEST: egamma cluster eta: " << eg->caloCluster()->eta() );
              ATH_MSG_INFO(" REGTEST: egamma cluster phi: " << eg->caloCluster()->phi() );
              ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. etaCalo = " << eg->caloCluster()->auxdata<float>("etaCalo")); 
              ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. etaCalo = " << eg->caloCluster()->auxdata<float>("phiCalo")); 
          } else{
              ATH_MSG_INFO(" REGTEST: problems with egamma cluster pointer" );
          }
          ATH_MSG_INFO(" REGTEST: trackmatch variables ");
          if(eg->trackParticle()){
              ATH_MSG_INFO(" REGTEST: pt=  " << eg->trackParticle()->pt());
              ATH_MSG_INFO(" REGTEST: charge=  " << eg->trackParticle()->charge());
              ATH_MSG_INFO(" REGTEST: E/p=  " << eg->caloCluster()->et() / eg->trackParticle()->pt() );
              eg->trackCaloMatchValue(val_float,xAOD::EgammaParameters::deltaEta1);
              ATH_MSG_INFO(" REGTEST: Delta eta 1st sampling=  " << val_float);
              eg->trackCaloMatchValue(val_float,xAOD::EgammaParameters::deltaPhi2);
              ATH_MSG_INFO(" REGTEST: Delta phi 2nd sampling=  " << val_float);
          } else{
              ATH_MSG_INFO(" REGTEST: no electron eg->trackParticle() pointer");
          }
      }
  }

  msg(MSG::INFO) << "REGTEST ==========END of Electron EDM/Navigation check ===========" << endreq;
  return StatusCode::SUCCESS;
}

StatusCode TrigDecisionChecker::checkPhotonEDM(std::string trigItem){
  msg(MSG::INFO) << "REGTEST ==========START of Photon EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed("HLT_"+trigItem));

  Trig::FeatureContainer fc = m_trigDec->features("HLT_"+trigItem);
  const std::vector< Trig::Feature<xAOD::PhotonContainer> > vec_ph = fc.get<xAOD::PhotonContainer>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::PhotonContainer> > = " << vec_ph.size());
  //float val_float=-99.;
  for(auto phfeat : vec_ph){
      ATH_MSG_INFO("REGTEST: Got photon container, size = " << phfeat.cptr()->size());
      const xAOD::PhotonContainer *phCont = phfeat.cptr();

      for(const auto& eg : *phCont){
          if (eg) {
              ATH_MSG_INFO(" REGTEST: egamma energy: " << eg->e() );
              ATH_MSG_INFO(" REGTEST: egamma eta: " << eg->eta() );
              ATH_MSG_INFO(" REGTEST: egamma phi: " << eg->phi() );
          } else{
              ATH_MSG_INFO(" REGTEST: problems with egamma pointer" );
              return StatusCode::SUCCESS;
          }
          ATH_MSG_INFO(" REGTEST: caloCluster variables ");
          if (eg->caloCluster()) {
              ATH_MSG_INFO(" REGTEST: egamma cluster transverse energy: " << eg->caloCluster()->et() );
              ATH_MSG_INFO(" REGTEST: egamma cluster eta: " << eg->caloCluster()->eta() );
              ATH_MSG_INFO(" REGTEST: egamma cluster phi: " << eg->caloCluster()->phi() );
              ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. etaCalo = " << eg->caloCluster()->auxdata<float>("etaCalo")); 
              ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. etaCalo = " << eg->caloCluster()->auxdata<float>("phiCalo")); 
          } else{
              ATH_MSG_INFO(" REGTEST: problems with egamma cluster pointer" );
          }
      }
  }

  msg(MSG::INFO) << "REGTEST ==========END of Photon EDM/Navigation check ===========" << endreq;
  return StatusCode::SUCCESS;
}

StatusCode TrigDecisionChecker::checkMinBiasEDM(std::string trigItem){
  msg(MSG::INFO) << "REGTEST ==========START of MinBias EDM/Navigation check for chain " << trigItem << " ===========" << endreq;

  ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed(trigItem));

  Trig::FeatureContainer fc = m_trigDec->features(trigItem);

  checkTrigSpacePointCounts(fc);
	checkTrigT2MBTSBits(fc);
	checkTrigVertexCounts(fc);	
	checkTrigTrackCounts(fc);

  msg(MSG::INFO) << "REGTEST ==========END of MinBias EDM/Navigation check ===========" << endreq;
  return StatusCode::SUCCESS;
}

void TrigDecisionChecker::checkTrigSpacePointCounts(const Trig::FeatureContainer& fc){
	
	const std::vector< Trig::Feature<xAOD::TrigSpacePointCounts> > vec_sp = fc.get<xAOD::TrigSpacePointCounts>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::TrigSpacePointCounts> > = " << vec_sp.size());
	
	float sum;
  auto fun = [&sum](const std::vector<float>& vec){sum = 0.; for (const auto &i: vec) sum += i; return sum;};

  for(const auto& spfeat : vec_sp){
		const xAOD::TrigSpacePointCounts *spCounts = spfeat.cptr();
		if (spCounts){
			ATH_MSG_INFO(" REGTEST: SUM of contentsPixelClusEndcapC: " << fun(spCounts->contentsPixelClusEndcapC()) );
			ATH_MSG_INFO(" REGTEST: SUM of contentsPixelClusBarrel: " << fun(spCounts->contentsPixelClusBarrel()) );
			ATH_MSG_INFO(" REGTEST: SUM of contentsPixelClusEndcapA: " << fun(spCounts->contentsPixelClusEndcapA()) );

			ATH_MSG_INFO(" REGTEST: pixelClusTotBins: " << spCounts->pixelClusTotBins() );
			ATH_MSG_INFO(" REGTEST: pixelClusTotMin: " << spCounts->pixelClusTotMin() );
			ATH_MSG_INFO(" REGTEST: pixelClusTotMax: " << spCounts->pixelClusTotMax() );
			ATH_MSG_INFO(" REGTEST: pixelClusSizeBins: " << spCounts->pixelClusSizeBins() );
			ATH_MSG_INFO(" REGTEST: pixelClusSizeMin: " << spCounts->pixelClusSizeMin() );
			ATH_MSG_INFO(" REGTEST: pixelClusSizeMax: " << spCounts->pixelClusSizeMax() );
			ATH_MSG_INFO(" REGTEST: sctSpEndcapC: " << spCounts->sctSpEndcapC() );
			ATH_MSG_INFO(" REGTEST: sctSpBarrel: " << spCounts->sctSpBarrel() );	  
			ATH_MSG_INFO(" REGTEST: sctSpEndcapA: " << spCounts->sctSpEndcapA() );
		} else{
			ATH_MSG_INFO(" REGTEST: problems with TrigSpacePointCounts pointer" );
			return;
		}
	}
}

void TrigDecisionChecker::checkTrigT2MBTSBits(const Trig::FeatureContainer& fc){
	
	const std::vector< Trig::Feature<xAOD::TrigT2MbtsBits> > vec_mbts = fc.get<xAOD::TrigT2MbtsBits>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::TrigT2MbtsBits> > = " << vec_mbts.size());

	float sum;
  auto fun = [&sum](const std::vector<float>& vec){sum = 0.; for (const auto &i: vec) sum += i; return sum;};

	for(const auto& mbtsfeat : vec_mbts){
		const xAOD::TrigT2MbtsBits *mbtsBits = mbtsfeat.cptr();
		if (mbtsBits){
			ATH_MSG_INFO(" REGTEST: SUM of triggerEnergies: " << fun(mbtsBits->triggerEnergies()) );
			ATH_MSG_INFO(" REGTEST: SUM of triggerTimes: " << fun(mbtsBits->triggerTimes()) );
		} else{
			ATH_MSG_INFO(" REGTEST: problems with TrigT2MBTSBits pointer" );
			return;
		}
	}
}

void TrigDecisionChecker::checkTrigVertexCounts(const Trig::FeatureContainer& fc){
	
	const std::vector< Trig::Feature<xAOD::TrigVertexCounts> > vec_v = fc.get<xAOD::TrigVertexCounts>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::TrigVertexCounts> > = " << vec_v.size());

	auto funu = [](const std::vector<uint>& vec){uint sum = 0; for (const auto &i: vec) sum += i; return sum;};
	auto funf = [](const std::vector<float>& vec){float sum = 0.; for (const auto &i: vec) sum += i; return sum;};

	for(const auto& vfeat : vec_v){
		const xAOD::TrigVertexCounts *vCounts = vfeat.cptr();
		if (vCounts){
			ATH_MSG_INFO(" REGTEST: SUM of vtxNtrks: " << funu(vCounts->vtxNtrks()) );
			ATH_MSG_INFO(" REGTEST: SUM of vtxTrkPtSqSum: " << funf(vCounts->vtxTrkPtSqSum()) );
		} else{
			ATH_MSG_INFO(" REGTEST: problems with TrigVertexCounts pointer" );
			return;
		}
	}
}

void TrigDecisionChecker::checkTrigTrackCounts(const Trig::FeatureContainer& fc){

	const std::vector< Trig::Feature<xAOD::TrigTrackCounts> > vec_t = fc.get<xAOD::TrigTrackCounts>();
  ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::TrigTrackCounts> > = " << vec_t.size());

	float sum;
  auto fun = [&sum](const std::vector<float>& vec){sum = 0.; for (const auto &i: vec) sum += i; return sum;};

	for(const auto& tfeat : vec_t){
		const xAOD::TrigTrackCounts *tCounts = tfeat.cptr();
		if (tCounts){
			ATH_MSG_INFO(" REGTEST: SUM of z0_pt: " << fun(tCounts->z0_pt()) );
			ATH_MSG_INFO(" REGTEST: SUM of eta_phi: " << fun(tCounts->eta_phi()) );

			ATH_MSG_INFO(" REGTEST: z0Bins: " << tCounts->z0Bins() );
			ATH_MSG_INFO(" REGTEST: z0Min: " << tCounts->z0Min() );
			ATH_MSG_INFO(" REGTEST: z0Max: " << tCounts->z0Max() );
			ATH_MSG_INFO(" REGTEST: ptBins: " << tCounts->ptBins() );
			ATH_MSG_INFO(" REGTEST: ptMin: " << tCounts->ptMin() );
			ATH_MSG_INFO(" REGTEST: ptMax: " << tCounts->ptMax() );
			ATH_MSG_INFO(" REGTEST: etaBins: " << tCounts->etaBins() );
			ATH_MSG_INFO(" REGTEST: etaMin: " << tCounts->etaMin() );
			ATH_MSG_INFO(" REGTEST: etaMax: " << tCounts->etaMax() );
		} else{
			ATH_MSG_INFO(" REGTEST: problems with TrigTrackCounts pointer" );
			return;
		}
	}
}

StatusCode TrigDecisionChecker::checkJetEDM(std::string trigItem){
    ATH_MSG_DEBUG("in checkJetEDM()");
    
    ATH_MSG_INFO("REGTEST =====For chain " << trigItem << "=====");
    
    ATH_MSG_INFO("Chain passed = " << m_trigDec->isPassed(trigItem));
    
    Trig::FeatureContainer fc = m_trigDec->features(trigItem);
    const std::vector< Trig::Feature<xAOD::JetContainer> > vec_jet = fc.get<xAOD::JetContainer>();
    ATH_MSG_INFO("Size of vector< Trig::Feature<xAOD::JetContainer> > = " << vec_jet.size());
    for(auto jetfeat : vec_jet){
        const xAOD::JetContainer * jetCont = jetfeat.cptr();
        
        int jetContsize = jetCont->size();
        ATH_MSG_INFO("REGTEST Got jet container, size: " << jetContsize);
        
        int i = 0;
        for(const auto & thisjet : *jetCont){
            ++i;
            ATH_MSG_INFO("REGTEST Looking at jet " << i);
            if (thisjet) {
                //checks jet variables
                ATH_MSG_DEBUG("REGTEST    Checking jet variables");
                ATH_MSG_INFO("REGTEST    pt: " << thisjet->pt() );
                ATH_MSG_INFO("REGTEST    eta: " << thisjet->eta() );
                ATH_MSG_INFO("REGTEST    phi: " << thisjet->phi() );
                ATH_MSG_INFO("REGTEST    m: " << thisjet->m() );
                ATH_MSG_INFO("REGTEST    e: " << thisjet->e() );
                ATH_MSG_INFO("REGTEST    px: " << thisjet->px() );
                ATH_MSG_INFO("REGTEST    py: " << thisjet->py() );
                ATH_MSG_INFO("REGTEST    pz: " << thisjet->pz() );
                ATH_MSG_INFO("REGTEST    type: " << thisjet->type() );
                ATH_MSG_INFO("REGTEST    algorithm (kt: 0, cam: 1, antikt: 2, ...): " << thisjet->getAlgorithmType() );
                ATH_MSG_INFO("REGTEST    size parameter: " << thisjet->getSizeParameter() );
                ATH_MSG_INFO("REGTEST    input (LCTopo: 0, EMTopo: 1, ...): " << thisjet->getInputType() );
                ATH_MSG_INFO("REGTEST    constituents signal state (uncalibrated: 0, calibrated: 1): " << thisjet->getConstituentsSignalState() );
                ATH_MSG_INFO("REGTEST    number of constituents: " << thisjet->numConstituents() );
            }
            else{
                ATH_MSG_ERROR("REGTEST Problem with jet pointer" );
                return StatusCode::FAILURE;
            }
        }
        if (jetContsize == i) ATH_MSG_INFO("REGTEST size of jet container == number of displayed jets: " << (jetContsize == i) );
        else ATH_MSG_WARNING("REGTEST Problem with displaying jets");
    }
    
    ATH_MSG_DEBUG("leaving checkJetEDM()");
    
    return StatusCode::SUCCESS;
}


