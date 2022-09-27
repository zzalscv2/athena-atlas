/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "MistimedStreamMonitorAlgorithm.h"
#include <iostream>
#include <vector>


MistimedStreamMonitorAlgorithm::MistimedStreamMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
    
    
{
}

StatusCode MistimedStreamMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("MistimedStreamMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_xAODTriggerTowerContainerName "<< m_xAODTriggerTowerContainerName); 

  // we initialise all the containers that we need
  ATH_CHECK( AthMonitorAlgorithm::initialize() );
  ATH_CHECK( m_xAODTriggerTowerContainerName.initialize() );
  ATH_CHECK( m_cpmTowerLocation.initialize());
  ATH_CHECK( m_jetElementLocation.initialize());
  ATH_CHECK( m_trigDec.retrieve() );
  ATH_CHECK( m_ttTool.retrieve());
  ATH_CHECK( m_runParametersContainer.initialize() );
  ATH_CHECK( m_readoutConfigContainer.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode MistimedStreamMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("MistimedStreamMonitorAlgorith::fillHistograms");
  
  

  // Retrieve L1CaloRunParametersContainer
  SG::ReadCondHandle<L1CaloRunParametersContainer> runParameters( m_runParametersContainer, ctx);
  ATH_CHECK(runParameters.isValid()); 

  
  unsigned int readoutConfigID   = runParameters->runParameters(1)->readoutConfigID(); 
  ATH_MSG_DEBUG("RunParameters:: readoutConfigID " <<  readoutConfigID);
  
  SG::ReadCondHandle<L1CaloReadoutConfigContainer> readoutConfig(m_readoutConfigContainer,  ctx);
  
    
  unsigned int channelID = 0;
  unsigned int numFadcSlices = 0; 
  unsigned int l1aFadcSlice = 0; 
  unsigned int readout80ModePpm = 0;


  //the readout config ID tells you, which readoutConfig is loaded. now you can retrieve the l1aFadcSlice from this DB entry
  if (  readoutConfig->readoutConfig(readoutConfigID)->channelId() == readoutConfigID){
    ATH_MSG_DEBUG("readoutConfigID " <<  readoutConfigID);
    channelID =  readoutConfig->readoutConfig(readoutConfigID)->channelId();
    numFadcSlices = readoutConfig->readoutConfig(readoutConfigID)->numFadcSlices();
    l1aFadcSlice = readoutConfig->readoutConfig(readoutConfigID)->l1aFadcSlice();
    readout80ModePpm =  readoutConfig->readoutConfig(readoutConfigID)->readout80ModePpm();
    ATH_MSG_DEBUG("channelID :: " << channelID);
    ATH_MSG_DEBUG("numFadcSlices :: " <<  numFadcSlices);
    ATH_MSG_DEBUG("l1aFadcSlice :: " <<  l1aFadcSlice);
    ATH_MSG_DEBUG("readout80ModePpm :: " <<  readout80ModePpm);
    
  }
  
  // Retrieve jetElementfrom SG
  SG::ReadHandle<xAOD::JetElementContainer> jetElementTES(m_jetElementLocation, ctx);
  if(!jetElementTES.isValid()){
    ATH_MSG_ERROR("No JetElement container found in TES  "<< m_jetElementLocation); 
    return StatusCode::FAILURE;
  }
    

  // Retrieve Core CPM Towers from SG
  SG::ReadHandle<xAOD::CPMTowerContainer> cpmTowerTES(m_cpmTowerLocation, ctx);
  if(!cpmTowerTES.isValid()){
    ATH_MSG_ERROR("No CPMTower container found in TES  "<< m_cpmTowerLocation); 
    return StatusCode::FAILURE;
  }

  // Retrieve Trigger Towers from SG
  SG::ReadHandle<xAOD::TriggerTowerContainer> triggerTowerTES(m_xAODTriggerTowerContainerName, ctx);
  if(!triggerTowerTES.isValid()){
    ATH_MSG_ERROR("No Trigger Tower container found in TES  "<< m_xAODTriggerTowerContainerName); 
    return StatusCode::FAILURE;
  }


  
  // Retrieve EventInfo from SG and save lumi block number, global event number and run number
  unsigned int lumiNo = GetEventInfo(ctx)->lumiBlock();
  unsigned int currentRunNo = ctx.eventID().run_number();  
  unsigned int currentEventNo =  ctx.eventID().event_number();


  
  ATH_MSG_DEBUG("Lumi Block :: " << lumiNo);
  ATH_MSG_DEBUG("Run Number :: " << currentRunNo);
  ATH_MSG_DEBUG("Event Number :: " << currentEventNo);



  Monitored::Scalar<int> cutFlowX = Monitored::Scalar<int>("cutFlowX", 0);
  Monitored::Scalar<int> cutFlowY = Monitored::Scalar<int>("cutFlowY", 0);
  

  
  cutFlowX=All;
  fill(m_packageName,cutFlowX);
  

  //for the algorithm to run, we need the 2 adc slices before and after the l1a-slice
  //add some readout compatibility checks of the algo here
  if(readout80ModePpm){
    if(numFadcSlices < 9){
      ATH_MSG_DEBUG("Number of ADC slices < 9 for 80 MHz readout, algorithm cannot run, aborting...");
      return StatusCode::SUCCESS;
    }
    if(l1aFadcSlice < 4){
      ATH_MSG_DEBUG("L1a readout pointer < 4 for 80 MHz readout, algorithm cannot run, aborting...");
      return StatusCode::SUCCESS;
    }
    if(numFadcSlices - l1aFadcSlice < 4){
      ATH_MSG_DEBUG("L1a readout pointer is at "<< l1aFadcSlice << " with "<< numFadcSlices << "slices at 80 MHz readout mode, algorithm cannot run, aborting...");
      return StatusCode::SUCCESS;
    } 
  }else{
    if(numFadcSlices < 5){
      ATH_MSG_DEBUG("Number of ADC slices < 5 for 40 MHz readout, algorithm cannot run, aborting...");
      return StatusCode::SUCCESS;
    }
    if(l1aFadcSlice < 2){
      ATH_MSG_DEBUG("L1a readout pointer < 2 for 40 MHz readout, algorithm cannot run, aborting...");
      return StatusCode::SUCCESS;
     } 
     if(numFadcSlices - l1aFadcSlice < 2){
       ATH_MSG_DEBUG("L1a readout pointer is at "<< l1aFadcSlice << " with "<< numFadcSlices << "slices at 40 MHz readout mode, algorithm cannot run, aborting...");
       return StatusCode::SUCCESS;
     }  
  }   
  cutFlowX=UnsuitableReadout;
  fill(m_packageName,cutFlowX);

  
  //Select events that fired HLT_mistimedmonj400
  if(! (m_trigDec->isPassed("HLT_mistimemonj400_L1All",TrigDefs::requireDecision))){
    ATH_MSG_DEBUG("TrigDec don't pass HLT_mistimemonj400_L1All");
    return StatusCode::SUCCESS;
  }
  
  cutFlowX=HLT_mistimemonj400;
  fill(m_packageName,cutFlowX,cutFlowY);
  
  //Only select events which passed the L1_J100
  if(! (m_trigDec->isPassed("L1_J100"))){ 
    ATH_MSG_DEBUG("TrigDec don't pass L1_J100");
    return StatusCode::SUCCESS;
  }
  
  
  cutFlowX=L1_J100;
  fill(m_packageName,cutFlowX);

  
   // now classify the tower signals by looking at their FADC counts, if it exceeds 70
  int satCounter = 0; // saturated TT
  int badCounter = 0; // category 2 really bad
  int bad2Counter = 0; // category 4 bad peak 2 
  int bad3Counter = 0; // category 6 bad peak 3 
  int good3Counter = 0; // category 5 good peak 3 
  int good2Counter = 0; // category 5 good peak 3 
  int emActivityCounter = 0; //count number of TT in EM layer with ADC > 70 

  // =====================================================================
  // ================= Container: TriggerTower ===========================
  // =====================================================================

    // Creating a new container for saving pulseClassification
  std::unique_ptr<xAOD::TriggerTowerContainer> ttContainer = std::make_unique<xAOD::TriggerTowerContainer>();
  std::unique_ptr<xAOD::TriggerTowerAuxContainer> ttContainerAux = std::make_unique<xAOD::TriggerTowerAuxContainer>();
  ttContainer->setStore(ttContainerAux.get());
  


  // Creating a new contianer for TT with pulseClasification 
  for (const xAOD::TriggerTower* tt : *triggerTowerTES) {
  
    float ttPulseCategory = 0;
    const std::vector<uint16_t>& ttADC =  (tt)->adc();
    std::vector<uint16_t> readoutCorrectedADC; //this is the standard readout ADC vector: 5 40MHz samples with l1A in the middle
    if(!readout80ModePpm){//40 MHz
      //just acess the acd vector, as the sanity checks where done above
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice-2));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice-1));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice+1));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice+2));
    }else{//80 MHz
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice-4));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice-2));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice+2));
      readoutCorrectedADC.push_back(ttADC.at(l1aFadcSlice+4));
    }
    
    // retrieve max ADC value and position, this seems to be buggy in the DAOD
    auto maxValIterator = std::max_element(readoutCorrectedADC.begin(), readoutCorrectedADC.end());
    int maxADCval = *maxValIterator;
    int adcPeakPositon = std::distance(std::begin(readoutCorrectedADC), maxValIterator);
    
    if(maxADCval < 70){
      ttPulseCategory = 0.1;
    }else if(maxADCval == 1023) {
      satCounter++;
      ttPulseCategory = 1;
      if(! (tt)->layer()) emActivityCounter++;
    }else{
      bool goodQual = pulseQuality(readoutCorrectedADC, adcPeakPositon);
      if(! (tt)->layer()) emActivityCounter++;
      //look at any of the five FADC values
      if(adcPeakPositon == 2){ // can be class 3 or 4 now
	if(goodQual){
	  good2Counter++;
	  ttPulseCategory = 3;
	}else{
	  bad2Counter++;
	  ttPulseCategory = 4;
	}
      }else if(adcPeakPositon == 3){ // can be class 5 or 6 now
	if(goodQual){
	  good3Counter++;
	  ttPulseCategory = 5;
	}else{
	  bad3Counter++;
	  ttPulseCategory = 6;
	}
      }else{
          //this is class 2 - really bad
	badCounter++;
	ttPulseCategory = 2;
      }
    }
    
    // decorate the TT in order to have to recompute the pulse categorisation
    xAOD::TriggerTower* newTT = new xAOD::TriggerTower; //create a new TT object
    ttContainer->push_back(newTT); // add the newTT to new output TT container (at the end of it)
    *newTT = *(tt);// copy over all information from TT to newTT
    newTT->auxdata<float>("pulseClassification") = ttPulseCategory; //decorate
  }





  if(badCounter > 4){
    //reject events with more than 4 wrongly peaked towers
    return StatusCode::SUCCESS;
  }
  
  cutFlowX=badpeakTT;
  fill(m_packageName,cutFlowX);
  
  if(bad2Counter > 4){
    //reject events with more than 4 pulses peaking in slice 2 that are badly timed or mis-shapen
    return StatusCode::SUCCESS;
  }
  cutFlowX=badCentralTT;
  fill(m_packageName,cutFlowX);
  
  
  if(bad3Counter > 4){
    //reject events with more than 4 pulses peaking in slice 3 that are badly timed or mis-shapen
    return StatusCode::SUCCESS;
  }
  cutFlowX=badLateTT;
  fill(m_packageName,cutFlowX);
  
    
  if(good3Counter < 2){
    //reject events with less than 2 pulses nicely peaking in slice 3 
    return StatusCode::SUCCESS;
  }
  cutFlowX=lateTT;
  fill(m_packageName,cutFlowX);

  if(good2Counter > 2){
    //reject events with more than 2 pulses nicely peaking in slice 2 to avoid event being triggered by pileup 
    return StatusCode::SUCCESS;
  }
  cutFlowX= InTime;
  fill(m_packageName,cutFlowX);
    
  if(!emActivityCounter){
    //reject events with no activity in the EM layer
    return StatusCode::SUCCESS;
  }
  cutFlowX=  TTEMLayer;
  fill(m_packageName,cutFlowX);
  
    
  
  // scope for mutable error event per lumi block tt counter
  // it allows only one event per lumiblock
  std::lock_guard<std::mutex> lock(m_mutex);	
  m_event_counter[lumiNo]+=1;
  const int eventCounter = m_eventCounter++;
 
  



  if( (m_event_counter[lumiNo] <=1) && (eventCounter < m_maxEvents) ){ 
    ATH_MSG_DEBUG( "EventID :: " <<  m_event_counter[lumiNo]);
    
      // Saving the lumiblock and event number of the events with mistimed 
      auto  eventMonitor= Monitored::Scalar<std::string>("eventMonitor", "Event"+std::to_string(eventCounter)+"="+std::to_string(currentEventNo));
      auto  lbMonitor= Monitored::Scalar<std::string>("lbMonitor", std::to_string(lumiNo));
      std::string groupName = "Event_";
      fill(groupName, eventMonitor, lbMonitor );
    

      // Create a vector of trigger towers with quantities to be monitored
      std::vector<MonitorTT> vecMonTTDecor;     // All towers
  
      
      // Loop over trigger tower container
      //Create the trigger tower objects and calculate scaled phi
      for (const xAOD::TriggerTower* tt : *ttContainer) {
	ATH_CHECK( makeTowerPPM(tt, vecMonTTDecor) );     
	ATH_MSG_DEBUG( "tt->pulseClassification :: " <<  tt->auxdata<float>("pulseClassification"));
	
      }

      
      groupName = "EventofInterest_" +  std::to_string(eventCounter) + "_";
      auto  bcidWord  = Monitored::Scalar<uint8_t>("bcidWord",  0);
      auto  pulseCat = Monitored::Scalar<float>("pulseCat",  0);
      
      for (auto& myTower : vecMonTTDecor) {
	ATH_MSG_DEBUG(" looping over TTs"); 
	const int layer =   (myTower.tower)->layer();
	pulseCat = (myTower.tower)->auxdata<float>("pulseClassification");
	bcidWord = (myTower.tower)->bcidVec()[0]; // look at the status bit in the central time slice
	ATH_MSG_DEBUG("groupName :: " <<  groupName);
	
	// Check if TT is in EM or HAD layer:
      if (layer == 0) { //========== ELECTROMAGNETIC LAYER =========================
	ATH_CHECK( fillPPMEtaPhi(myTower, groupName+"TT_EM", "pulseCat",  pulseCat) ); 
	if(pulseCat > 0.5 && bcidWord > 0)  {
	  ATH_CHECK( fillPPMEtaPhi(myTower, groupName+"TT_EM", "bcidWord",  bcidWord) ); 
	  
	}
	}
      else if(layer == 1 ) { //========== HADRONIC LAYER ===============================
	ATH_CHECK( fillPPMEtaPhi(myTower, groupName+"TT_HAD", "pulseCat",  pulseCat) ); 
	if(pulseCat > 0.5 && bcidWord > 0 )  ATH_CHECK( fillPPMEtaPhi(myTower, groupName+"TT_HAD", "bcidWord",  bcidWord) ); 
      }
      
      }



      //Loop over CPM tower container
      //Create the CPM objects and calculate scaled phi
      std::vector<MonitorCPM> vecMonCPM;     // All towers
      for (const xAOD::CPMTower* cpm : *cpmTowerTES) {
	ATH_CHECK( makeTowerCPM(cpm, vecMonCPM) );     
	
      }

    
      // Coordinates for CPM tower and JetElement containers 
      auto etalut = Monitored::Scalar<double>("etalut", 0);
      auto philut = Monitored::Scalar<double>("philut", 0);
      

      // lut variables  
      auto  emLUT0 =  Monitored::Scalar<int>("emLUT0", 0);
      auto  emLUT1 =  Monitored::Scalar<int>("emLUT1", 0);
      auto  emLUT2 =  Monitored::Scalar<int>("emLUT2", 0);
      auto  hadLUT0 =  Monitored::Scalar<int>("hadLUT0", 0);
      auto  hadLUT1 =  Monitored::Scalar<int>("hadLUT1", 0);
      auto  hadLUT2 =  Monitored::Scalar<int>("hadLUT2", 0);


      


      //loop over the cpm tower container to fill the lut histos 
      for (auto& myTower : vecMonCPM) {

	std::vector<uint8_t> cpmEMenergy = (myTower.tower)->emEnergyVec();
	std::vector<uint8_t> cpmHADenergy = (myTower.tower)->hadEnergyVec();
	// eta scaled 
	etalut = myTower.etaScaled;
	
	for (auto phi:  myTower.phiScaled) {
	  // phi scaled
	  philut = phi;
	  
	  if(cpmEMenergy.size() > 2){ // expect 3 slices to be read out
	    ATH_MSG_DEBUG("CPM :: emLUT0 :: " <<  unsigned(cpmEMenergy.at(0)) << ":: emLUT1 :: " <<  unsigned(cpmEMenergy.at(1)) << ":: emLUT2 :: "  <<  unsigned(cpmEMenergy.at(2)));
	  
	    emLUT0 = (int)cpmEMenergy.at(0);
	    if(cpmEMenergy.at(0) > 0) fill(groupName+"lut_EM0",etalut,philut, emLUT0);
	  
	    emLUT1 =  (int) cpmEMenergy.at(1);
	    if(cpmEMenergy.at(1) > 0) fill(groupName+"lut_EM1",etalut,philut, emLUT1);
	
	    emLUT2 = (int) cpmEMenergy.at(2);
	    if(cpmEMenergy.at(2) > 0)  fill(groupName+"lut_EM2",etalut,philut, emLUT2);
	  }
	  if(cpmHADenergy.size() > 2){
	    ATH_MSG_DEBUG("CPM :: hadLUT0 :: " <<  unsigned(cpmHADenergy.at(0)) << ":: hadLUT1 :: " <<  unsigned(cpmHADenergy.at(1)) << ":: hadLUT2 :: "  <<  unsigned(cpmHADenergy.at(2)));
	  
	    hadLUT0 = (int) cpmHADenergy.at(0);
	    if(cpmHADenergy.at(0) > 0) fill(groupName+"lut_HAD0",etalut,philut, hadLUT0);
	    hadLUT1 =  (int) cpmHADenergy.at(1);
	    if(cpmHADenergy.at(1) > 0) fill(groupName+"lut_HAD1",etalut,philut, hadLUT1);
	    hadLUT2 =  (int) cpmHADenergy.at(2);
	    if(cpmHADenergy.at(2) > 0)fill(groupName+"lut_HAD2",etalut,philut, hadLUT2);

	  }
	}}
      

   
   
      
      std::vector<MonitorJE> vecMonJE;     // All elements

    //Create the JetElement objects and calculate scaled phi
    for (const xAOD::JetElement* je : *jetElementTES) {
      ATH_CHECK( makeTowerJE(je, vecMonJE) );     
      
    }
    
    //loop over the jet element container to fill the lut histos 
    for (auto& jet : vecMonJE) {
      
      std::vector<uint16_t> jepEMenergy = (jet.element)->emJetElementETVec();
      std::vector<uint16_t> jepHADenergy = (jet.element)->hadJetElementETVec();
      for (auto eta: jet.etaScaled) {
	etalut = eta;
	if ( std::abs(eta) > 2.5){
	  for (auto phi:  jet.phiScaled) {
	    philut = phi;
	    if(jepEMenergy.size() > 2){
	      ATH_MSG_DEBUG("JetElement :: emLUT0 :: " <<  unsigned(jepEMenergy.at(0)) << ":: emLUT1 :: " <<  unsigned(jepEMenergy.at(1)) << ":: emLUT2 :: "  <<  unsigned(jepEMenergy.at(2)));

	      emLUT0 = (int)jepEMenergy.at(0);
	      if(jepEMenergy.at(0) > 0) fill(groupName+"lut_EM0",etalut,philut, emLUT0);
	  
	      emLUT1 = (int)jepEMenergy.at(1);
	      if(jepEMenergy.at(1) > 0) fill(groupName+"lut_EM1",etalut,philut, emLUT1);
	  
	      emLUT2 = (int)jepEMenergy.at(2);
	      if(jepEMenergy.at(2) > 0) fill(groupName+"lut_EM2",etalut,philut, emLUT2);
	  }
	
	    if(jepHADenergy.size()> 2){

	    ATH_MSG_DEBUG("JetElement :: hadLUT0 :: " <<  unsigned(jepHADenergy.at(0)) << ":: hadLUT1 :: " <<  unsigned(jepHADenergy.at(1)) << ":: hadLUT2 :: "  <<  unsigned(jepHADenergy.at(2)));
	  
	    hadLUT0 = (int)jepHADenergy.at(0);
	    if(jepHADenergy.at(0) > 0) fill(groupName+"lut_HAD0",etalut,philut, hadLUT0);
	    
	    hadLUT1 = (int)jepHADenergy.at(1);
	    if(jepHADenergy.at(1) > 0) fill(groupName+"lut_HAD1",etalut,philut, hadLUT1);
	    
	    hadLUT2 = (int)jepHADenergy.at(2);
	    if(jepHADenergy.at(2) > 0)  fill(groupName+"lut_HAD2",etalut,philut, hadLUT2);
	  }}
	}}}
  }

  else if ( eventCounter >= m_maxEvents ) { 
    auto  eventMonitor_all= Monitored::Scalar<std::string>("eventMonitor_all", std::to_string(currentEventNo));
    auto  lbMonitor_all= Monitored::Scalar<std::string>("lbMonitor_all", std::to_string(lumiNo));
    fill("Event_all_", eventMonitor_all, lbMonitor_all );
  }
  


  return StatusCode::SUCCESS;
}


StatusCode  MistimedStreamMonitorAlgorithm::makeTowerPPM( const xAOD::TriggerTower* tt, 
                                                    std::vector<MonitorTT> &vecMonTT) const
{
  // Geometry
  const double phi = tt->phi();
  double phiMod = phi * m_phiScaleTT;
 
  
 
  // Fill TT quantities
  MonitorTT monTT;
  monTT.tower = tt;
  monTT.phiScaled = phiMod;
  vecMonTT.push_back(monTT);
   
  return StatusCode::SUCCESS; 
}


StatusCode  MistimedStreamMonitorAlgorithm::makeTowerCPM( const xAOD::CPMTower* cpm, 
                                                    std::vector<MonitorCPM> &vecMonCPM) const
{
  // Geometry
  const double phi = cpm->phi();
  double phiMod = phi * m_phiScaleTT;
  
 
  // Fill CPM quantities
  MonitorCPM monCPM;
  monCPM.tower = cpm;
  
  double etaMod = monCPM.tower->eta();
  const double absEta = std::abs(etaMod);
  
  const std::vector<double> offset32 = {1.5, 0.5, -0.5, -1.5};                                                                             
  const std::vector<double> offset25 = {0.5, -0.5}; 
  std::vector<double> offset = {}; 
  
  if (absEta > 3.2) {
    // Fill four bins in phi
    phiMod = std::floor(phiMod/4)*4. + 2.;
    offset = offset32;
  } 
  else if (absEta > 2.5) {
    // Fill two bins in phi
    phiMod = std::floor(phiMod/2)*2. + 1.;
    offset = offset25;
  }     
  else {
    offset = {0.};
  }

  
  
  // Fill the histograms 
  for (auto phiOffset : offset)  {
    monCPM.phiScaled.push_back(phiMod + phiOffset);
  }      

  monCPM.etaScaled = etaMod;


  vecMonCPM.push_back(monCPM);
   
  return StatusCode::SUCCESS; 
}

StatusCode  MistimedStreamMonitorAlgorithm::makeTowerJE( const xAOD::JetElement* je, 
                                                    std::vector<MonitorJE> &vecMonJE) const
{

  // Use JEP info to fill the forward part of the lut plots, but since this has TT granularity we have to play some tricks
  
  // Geometry
  const double phi = je->phi();
  double phiMod = phi * m_phiScaleTT;
  
  // Fill JE quantities
  MonitorJE monJE;
  monJE.element = je;

  double etaMod = monJE.element->eta();
  const double absEta = std::abs(etaMod);
  int signeta = 1;         
  if( etaMod < 0) signeta = -1;

  
  const std::vector<double> offset32 = {1.5, 0.5, -0.5, -1.5};                                                                             
  const std::vector<double> offset25 = {0.5, -0.5}; 
  std::vector<double> offset = {}; 
  
  if (absEta > 3.2) {
    // Fill four bins in phi
    phiMod = std::floor(phiMod/4)*4. + 2.;
    offset = offset32;
    monJE.etaScaled.push_back(signeta*4.7);
    monJE.etaScaled.push_back(signeta*3.7);
    monJE.etaScaled.push_back(signeta*3.5);

  } 
  else if(absEta > 2.9) {
    phiMod = std::floor(phiMod/2)*2. + 1.;
    offset = offset25;
    monJE.etaScaled.push_back(signeta*3.15);
  }
  
 
  if (absEta > 2.5) {
    // Fill two bins in phi
    phiMod = std::floor(phiMod/2)*2. + 1.;
    offset = offset25;
    monJE.etaScaled.push_back(etaMod);
  }     
  else {
    offset = {0.};
    monJE.etaScaled.push_back(etaMod);
  }
  
  
  // Fill the histograms 
  for (auto phiOffset : offset)  {
    monJE.phiScaled.push_back(phiMod + phiOffset);
  }      

  vecMonJE.push_back(monJE);

  return StatusCode::SUCCESS; 
}


StatusCode  MistimedStreamMonitorAlgorithm::fillPPMEtaPhi( MonitorTT &monTT, 
                                           const std::string& groupName, 
                                           const std::string& weightName,
                                           double weight) const {
  
 
  double phiMod = monTT.phiScaled;  // Integer binning for 2D plots 
  double etaMod = monTT.tower->eta();
  const double absEta = std::abs(etaMod);
  
  const std::vector<double> offset32 = {1.5, 0.5, -0.5, -1.5};                                                                             
  const std::vector<double> offset25 = {0.5, -0.5}; 
  std::vector<double> offset = {}; 
    
  if (absEta > 3.2) {
    // Fill four bins in phi
    phiMod = std::floor(phiMod/4)*4. + 2.;
    offset = offset32;
  } 
  else if (absEta > 2.5) {
    // Fill two bins in phi
    phiMod = std::floor(phiMod/2)*2. + 1.;
    offset = offset25;
  }     
  else {
    offset = {0.};
  }

  ATH_MSG_DEBUG("absEta: " << absEta << "offset.size(): " << offset.size());
 
  // Fill the histograms 
  for (auto phiOffset : offset)  {

    auto etaTT_2D = Monitored::Scalar<double>("etaTT_2D", etaMod);
    auto phiTT_2D = Monitored::Scalar<double>("phiTT_2D", phiMod + phiOffset);
    
    auto weight_2D = Monitored::Scalar<double>(weightName, weight); // Weight for filling 2D profile histograms; name must be included in python histogram definition
    ATH_MSG_DEBUG("groupName: weight_2D" << weight_2D); 
    
    fill(groupName, etaTT_2D, phiTT_2D, weight_2D);
    
  }      

  return StatusCode::SUCCESS;
} 







bool MistimedStreamMonitorAlgorithm::pulseQuality(const std::vector<uint16_t> ttPulse, int peakSlice) const {
     
    bool goodPulse = true;
    int size = ttPulse.size();
    if (peakSlice > size) {
      ATH_MSG_ERROR("Peak Slice " << peakSlice << " supress the ttPulse vector size "  <<  size ); 
    }

    int a = ttPulse[peakSlice-1];
    int b = ttPulse[peakSlice];
    int c = ttPulse[peakSlice+1];
    double tim = (0.5*a-0.5*c)/(a+c-2*b);
    double wid = (a+c-64.0)/(b-32.0);
    if ( tim < 0.0 ) goodPulse = false;
    else if ( tim > 0.3 ) goodPulse = false;
    if ( wid < 1.0 ) goodPulse = false;
    else if ( wid > 1.6 ) goodPulse = false;             

    ATH_MSG_DEBUG("Pulse qual= "<< goodPulse<<" tim = "<<tim<<" wid = "<<wid);
    ATH_MSG_DEBUG("a = "<< a <<" b = "<<b<<" c = "<<c);
    
    return goodPulse;
}
