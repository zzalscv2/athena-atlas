/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**************************************************************************
 **
 **   File: Trigger/TrigMonitoring/TrigIDTPMonitor/TrigIDTPMonitor.cxx
 **
 **   Description: Monitoring algorithm for ID T&P
 **                
 **
 **   Author: Sebastian Sanchez Herrera
 **           Johnny Raine <johnny.raine@cern.ch
 **
 **   Created:   19.01.2015
 **   Modified:  04.05.2015
 **
 **
 ***************************************************************************/

#include "TrigIDTPMonitor.h"

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "TrigConfHLTData/HLTTriggerElement.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTrigBphys/TrigBphys.h"
#include "xAODTrigBphys/TrigBphysContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

//#include "AthenaKernel/errorcheck.h"


TrigIDTPMonitor::TrigIDTPMonitor(const std::string& name, ISvcLocator* pSvcLocator) :
  HLT::FexAlgo(name, pSvcLocator) {

  //2D histogram containers
  declareMonitoredStdContainer("InvMass",  m_inv,      AutoClear);
  declareMonitoredStdContainer("LinesIM",  m_lines,    AutoClear);
  declareMonitoredStdContainer("InvMassTP",m_invTP,    AutoClear);
  declareMonitoredStdContainer("LinesTP",  m_linesTP,  AutoClear);
  declareMonitoredStdContainer("ProbePt",  m_pt,       AutoClear);
  declareMonitoredStdContainer("LinesPt",  m_linespt,  AutoClear);
  declareMonitoredStdContainer("ProbeEta", m_eta,      AutoClear);
  declareMonitoredStdContainer("LinesEta", m_lineseta, AutoClear);
  declareMonitoredStdContainer("ProbeD0",  m_d0,       AutoClear);
  declareMonitoredStdContainer("LinesD0",  m_linesd0,  AutoClear);
  declareMonitoredStdContainer("ProbePhi", m_phi,      AutoClear);
  declareMonitoredStdContainer("LinesPhi", m_linesphi, AutoClear);

  //verification of 2D histograms
  declareMonitoredVariable("InvMassD",   m_invD,        -1);
  declareMonitoredVariable("InvMassN",   m_invN,        -1);
  declareMonitoredVariable("InvMassTPD", m_invTPD,      -1);
  declareMonitoredVariable("InvMassTPN", m_invTPN,      -1);
  declareMonitoredVariable("ProbePTD",   m_ptD,         -1);
  declareMonitoredVariable("ProbePTN",   m_ptN,         -1);
  declareMonitoredVariable("ProbeEtaD",  m_etaD,      -999);
  declareMonitoredVariable("ProbeEtaN",  m_etaN,      -999);
  declareMonitoredVariable("ProbeD0D",   m_d0D,        -99);
  declareMonitoredVariable("ProbeD0N",   m_d0N,        -99);
  declareMonitoredVariable("ProbePhiD",  m_phiD,       -10);
  declareMonitoredVariable("ProbePhiN",  m_phiN,       -10);
  declareMonitoredVariable("deltaMass",  m_massDiff,-99999);
  declareMonitoredVariable("ID-TagMass", m_IDTmass,     -1);


  declareMonitoredStdContainer("Efficiencies", m_eff,  AutoClear);
  
}

TrigIDTPMonitor::~TrigIDTPMonitor(){ }

HLT::ErrorCode TrigIDTPMonitor::hltInitialize(){
  ATH_MSG_DEBUG( "Running TrigIDTPMonitor::hltInitialize" );
  return HLT::OK;
}

HLT::ErrorCode TrigIDTPMonitor::hltFinalize(){
  ATH_MSG_DEBUG( "Running TrigIDTPMonitor::hltFinalize" );
  return HLT::OK;
}

HLT::ErrorCode TrigIDTPMonitor::hltBeginRun() {
  ATH_MSG_INFO( "beginning run in this " << name() );
  return HLT::OK;
}

HLT::ErrorCode TrigIDTPMonitor::acceptInput(const HLT::TriggerElement* inputTE, bool& pass){
  ATH_MSG_DEBUG( "Running TrigIDTPMonitor::acceptInputs" );
  pass = true;
  return HLT::OK;
}

HLT::ErrorCode TrigIDTPMonitor::hltExecute(const HLT::TriggerElement* inputTE, HLT::TriggerElement* outputTE){
  
  ATH_MSG_DEBUG( "Running TrigIDTP::hltExecute" );

  ATH_MSG_DEBUG( "Input TE: " << *inputTE );
  ATH_MSG_DEBUG( "Output TE: " << *outputTE );

  //defined values
  m_mumuMass=91187.6;
  m_massAcceptanceTP=20000.0;
  m_massAcceptanceTID=20000.0;
  m_lowerPtCutP = 10000.0;
  m_lowerPtCutT = 15000.0;
  m_lowerPtCutID = 0.0;
  m_etaUpperCut = 2.5;

  
  //retrieve event info
  const EventInfo* pEventInfo(0);
  const xAOD::EventInfo* evtInfo(0);
  int IdRun=0;
  int IdEvent=0;
  if( store()->retrieve(evtInfo).isFailure() ) {
    ATH_MSG_DEBUG( "Failed to get xAOD::EventInfo " );
    if ( store()->retrieve(pEventInfo).isFailure() ) {//try old event format
      ATH_MSG_DEBUG( "Failed to get EventInfo " );
    }
    else {
      IdRun   = pEventInfo->event_ID()->run_number();
      IdEvent = pEventInfo->event_ID()->event_number();
      ATH_MSG_DEBUG( " Run " << IdRun << " Event " << IdEvent );
    }//found using old event info
  }
  else{//found xAOD info
    ATH_MSG_DEBUG( " Run " << evtInfo->runNumber() << " Event " << evtInfo->eventNumber() );
    IdRun = evtInfo->runNumber();
    IdEvent = evtInfo->eventNumber();
  }


  //retrieve TrigBphys object
  const xAOD::TrigBphysContainer * trigBphysColl(NULL);

  if(getFeature(outputTE, trigBphysColl, "EFBMuMuFex") != HLT::OK){
    ATH_MSG_WARNING( "Failed to get TrigBphysics collection" );
    ATH_MSG_DEBUG( "End of hltExecute" );
  }
  
  ATH_MSG_DEBUG( "Got TrigBphysCollection = " << trigBphysColl );

  if(trigBphysColl == 0){
    ATH_MSG_DEBUG( "No Bphys particles to analyse" );
    return HLT::OK;
  }

  ATH_MSG_DEBUG( "Got TrigBphysCollection with " << trigBphysColl->size() << " TrigBphys particles" );
  
  if(trigBphysColl->size() == 0){
    ATH_MSG_DEBUG( "No Bphys particles to analyse" );
    return HLT::OK;
  }

  double invMass = 0;
  
  for(auto bphys : *trigBphysColl){

    //print information
    ATH_MSG_DEBUG( "Info TrigEFBPhys Object:" );
    ATH_MSG_DEBUG( "\tRoI_ID: "<< bphys->roiId() );
    ATH_MSG_DEBUG( "\tRoI_ID: "<< bphys->roiId() );
    ATH_MSG_DEBUG( "\tParticle type: "<< bphys->particleType() );
    ATH_MSG_DEBUG( "\teta: "<< bphys->eta() );
    ATH_MSG_DEBUG( "\tphi: "<< bphys->phi() );
    ATH_MSG_DEBUG( "\tmass: "<< bphys->mass() );
    ATH_MSG_DEBUG( "\tmass after fit: "<< bphys->fitmass() );
    ATH_MSG_DEBUG( "\tfit chi2: "<< bphys->fitchi2() );
    ATH_MSG_DEBUG( "\tfit ndof: "<< bphys->fitndof() );
    ATH_MSG_DEBUG( "\tx: "<< bphys->fitx() );
    ATH_MSG_DEBUG( "\ty: "<< bphys->fity() );
    ATH_MSG_DEBUG( "\tz: "<< bphys->fitz() );

    //find Bphys inv mass closest to Z mass for histogramming
    if(abs((bphys->mass())-m_mumuMass)<(abs(invMass-m_mumuMass)))
      {
        invMass=bphys->mass();
      }
  }

  if(abs(invMass-m_mumuMass)>m_massAcceptanceTP)
    {
      ATH_MSG_DEBUG( "Invalid Candidate from TrigEFBPhys mass" );
      return HLT::OK;
    }


  //access Trigger Elements
  std::vector<HLT::TriggerElement*> tes = config()->getNavigation()->getDirectPredecessors(inputTE);

  ATH_MSG_DEBUG( "Retrieved Trigger Elements = " << tes );

  if(tes.size() != 2){
    ATH_MSG_DEBUG( "There are " << tes.size() << " Trigger Elements" );
    ATH_MSG_INFO( "Exiting IDTPMonitor" );
    return HLT::OK;
  }


  //get Labels for T&P identification
  std::string teInLabel1;
  std::string teInLabel2;

  TrigConf::HLTTriggerElement::getLabel( tes.front()->getId(), teInLabel1 );
  ATH_MSG_DEBUG( "Name1: " << teInLabel1 );

  TrigConf::HLTTriggerElement::getLabel( tes.back()->getId(), teInLabel2 );
  ATH_MSG_DEBUG( "Name2: " << teInLabel2 );


  //Is this section necessary

  //Muon ID Tracks

  const xAOD::TrackParticleContainer *muon1TrkContainer(NULL);
  const xAOD::TrackParticleContainer *muon2TrkContainer(NULL);

  ATH_MSG_DEBUG( "FTF get" );

  if ( getFeature(tes.front(), muon1TrkContainer, "InDetTrigTrackingxAODCnv_Muon_FTF") != HLT::OK){//,"InDetTrigParticleCreation_Muon_EFID") != HLT::OK ) {
    ATH_MSG_WARNING( "Navigation error while getting TrackParticleContainer 1 - FTF" );
    return HLT::OK;
  }

  if (muon1TrkContainer == NULL ){
    ATH_MSG_DEBUG( "Navigation error whilst getting TrackParticleContainer 1. Received null pointer" );
    return HLT::OK;
  }

  ATH_MSG_DEBUG( "Got Particle collection 1 with " << muon1TrkContainer->size() << " particles" );

  if ( getFeature(tes.back(), muon2TrkContainer, "InDetTrigTrackingxAODCnv_Muon_FTF") != HLT::OK){//,"InDetTrigParticleCreation_Muon_EFID") != HLT::OK ) {
    ATH_MSG_WARNING( "Navigation error while getting TrackParticleContainer 2 - FTF" );
    return HLT::OK;
  }

  if (muon1TrkContainer == NULL ){
    ATH_MSG_DEBUG( "Navigation error whilst getting TrackParticleContainer 2. Received null pointer" );
    return HLT::OK;
  }

  ATH_MSG_DEBUG( "Got Particle collection 2 with " << muon2TrkContainer->size() << " particles" );
  
  for(auto p1 : *muon1TrkContainer){
    for(auto p2 : *muon2TrkContainer){
      double invm = TrackInvMass(p1,p2);
      ATH_MSG_DEBUG( "FTF Inv Mass = " << invm );
      if(abs(invm-m_mumuMass) > m_massAcceptanceTID)
	ATH_MSG_DEBUG( "Inefficient event from EF" );
    }
  }

  ATH_MSG_DEBUG( "PT get" );

  if ( getFeature(tes.front(), muon1TrkContainer, "InDetTrigTrackingxAODCnv_Muon_EFID") != HLT::OK){//,"InDetTrigParticleCreation_Muon_EFID") != HLT::OK ) {
    //if( getFeature(tes.front(), muon1TrkContainer, "InDetTrigTrackingxAODCnv_Bphysics_IDTrig") != HLT::OK){
    ATH_MSG_WARNING( "Navigation error while getting TrackParticleContainer 1 - PT" );
    return HLT::OK;
    // }
  }

  if (muon1TrkContainer == NULL ){
    ATH_MSG_DEBUG( "Navigation error whilst getting TrackParticleContainer 1. Received null pointer" );
    return HLT::OK;
  }

  ATH_MSG_DEBUG( "Got Particle collection 1 with " << muon1TrkContainer->size() << " particles" );

  if ( getFeature(tes.back(), muon2TrkContainer, "InDetTrigTrackingxAODCnv_Muon_EFID") != HLT::OK){//,"InDetTrigParticleCreation_Muon_EFID") != HLT::OK ) {
    //if( getFeature(tes.front(), muon1TrkContainer, "InDetTrigTrackingxAODCnv_Bphysics_IDTrig") != HLT::OK){
    ATH_MSG_WARNING( "Navigation error while getting TrackParticleContainer 2 - PT" );
    return HLT::OK;
    //}
  }

  if (muon1TrkContainer == NULL ){
    ATH_MSG_DEBUG( "Navigation error whilst getting TrackParticleContainer 2. Received null pointer" );
    return HLT::OK;
  }

  ATH_MSG_DEBUG( "Got Particle collection 2 with " << muon2TrkContainer->size() << " particles" );
  
  for(auto p1 : *muon1TrkContainer){
    for(auto p2 : *muon2TrkContainer){
      double invm = TrackInvMass(p1,p2);
      ATH_MSG_DEBUG( "PT Inv Mass = " << invm );
      if(abs(invm-m_mumuMass) > m_massAcceptanceTID)
	ATH_MSG_DEBUG( "Inefficient event from EF" );
    }
  }

  //endis

  const TrigRoiDescriptor *roi1(0);
  const TrigRoiDescriptor *roi2(0);

  if( getFeature(tes.front(), roi1, "forID") != HLT::OK){
    ATH_MSG_WARNING( "Navigation error whilst getting RoI descriptor 1" );
    return HLT::OK;
  }

  if( roi1==NULL ){
    ATH_MSG_DEBUG( "Navigation Error: RoI descriptor 1 retrieved null pointer" );
    return HLT::OK;
  }

  if( getFeature(tes.back(), roi2, "forID") != HLT::OK){
    ATH_MSG_WARNING( "Navigation error whilst getting RoI descriptor 2" );
    return HLT::OK;
  }

  if( roi2==NULL ){
    ATH_MSG_DEBUG( "Navigation Error: RoI descriptor 2 retrieved null pointer" );
    return HLT::OK;
  }

  //print roi descriptor info
  ATH_MSG_VERBOSE( "Muon1 RoI Descriptor info: ");
  ATH_MSG_VERBOSE( "\tz1: " << roi1->zed() << " +/- " << ((roi1->zedPlus()-roi1->zedMinus())/2.) );
  ATH_MSG_VERBOSE( "\teta1: " << roi1->eta() << " +/- " << ((roi1->etaPlus()-roi1->etaMinus())/2.) );
  ATH_MSG_VERBOSE( "\t[ eta1(z+) " << roi1->etaPlus() << " eta1(z-) " << roi1->etaMinus() << " ]" );
  ATH_MSG_VERBOSE( "\tphi1: " << roi1->phi() << " +/- " << ((roi1->phiPlus()-roi1->phiMinus())/2.) );
		
  
  ATH_MSG_VERBOSE( "Muon1 Ro2 Descriptor info: " );
  ATH_MSG_VERBOSE( "\tz2: " << roi2->zed() << " +/- " << ((roi2->zedPlus()-roi2->zedMinus())/2.) );
  ATH_MSG_VERBOSE( "\teta2: " << roi2->eta() << " +/- " << ((roi2->etaPlus()-roi2->etaMinus())/2.) );
  ATH_MSG_VERBOSE( "\t[ eta2(z+) " << roi2->etaPlus() << " eta2(z-) " << roi2->etaMinus() << " ]" );
  ATH_MSG_VERBOSE( "\tphi2: " << roi2->phi() << " +/- " << ((roi2->phiPlus()-roi2->phiMinus())/2.) );

  //retrieve Muon Containers

  const xAOD::MuonContainer *mc1;
  const xAOD::MuonContainer *mc2;

  if( getFeature(tes.front(), mc1) != HLT::OK){
    ATH_MSG_WARNING( "Navigation error whilst getting Muon Container 1" );
    return HLT::OK;
  }

  if( mc1 == NULL ){
    ATH_MSG_DEBUG( "Navigation error, Muon Container 1 retrieved null pointer" );
    return HLT::OK;
  }

  if( getFeature(tes.back(), mc2) != HLT::OK){
    ATH_MSG_WARNING( "Navigation error whilst getting Muon Container 2" );
    return HLT::OK;
  }

  if( mc2 == NULL ){
    ATH_MSG_DEBUG( "Navigation error, Muon Container 2 retrieved null pointer" );
    return HLT::OK;
  }

  //check one muon per roi

  if( mc1->size() != 1 ){
    if( mc1->size() == 0 )
      ATH_MSG_DEBUG( "No Muon in first RoI" );
    else
      ATH_MSG_DEBUG( "More than one Muon in first RoI" );
    return HLT::OK;
  }
  ATH_MSG_DEBUG( "Retrieved muon container 1: " << mc1 );

  if( mc2->size() != 1 ){
    if( mc2->size() == 0 )
      ATH_MSG_DEBUG( "No Muon in first RoI" );
    else
      ATH_MSG_DEBUG( "More than one Muon in first RoI" );
    return HLT::OK;
  }
  ATH_MSG_DEBUG( "Retrieved muon container 2: " << mc2 );


  bool tag1probe2=false;
  bool tag2probe1=false;

  std::vector<const xAOD::TrackParticle*> tagMuonTracks;
  std::vector<const xAOD::TrackParticle*> probeMuonTracks;

  double deltaEtaRoi1 = ((roi1->etaPlus()-roi1->etaMinus())/2.);
  double deltaEtaRoi2 = ((roi2->etaPlus()-roi2->etaMinus())/2.);
  double deltaPhiRoi1 = ((roi1->phiPlus()-roi1->phiMinus())/2.);
  double deltaPhiRoi2 = ((roi2->phiPlus()-roi2->phiMinus())/2.);
  double deltaEta;
  double deltaPhi;

  std::string tagname = "EF_Comb_mu13_idperf";
  std::string probename = "EF_SuperEF_mu13_MU10";

  //Check tag has combined tracks, add to vector of combined tracks
  //Check probe has extrapolated tracks, check they are in agreement with RoI, add to vector of tracks
  if(teInLabel1.compare(tagname) == 0 && teInLabel2.compare(probename) == 0){
    ATH_MSG_VERBOSE( "The first muon is the tag, the second muon is the probe" );
    tag1probe2 = true;
  }
  else if(teInLabel1.compare(probename) == 0 && teInLabel2.compare(tagname) == 0){
    ATH_MSG_VERBOSE( "The second muon is the tag, the first muon is the probe" );
    tag2probe1 = true;
  }

  if(tag1probe2){

    //tag muon
    for(auto muonInfo : *mc1){
      if(muonInfo == NULL){
	ATH_MSG_WARNING( "Non initialised muon in first roi, leaving!" );
	return HLT::OK;
      }

      if(muonInfo->primaryTrackParticle() != NULL){
	const xAOD::TrackParticle* muonTrack =0;
	
	muonTrack = muonInfo->primaryTrackParticle();
	//muonTrack = muonInfo->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle);
	if(muonTrack == NULL){
	  ATH_MSG_DEBUG( "Tag Muon returns null primaryTrackParticleLink" );
	  continue;
	}
	if(muonTrack != muonInfo->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle)){
	  ATH_MSG_DEBUG( "Tag not matched to Combined track" );
	  continue;
	}
	
	if(muonTrack->charge() > 0 || true){
	  tagMuonTracks.push_back(muonTrack);
	  ATH_MSG_DEBUG( "Track added to tag vector: " << muonInfo );
	}
      }
    }
    
    //probe muon
    for(auto muonInfo : *mc2){
      if(muonInfo == NULL){
	ATH_MSG_WARNING( "Non initialised muon in second roi, leaving!" );
	return HLT::OK;
      }

      if(muonInfo->primaryTrackParticle() != NULL){
      
	const xAOD::TrackParticle* muonTrack = 0;      
	if(muonInfo->muonSpectrometerTrackParticleLink().isValid())
	  muonTrack = (*muonInfo->muonSpectrometerTrackParticleLink());
	if(muonTrack == NULL){
	  ATH_MSG_DEBUG( "No muonSpectrometerTrackParticle for probe muon" );
	  continue;
	}
	
	/*for(auto muonTrack : muonInfo->muonSpectrometerTrackParticleLink()){
	  if(muonTrack == NULL){
	  ATH_MSG_ERROR( "Probe Muon returns null muonSpectrometerTrackParticleLink" );
	  continue;
	  }
	  if(muonTrack != muonInfo->trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle)){
	  ATH_MSG_ERROR( "Probe not matched to Combined track" );
	  continue;
	  }*/
	 
	deltaEta = fabs( muonTrack->eta() - roi2->eta() );
	deltaPhi = fabs( muonTrack->phi() - roi2->phi() );
	
	ATH_MSG_VERBOSE( "deltaEta = " << deltaEta );
	ATH_MSG_VERBOSE( "deltaPhi = " << deltaPhi );
	ATH_MSG_VERBOSE( "deltaEtaRoi2 = " << deltaEtaRoi2 );
	ATH_MSG_VERBOSE( "deltaPhiRoi2 = " << deltaPhiRoi2 );
	
	if( (deltaEta < deltaEtaRoi2) && (deltaPhi < deltaPhiRoi2) ){
	  probeMuonTracks.push_back(muonTrack);
	  ATH_MSG_DEBUG( "Probe track added to muon tracks vector: " << muonTrack);
	}
	else{
	  ATH_MSG_DEBUG( "Muon outside corresponding region" );
	}
      }
    }
  }
  else if(tag2probe1){

    //tag muon
    for(auto muonInfo : *mc2){
      if(muonInfo == NULL){
	ATH_MSG_WARNING( "Non initialised muon in first roi, leaving!" );
	return HLT::OK;
      }

      if(muonInfo->primaryTrackParticle() != NULL){
	const xAOD::TrackParticle* muonTrack = 0;
	muonTrack = muonInfo->primaryTrackParticle();
	//muonTrack = muonInfo->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle);
	if(muonTrack == NULL){
	  ATH_MSG_DEBUG( "Tag Muon returns null TrackParticle" );
	  continue;
	}
	if(muonTrack != muonInfo->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle)){
	  ATH_MSG_DEBUG( "Tag not matched to Combined track" );
	  continue;
	}
	
	if(muonTrack->charge() > 0 || true){
	  tagMuonTracks.push_back(muonTrack);
	  ATH_MSG_DEBUG( "Track added to tag vector: " << muonInfo );
	}
      }
    }
  

    //probe muon
    for(auto muonInfo : *mc1){
      if(muonInfo == NULL){
	ATH_MSG_WARNING( "Non initialised muon in second roi, leaving!" );
	return HLT::OK;
      }

      if(muonInfo->primaryTrackParticle() != NULL){
	const xAOD::TrackParticle* muonTrack = 0;      
	if(muonInfo->muonSpectrometerTrackParticleLink().isValid())
	  muonTrack = (*muonInfo->muonSpectrometerTrackParticleLink());
	if(muonTrack == NULL){
	  ATH_MSG_DEBUG( "No muonSpectrometerTrackParticle for probe muon" );
	  continue;
	}
	

	/*for(const auto muonTrack : muonInfo->muonSpectrometerTrackParticleLink()){
	  if(muonTrack == NULL){
	  ATH_MSG_ERROR( "Probe Muon returns null muonSpectrometerTrackParticleLink" );
	  continue;
	  }
	  if(muonTrack != muonInfo->trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle)){
	  ATH_MSG_ERROR( "Probe not matched to Combined track" );
	  continue;
	  }*/
	  
	deltaEta = fabs( muonTrack->eta() - roi1->eta() );
	deltaPhi = fabs( muonTrack->phi() - roi1->phi() );
	
	ATH_MSG_VERBOSE( "deltaEta = " << deltaEta );
	ATH_MSG_VERBOSE( "deltaPhi = " << deltaPhi );
	ATH_MSG_VERBOSE( "deltaEtaRoi1 = " << deltaEtaRoi1 );
	ATH_MSG_VERBOSE( "deltaPhiRoi1 = " << deltaPhiRoi1 );
	
	if( (deltaEta < deltaEtaRoi1) && (deltaPhi < deltaPhiRoi1) ){
	  probeMuonTracks.push_back(muonTrack);
	  ATH_MSG_DEBUG( "Probe track added to muon tracks vector: " << muonTrack);
	}
	else{
	  ATH_MSG_DEBUG( "Muon outside corresponding region" );
	}
      }
    }   
  }
  else{
    ATH_MSG_DEBUG( "No tag/probe pair. Leaving!" );
    return HLT::OK;
  }
  //

  //check vectors for number of tracks


  if(tagMuonTracks.size() == 0){
    ATH_MSG_DEBUG( "Empty Tag vector, nothing to analyse, leaving" );
    return HLT::OK;
  }

  if(probeMuonTracks.size() == 0){
    ATH_MSG_DEBUG( "Empty Probe vector, nothing to analyse, leaving" );
    return HLT::OK;
  }

  if(tagMuonTracks.size() > 1){
    ATH_MSG_DEBUG( "More than one tag tracks in vector" );
  }
  
  if(probeMuonTracks.size() > 1){
    ATH_MSG_DEBUG( "More than one probe tracks in vector" );
  }

  //check whether probe RoI corresponds to ID tracks
 
  const xAOD::TrackParticleContainer* PTTrkContainer(NULL);

  if(tag1probe2){
    if(getFeature(tes.back(), PTTrkContainer, "InDetTrigTrackingxAODCnv_Muon_EFID") != HLT::OK){
      if(getFeature(tes.back(), PTTrkContainer, "InDetTrigTrackingxAODCnv_Muon_IDTrig") != HLT::OK){
        if(getFeature(tes.back(), PTTrkContainer, "InDetTrigTrackingxAODCnv_Bphysics_IDTrig") != HLT::OK){
	  ATH_MSG_WARNING( "There was an error retrieving Probe's ID TrackParticleContainer" );
	  return HLT::OK;
	}
      }
    }
  }
  else if(tag2probe1){
    if(getFeature(tes.front(), PTTrkContainer, "InDetTrigTrackingxAODCnv_Muon_EFID") != HLT::OK){
      if(getFeature(tes.front(), PTTrkContainer, "InDetTrigTrackingxAODCnv_Muon_IDTrig") != HLT::OK){
        if(getFeature(tes.front(), PTTrkContainer, "InDetTrigTrackingxAODCnv_Bphysics_IDTrig") != HLT::OK){
	  ATH_MSG_WARNING( "There was an error retrieving Probe's ID TrackParticleContainer" );
	  return HLT::OK;
	}
      }
    } 
  }

  if( PTTrkContainer == NULL ){
    ATH_MSG_DEBUG( "Navigation error whilst getting Probe ID TrackParticleContainer (retrieved null pointer)" );
    return HLT::OK;
  }

  ATH_MSG_DEBUG( "Retrieved ID track collection with " << PTTrkContainer->size() << " particles" );

  //TrigInDetTrackCollection Strategy tests
  const xAOD::TrackParticleContainer *FTFTrkContainer(NULL);    
  bool gotFTF = false;
  
  if(tag2probe1){    
    if(getFeature(tes.front(), FTFTrkContainer, "InDetTrigTrackingxAODCnv_Muon_FTF") != HLT::OK){
      ATH_MSG_DEBUG( "No FTF Tracking particle container found" );
    }
  }
  else if(tag1probe2){
    if(getFeature(tes.back(), FTFTrkContainer, "InDetTrigTrackingxAODCnv_Muon_FTF") != HLT::OK){
      ATH_MSG_DEBUG( "No FTF Tracking particle container found" );
    }  
  }
  if(FTFTrkContainer->size() != 0){
    gotFTF = true;
    ATH_MSG_DEBUG( "Got FTF container" );
  }

  //iterate over tag/probe pairs and check for efficiency
  for(auto tagTrack : tagMuonTracks){
    for(auto probeTrack : probeMuonTracks){

      double m_trackInvMass = TrackInvMass(tagTrack, probeTrack);

      m_massDiff = m_mumuMass - m_trackInvMass;
      float invmassdiff = fabs(m_massDiff);

      ATH_MSG_VERBOSE( "tagTrack pT = " << tagTrack->pt() );
      ATH_MSG_VERBOSE( "probeTrack pT = " << probeTrack->pt() );
      ATH_MSG_VERBOSE( "TrackInvMass = " << m_trackInvMass );
      ATH_MSG_VERBOSE( "Mass difference of track inv mass and mumuMass = " << invmassdiff );
      ATH_MSG_VERBOSE( "Mass acceptance = " << m_massAcceptanceTP );
      ATH_MSG_VERBOSE( "probeTrack d0 = " << probeTrack->d0() );

      if( invmassdiff < m_massAcceptanceTP){


	//check tag and probe opposite charge?
	
	//Valid pair
	m_inv.push_back(invMass);
	m_lines.push_back(0.5);
	
	m_invTP.push_back(m_trackInvMass);
	m_linesTP.push_back(0.5);
	
	m_pt.push_back(probeTrack->pt());
	m_linespt.push_back(0.5);
	
	m_eta.push_back(probeTrack->eta());
	m_lineseta.push_back(0.5);
	
	m_d0.push_back(probeTrack->d0());
	m_linesd0.push_back(0.5);
	
	m_phi.push_back(probeTrack->phi());
	m_linesphi.push_back(0.5);
	
	m_eff.push_back(0.5);

	//verify histograms denominator
	m_invD=invMass;
	m_ptD=probeTrack->pt();
	m_etaD=probeTrack->eta();
	m_d0D=probeTrack->d0();
	m_invTPD=TrackInvMass(tagTrack,probeTrack);
	m_phiD=probeTrack->phi();

	//compatible ID track for probe?
	bool found = false;
	
	for(auto particle : *PTTrkContainer){

	  if(found) continue;

	  float IDmassdiff = fabs(m_mumuMass - compatibleIDTrack(tagTrack, probeTrack, particle));

	  ATH_MSG_VERBOSE( "Mass difference of IDtrack and mumuMass = " << IDmassdiff );
	  ATH_MSG_VERBOSE( "Mass acceptance = " << m_massAcceptanceTID );
	  
	  if(IDmassdiff < m_massAcceptanceTID){

	    m_inv.push_back(invMass);
	    m_lines.push_back(1.5);

	    m_invTP.push_back(TrackInvMass(tagTrack,probeTrack));
	    m_linesTP.push_back(1.5);

	    //m_pt.push_back(probeTrack->pt());
	    m_pt.push_back(particle->pt());
	    m_linespt.push_back(1.5);

	    //m_eta.push_back(probeTrack->eta());
	    m_eta.push_back(particle->eta());
	    m_lineseta.push_back(1.5);

	    //m_d0.push_back(probeTrack->d0());
	    m_d0.push_back(particle->d0());
	    m_linesd0.push_back(1.5);

	    //m_phi.push_back(probeTrack->phi());
	    m_phi.push_back(particle->phi());
	    m_linesphi.push_back(1.5);

	    m_eff.push_back(1.5);

	    //verify histograms numerator
	    m_invN=invMass;
	    m_ptN=probeTrack->pt();
	    m_etaN=probeTrack->eta();
	    m_d0N=probeTrack->d0();
	    m_phiN=probeTrack->phi();
	    m_invTPN=TrackInvMass(tagTrack,probeTrack);
	    m_IDTmass=compatibleIDTrack(tagTrack, probeTrack, particle);

	    ATH_MSG_INFO( "Found probe ID candidate from Precision Tracking" );
	    found = true;
	    
	  }
	}
	if(!found)
	  ATH_MSG_INFO( "Inefficient event, no Precision Tracking ID match found" );

	//}	
	//}

	//check for FTF
	if(gotFTF){
	  bool FTFfound = false;
	  for(auto FTFTrack : *FTFTrkContainer){
	    if(FTFfound) continue;
	
	    float IDmassdiff = fabs(m_mumuMass - compatibleIDTrack(tagTrack, probeTrack, FTFTrack));

	    //float massdiff = 300000;
	    //float FTFinvmass = 0;
	    /*if(FTFTrack->pt() > m_lowerPtCutID){
	      FTFinvmass = TrackInvMass(tagTrack, FTFTrack);
	      massdiff = fabs(m_mumuMass - FTFinvmass);
	      }*/
	
	    ATH_MSG_VERBOSE( "Mass difference of FTFtrack and mumuMass = " << IDmassdiff );
	    ATH_MSG_VERBOSE( "Mass acceptance = " << m_massAcceptanceTID );
	
	    if(IDmassdiff < m_massAcceptanceTID){
	  
	      m_inv.push_back(invMass);
	      m_lines.push_back(2.5);
	  
	      //m_invTP.push_back(FTFinvmass);
	      m_invTP.push_back(TrackInvMass(tagTrack, probeTrack));
	      m_linesTP.push_back(2.5);
	      
	      m_pt.push_back(FTFTrack->pt());
	      m_linespt.push_back(2.5);
	      
	      m_eta.push_back(FTFTrack->eta());
	      m_lineseta.push_back(2.5);
	      
	      m_d0.push_back(FTFTrack->d0());
	      m_linesd0.push_back(2.5);
	      
	      m_phi.push_back(FTFTrack->phi());
	      m_linesphi.push_back(2.5);
	      
	      m_eff.push_back(2.5);
	  
	      FTFfound = true;
	      ATH_MSG_INFO( "Found probe ID candidate from FTF" );
	    }
	  }
	  if(!FTFfound){
	    ATH_MSG_INFO( "Inefficient event in FTF, no ID candidate found" );
	  }
	}
      }
    }
  }
   
  /*if(gotPT){
    bool found = false;
    for(auto PTTrack : *PTTrkContainer){
    if(found) continue;
	
    float massdiff = 300000;
    float PTinvmass = 0;
    if(PTTrack->pt() > m_lowerPtCutID){
    PTinvmass = TrackInvMass(tagTrack, PTTrack);
    massdiff = fabs(m_mumuMass - PTinvmass);
    }
	
    ATH_MSG_DEBUG( "Mass difference of PTtrack and mumuMass = " << massdiff );
    ATH_MSG_DEBUG( "Mass acceptance = " << m_massAcceptanceTID );
	
    if(massdiff < m_massAcceptanceTID){
	  
    m_inv.push_back(invMass);
    m_lines.push_back(3.5);
	  
    m_invTP.push_back(PTinvmass);
    m_linesTP.push_back(3.5);
	  
    m_pt.push_back(PTTrack->pt());
    m_linespt.push_back(3.5);
	  
    m_eta.push_back(PTTrack->eta());
    m_lineseta.push_back(3.5);
	  
    m_d0.push_back(PTTrack->d0());
    m_linesd0.push_back(3.5);
	  
    m_phi.push_back(PTTrack->phi());
    m_linesphi.push_back(3.5);
	  
    m_eff.push_back(3.5);
	  
    found= true;
    ATH_MSG_INFO( "Found PT Candidate" );
    }
    }
    if(!found){
    ATH_MSG_INFO( "Inefficiency in PT" );
    }
    }*/    

  ATH_MSG_INFO( "Finishing TrigIDTPMonitor" );

  return HLT::OK;
}

double TrigIDTPMonitor::TrackInvMass(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2){  

  double pt1 = track1->pt();
  double px1 = track1->p4().Px();
  double py1 = track1->p4().Py();
  double pz1 = track1->p4().Pz();

  double pt2 = track2->pt();
  double px2 = track2->p4().Px();
  double py2 = track2->p4().Py();
  double pz2 = track2->p4().Pz();

  double pX = px1+px2;
  double pY = py1+py2;
  double pZ = pz1+pz2;
  
  double mumasssquared = 11163.69;
  
  double E = sqrt(pt1*pt1 + pz1*pz1 + mumasssquared) + sqrt(pt2*pt2 + pz2*pz2 + mumasssquared);
  double P2 = pX*pX + pY*pY + pZ*pZ;
  double invmass2 = E*E - P2;

  return sqrt(invmass2);
}


double TrigIDTPMonitor::compatibleIDTrack(const xAOD::TrackParticle* tag, const xAOD::TrackParticle* probe, const xAOD::TrackParticle* IDtrack){

  if((tag->charge() != probe->charge()) && (IDtrack->charge() == probe->charge())){

    if((IDtrack->pt() > m_lowerPtCutID) && (probe->pt() > m_lowerPtCutP)
       && (tag->pt() > m_lowerPtCutT) && (fabs(probe->eta()) < m_etaUpperCut)
       && (fabs(tag->eta()) < m_etaUpperCut) ){
      
      double particleM = TrackInvMass(IDtrack, tag);
      
      if(particleM > 0)
	return particleM;
      else
	return 300000.0;
    }
  }

  return 300000.0;
}
