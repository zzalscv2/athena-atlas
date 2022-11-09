/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetEfficiencyMonitorAlgorithm.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "FourMomUtils/P4Helpers.h"

#include <cstdlib>


JetEfficiencyMonitorAlgorithm::JetEfficiencyMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JetEfficiencyMonitorAlgorithm::initialize() {
  ATH_MSG_DEBUG("JetEfficiencyMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);

  // we initialise all the containers that we need
  ATH_CHECK(m_jetKey.initialize()); //initialize offline jets
  ATH_CHECK(m_LRjetKey.initialize()); //initialize offline LR jets 
  ATH_CHECK(m_gFexSRJetContainerKey.initialize()); //initizlize gfex sr jets
  ATH_CHECK(m_gFexLRJetContainerKey.initialize()); //initizlize gfex lr jets

  return AthMonitorAlgorithm::initialize();
}

StatusCode JetEfficiencyMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  ATH_MSG_DEBUG("JetEfficiencyMonitorAlgorithm::fillHistograms");
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;

    //  Retrieve Offline Jets from SG
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey,ctx);
  if(!jets.isValid()){
    ATH_MSG_WARNING("Failed to retrieve Offline JetContainer");
    return StatusCode::SUCCESS;
  }
   //  Retrieve Offline LR Jets from SG
  SG::ReadHandle<xAOD::JetContainer> LRjets(m_LRjetKey,ctx);
  if(!LRjets.isValid()){
    ATH_MSG_WARNING("Failed to retrieve Offline LR JetContainer");
    return StatusCode::SUCCESS;
  }
  //  Retrieve gfex SR Jets from SG
  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexSRJetContainer{m_gFexSRJetContainerKey, ctx};
  if(!gFexSRJetContainer.isValid()){
    ATH_MSG_WARNING("No gFex SR Jet container found in storegate  "<< m_gFexSRJetContainerKey);
    return StatusCode::SUCCESS;
  }
  //  Retrieve gfex LR Jets from SG
  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexLRJetContainer{m_gFexLRJetContainerKey, ctx};
  if(!gFexLRJetContainer.isValid()){
    ATH_MSG_WARNING("No gFex LR Jet container found in storegate  "<< m_gFexLRJetContainerKey);
    return StatusCode::SUCCESS;
  }

  
  //////////DEFINITIONS and extracting variables from the python config file! /////////////////////
  bool use_emulated_gfex_trig =  m_emulated; //UPDATE THE NAME OF THE HISTOGRAMS IN THE THE PYTHON FILE TO SAY EMULATED OR NOT AS THIS CHANGES
  std::string  bootstrap_trigger = m_bootstrap_trigger;
  std::string orthogonal_trigger = "L1_RD0_FILLED"; //use the same orthogonal trigger for all kinds of jets! 
  std::vector<std::string> unbiased_triggers = {"L1_MU14FCH", "L1_MU18VFCH",  
                "L1_MU8F_TAU20IM", "L1_2MU8F", "L1_MU8VF_2MU5VF", "L1_3MU3VF",
                "L1_MU5VF_3MU3VF", "L1_4MU3V", "L1_2MU5VF_3MU3V", 
                "L1_RD0_FILLED"};
  std::vector<std::string> gFex_types {"SR", "LR"};
  std::vector<std::string> ref_trig {"BS", "ortho", "none", "unbiased"};
  constexpr int minPt = 15*GeV;
  constexpr float maxEta = 2.8;
  
  std::map<std::string, SG::ReadHandle<xAOD::gFexJetRoIContainer>> gFEX_Container {
    {"SR",gFexSRJetContainer}, {"LR", gFexLRJetContainer}};

  bool SR_gfex_physical_cuts_passed = false, LR_gfex_physical_cuts_passed = false;
  std::map<std::string, bool> physical_cuts_passed_type {
    {"SR", SR_gfex_physical_cuts_passed}, {"LR", LR_gfex_physical_cuts_passed}};

  bool SR_delta_jets_cuts_passed = false, LR_delta_jets_cuts_passed = false; 
  std::map<std::string, bool> delta_jets_cuts_passed {
    {"SR", SR_delta_jets_cuts_passed }, {"LR", LR_delta_jets_cuts_passed}};


//fill mapping to keep track of variables according to the three containers we like to acess
  double offline_pt = 0, gfex_SR_pt = 0, gfex_LR_pt=0;
  std::map<std::string, double> jet_pt {
    {"offline", offline_pt}, {"SR", gfex_SR_pt}, {"LR", gfex_LR_pt}};
  
  double offline_eta = 0, gfex_SR_eta = 0, gfex_LR_eta=0;
  std::map<std::string, double> jet_eta {
    {"offline", offline_eta}, {"SR", gfex_SR_eta}, {"LR", gfex_LR_eta}};
  
  double offline_phi = 0, gfex_SR_phi = 0, gfex_LR_phi=0;
  std::map<std::string, double> jet_phi {
    {"offline", offline_phi}, {"SR", gfex_SR_phi}, {"LR", gfex_LR_phi}};


  ////////////////////////////////////////////////////////////
  //////////////// Fille pt, eta and phi vals ////////////////
  ///////////////////////////////////////////////////////////
  //offline
  if (!jets->empty()) { //check that there are jets before accessing the container
    xAOD::JetContainer::const_iterator jet_itr = jets->begin();
    jet_pt["offline"] = (*jet_itr)->pt(), jet_eta["offline"] = (*jet_itr)->eta(), jet_phi["offline"] = (*jet_itr)->phi();
  } //(close IF) jets size > 0 loop
  //gFex SR and LR
  // gfex jets are not sorted according to size, so we need to look for the leading jets manually
  for (auto & g : gFex_types){ //iterate through SR and LR gfex jets.
    if (!gFEX_Container[g]->empty()) { //check that there are jets before accessing the container
      double max_pt = 0.0; //zero max pt to start, so we can keep iterating and find actaul leading gfex jet
      const xAOD::gFexJetRoI* lead_gfex_jet = nullptr;
      for (const auto* gfex_jet : *gFEX_Container[g]) {
        const double check_jet_pt = gfex_jet->et(), check_jet_eta = gfex_jet->eta();
        if (check_jet_pt > max_pt && std::abs(check_jet_eta) < maxEta){
          max_pt = check_jet_pt;
          lead_gfex_jet = gfex_jet;
        } //(close IF) loop if gfex jet satisfies pt and eta conditions
      } //(close FOR) loop that iterates through gfex tobs
      if (lead_gfex_jet != nullptr) {
        jet_eta[g] = lead_gfex_jet->eta(), jet_phi[g] = lead_gfex_jet->phi(), jet_pt[g] = lead_gfex_jet->et();
      } // (close IF) loop that checks if there is a leading gfex jet
    } // (close IF) loop that checks that there are gfex jets
  }// (close FOR) that loops through sr and lr gfex jets

  ////////////////////////////////////////////////////////////
  /////////////////////DEFINE PHYSICAL CUTS //////////////////
  ////////////////////////////////////////////////////////////
  //offline
  bool physical_cuts_passed = false;
  if(std::abs(jet_eta["offline"])<maxEta && (jet_pt["offline"] > minPt)) { physical_cuts_passed = true; }
  //gFex SR and LR
  for (auto & g : gFex_types){ //iterate through SR and LR gfex jets.
    if (std::abs(jet_eta[g])<maxEta && (jet_pt[g] > minPt) ) {physical_cuts_passed_type[g]=true; }
    if (physical_cuts_passed && physical_cuts_passed_type[g]) {
      double delta_eta = std::abs(jet_eta[g]-jet_eta["offline"]),
             delta_phi = P4Helpers::deltaPhi(jet_phi[g],jet_phi["offline"]);
      const double delta_r = std::sqrt(std::pow(delta_eta,2)+ std::pow(delta_phi,2));
      if(delta_r < 1) { delta_jets_cuts_passed[g] = true; }
    }//(close IF) loop if physical cuts for offlien jets it passed and physical cuts for the gfex jet are passed
  }//(closed IF) loop that checks that there is a leading gFex jet
  ////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  bool unbiased_trig_decision = false;
  for (auto & u : unbiased_triggers) {if (AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(u)) {unbiased_trig_decision = true;}}
  std::map<std::string, bool> ref_trigger_decision {
      {"BS", AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(bootstrap_trigger) }, 
      {"ortho", AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(orthogonal_trigger) },
      {"none", true},
      {"unbiased", unbiased_trig_decision}};


  auto run = Monitored::Scalar<int>("run",GetEventInfo(ctx)->runNumber());
  fill(m_packageName, run); //fill the run number histogram that was default



  
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  //////////////PREP LISTS OF TRIGGERS //////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////


  //Create and clean up the list of L1Triggers
  std::vector<std::string> L1Triggeritems = AthMonitorAlgorithm::getTrigDecisionTool()->getChainGroup("L1_.*J.*")->getListOfTriggers();
  std::vector<std::string> useful_L1triggers, excluded_L1triggers, plotting_trigger_list = m_L1TriggerList;

  for(unsigned int t=0; t<L1Triggeritems.size(); ++t){//iterate through all triggers of format L1_.*J.*
    auto prescale_value = AthMonitorAlgorithm::getTrigDecisionTool()->getPrescale(L1Triggeritems[t] );
    if (prescale_value == 1){ useful_L1triggers.push_back(L1Triggeritems[t]); } // add unprescaled triggers to a list 
  } //close for loop iteration that looks at all triggers

  std::map<std::string, int> l1_trigger_flatline_vals { //this is around where the l1trigger pt effiencies flatten out
  //in order to make eta effiency curves, its useful to isolate the pt behavior, so we want 100% pt effiency, or after 'flattening out' 
      {"L1_J15", 35*GeV}, {"L1_J20", 40*GeV},  {"L1_J30", 60*GeV},  
      {"L1_J40", 80*GeV}, {"L1_J50", 90*GeV}, {"L1_J75", 120*GeV}, 
      {"L1_J100", 140*GeV} };
  
 /////////////////////////////////////////////////////////////
 //Create and clean up lists of LR and SR gfex triggers
  std::vector<std::string> useful_SR_gFexTriggers, useful_LR_gFexTriggers;
  std::map<std::string, double> useful_SR_gFexTriggers_val { //trigger val corresponding to the SR gfex triggers
      {"L1_gJ20", 20*GeV}, {"L1_gJ30", 30*GeV},  {"L1_gJ40", 40*GeV},  
      {"L1_gJ50", 50*GeV}, {"L1_gJ60", 60*GeV}, {"L1_gJ100", 100*GeV},
      {"L1_gJ160", 160*GeV} };

  std::map<std::string, double> useful_LR_gFexTriggers_val {
      {"L1_gLJ80", 80*GeV}, {"L1_gLJ100", 100*GeV},  
      {"L1_gLJ140", 140*GeV},  {"L1_gLJ160", 160*GeV} };

  std::map<std::string, std::vector<double>> multijet_triggers_val {
      {"L1_3J50", {50*GeV, 50*GeV, 50*GeV}}, {"L1_4J15", {15*GeV, 15*GeV, 15*GeV, 15*GeV}},  
      {"L1_4J20", {20*GeV, 20*GeV, 20*GeV, 20*GeV}}, {"L1_J85_3J30", {85*GeV, 30*GeV, 30*GeV, 30*GeV}},
    {"L1_2J15_XE55", {15*GeV, 15*GeV}}, {"L1_2J50_XE40", {50*GeV, 50*GeV}}};

  for (auto & g : gFex_types){ //iterate through the gfex jet tob types (SR & LR) 
    std::string trigger_search;
    std::vector<std::string> excluded_gfex_triggers, plotting_gfex_trigger_list, useful_gFexTriggers;
    std::map<std::string, double> trigger_map;
    if (g =="SR") { 
      trigger_search = "L1_gJ.*";
      trigger_map = useful_SR_gFexTriggers_val;
      plotting_gfex_trigger_list = m_SRgfexTriggerList;
    } else if (g == "LR") {
      trigger_search = "L1_gLJ.*";
      trigger_map = useful_LR_gFexTriggers_val;
      plotting_gfex_trigger_list = m_LRgfexTriggerList;
    } //(close IF) loop that defines varaibles for LR and SR gfex jet tob types

    if (use_emulated_gfex_trig){ //if we are working with not run3 data, then we need to emulate our simple gfex jet triggers by applying cuts
      for(std::map<std::string, double>::iterator it = trigger_map.begin(); it != trigger_map.end(); ++it) {
        useful_gFexTriggers.push_back(it->first); //write the emualted triggers as the keys from the map written above
      } //(close FOR) loop that iterates through all of the triggers in the emulated trigger list
    } //(close IF) loop about if we are using an emulated trigger
    else { //if we are not using the emulated gfex triggers
      std::vector<std::string> TDT_triggers = AthMonitorAlgorithm::getTrigDecisionTool()->getChainGroup(trigger_search)->getListOfTriggers();
      for(unsigned int t=0; t< TDT_triggers.size(); ++t){
        auto prescale_value = AthMonitorAlgorithm::getTrigDecisionTool()->getPrescale(TDT_triggers[t]);
        if (prescale_value == 1){  //check if the trigger is unprescaled 
          auto itr = std::find(plotting_gfex_trigger_list.begin(), plotting_gfex_trigger_list.end(),TDT_triggers[t] );
          if (itr == plotting_gfex_trigger_list.cend() ) {
            excluded_gfex_triggers.push_back(TDT_triggers[t]);  
          } useful_gFexTriggers.push_back(TDT_triggers[t]); 
        } //(close IF) prescale value == 1 
      } //(close FOR) loop that iterates over the full list or trigger decision tool triggers 
    } //(close ELSE )loop that means we are not loooking at emulated gfex triggers 
  } //(close FOR) the iteration that goes through the "gfex types" -- SR and LR gfex jet tobs 
  //define emulatedString variable so that we can have emulated in the title (or not) according to the status of the gfex triggers 
  std::string emulatedString = " ";
  if (use_emulated_gfex_trig) {emulatedString = " Emulated "; } 
  std::map<std::string,  std::vector<std::string>> gFEX_triggers {
    {"SR",useful_SR_gFexTriggers}, {"LR",useful_LR_gFexTriggers}};
  std::map<std::string, std::map<std::string, double>> gFEX_triggers_val {
    {"SR",useful_SR_gFexTriggers_val}, {"LR",useful_LR_gFexTriggers_val}};

  
  //////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////
  //////////lets see what triggers are firing when we get a jet with more than 100pt?///////
  //////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::string> passed_triggers;
  std::vector<std::string> unprescaled_trigger_list;
  
  if (jet_pt["offline"] > 100*GeV){
    auto raw_pt  = Monitored::Scalar<int>("raw_pt", 0.);
    raw_pt = jet_pt["offline"];
    fill(m_packageName, raw_pt);  //fill a histogram of all pt values with no physical cuts
    
    std::vector<std::string> fired_trigger_list = m_TriggerList; //defined list of triggers we think will show up in the python config file
    std::vector<std::string> trigger_list = AthMonitorAlgorithm::getTrigDecisionTool()->getChainGroup("L1.*")->getListOfTriggers();

    for (unsigned int t=0; t< trigger_list.size(); ++t){ //iterate through all L1.* triggers present for the event 
      auto prescale_value = AthMonitorAlgorithm::getTrigDecisionTool()->getPrescale(trigger_list[t] );     
      if (prescale_value == 1) { unprescaled_trigger_list.push_back(trigger_list[t]); } //if unprescaled, then add to unprescaled trigger list
    } //(close FOR) loop that iterates trhoguh the list of all L1.* triggers present for the event 

    for (unsigned int t=0; t< unprescaled_trigger_list.size(); ++t){ //iterate through unprescaled trigger list 
      bool passed = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(unprescaled_trigger_list[t]);
      if (passed) { //check to see if the unprescaled trigger passed
        passed_triggers.push_back(unprescaled_trigger_list[t]); 
        auto itr = std::find(fired_trigger_list.begin(), fired_trigger_list.end(), unprescaled_trigger_list[t]); //extract the index of this triggerin plotting list
        if (itr != fired_trigger_list.cend()) {
          int index =  std::distance(fired_trigger_list.begin(), itr);
          auto trigger_list_index  = Monitored::Scalar<int>("otherTriggers", index);
          fill(m_packageName, trigger_list_index); //add count to this trigger's histogram
        } //(close IF) loop that checks if the fired trigger list item is in our array to be plotted
      } //(close IF)loop that checks if the present trigger of interest is passed
    } //(close FOR)loop that iterates throguh all the unprescale triggers present for the event
  } //(close IF) loop that checks if the leading jet has pt 100Gev or greater






  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////// FILL EFFIENCY HISTOGRAMS FOR THE OFFLINE JETS ///////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  if (physical_cuts_passed) { //check that the physical cuts are passed
    for (auto & r : ref_trig){ //iterate through the 4 different kinds of refernce triggers
      if (ref_trigger_decision[r]){ //check that the refernce trigger is passed (defintion of this is at the beggining of the file!)
        for(unsigned int t=0; t< useful_L1triggers.size(); ++t){ //iterate through all of the useful triggers list (that we make effiency curves for)
          std::string trigger_name = useful_L1triggers[t]; // define the trigger name
          bool trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name);

          //if we are dealing with a multijet trigger we want to add the "last" jet in the multi
          //loop through the the list of multijet triggers to see if the trigger we are looking at is present
          bool multijet_trigger = false;
          if (multijet_triggers_val.find(trigger_name) != multijet_triggers_val.end()) {
            multijet_trigger = true;
            unsigned int multijet_num = multijet_triggers_val[trigger_name].size();
            xAOD::JetContainer::const_iterator jet_itr_loop = jets->begin();
            if (jets->size() >= multijet_num) { //if the trigger is present, check that the number of jets in this event is geq to the number of jets required by trigger
              for (unsigned int m=0; m< multijet_num; ++m) { //iterate through each jet required by trigger and check that the value is satisfied
                const double jet_pt_loop = (*jet_itr_loop)->pt();
                if(t == multijet_num - 1) { //only want to fill histogram on the last jet of the multijet
                  auto pt_ref  = Monitored::Scalar<int>("pt_"+ r, jet_pt_loop);
                  auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
                  fill(m_packageName, pt_ref, passed_pt);
                  if (trig_of_interest_decision) {
                    auto passed_pt_val = Monitored::Scalar<int>("pt:" + r +  "_" + trigger_name, jet_pt_loop);
                    fill(m_packageName, passed_pt_val);
                  }  //(close IF) passed_pt if loop
                } //(close IF) loop that checks we are filling only the last jet of the multijet
                ++jet_itr_loop;
              } //(close FOR) loop that iterates trhrough each jet required by the multijet trigger
            } //(close IF) loop that checks if the number of jets preseent agrees with the multijet trigger
          }//(close IF) loop that checks determines if our trigger is a multijet trigger


          //filling histograms that are effiency curves as a funciton of eta
          //in order to ensure that we are isolating only the eta behavior, we have a flatline value where the pt effiencies flatten out to 1
          //these are hard coded per trigger
          if (l1_trigger_flatline_vals.find(trigger_name) != l1_trigger_flatline_vals.end()) {  //check to see if the trigger is in the list of triggers we have flatline values for
            if(jet_pt["offline"]>l1_trigger_flatline_vals[trigger_name]) { //is jet pt greater than the flatline value
              auto eta_ref  = Monitored::Scalar<int>("eta_" + r, jet_eta["offline"]);
              auto passed_eta = Monitored::Scalar<bool>("eta_" + r + "_" + trigger_name, trig_of_interest_decision);
              fill(m_packageName, eta_ref, passed_eta);
              if (trig_of_interest_decision) { //if the trigger passes, we can also add the eta value to a stand alone histogram
                auto passed_eta_val  = Monitored::Scalar<int>("eta:" + r +"_" + trigger_name, jet_eta["offline"]);
                fill(m_packageName, passed_eta_val);
              } //(close IF) passed eta if loop
            } //(close IF) jet pt is greater than pt flatline vlaue loop
          } //(close IF) loop that checks if the trigger of interest is int eh

        
          // if our trigger is NOT a multijet trigger, we can simply fill it without iterating through jets!
          if (! multijet_trigger) { // if to determine if our trigger is a multijet trigger
            auto pt_ref  = Monitored::Scalar<int>("pt_"+ r, jet_pt["offline"]);
            auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
            fill(m_packageName, pt_ref, passed_pt);
            if (trig_of_interest_decision) { //if loop to see if trigger of interest passed. If yes we want to fill a histogram with the value
              auto passed_pt_val = Monitored::Scalar<int>("pt:" + r +  "_" + trigger_name, jet_pt["offline"]);
              fill(m_packageName, passed_pt_val);
            } //(close IF) trig_of_interest_decision loop
          } //(close IF) not multijet trigger jet loop

        } //(close FOR) loop that iterates through all of the triggers we make effiency curves for
      } //(close IF) loop that checks if the refernce trigger decision is passed
    } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  } //(close IF) loop that checks if the physical properties were passed for the jet
 





  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////// FILL EFFIENCY HISTOGRAM OF ONLY LEADING OFFLINE JET FOR GFEX JET TRIGGERS /////////////////
  ////////////////////////////////// LOOKS AT BOTH SR AND LR JETS /////////////////////////// /////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  for (auto & g : gFex_types){ //iterate through SR and LR gfex jets.
    if (delta_jets_cuts_passed[g]){ //check that the physical cuts are satisfied before flling anything
      //iterate through orthogoanl trigger as reference trigger, and no trigger for reference trigger
      std::vector<std::string> ref_trig {"ortho", "none", "unbiased"};
      for (auto & r : ref_trig){ //iterate through our different kinds of refernce triggers
        if (ref_trigger_decision[r]){ //if the reference trigger decision is true 
          for(unsigned int t=0; t< gFEX_triggers[g].size(); ++t){ // iterate through the triggers we want to fill histograms for
            std::string trigger_name = gFEX_triggers[g][t]; // define the trigger name from the triggers that were extracted
    
            bool trig_of_interest_decision = false;
            if (use_emulated_gfex_trig && jet_pt[g] > gFEX_triggers_val[g][trigger_name]) {  //check if the emulated trigger would pass, if we want to use it
              trig_of_interest_decision = true; 
            } else if (! use_emulated_gfex_trig) {  //if we dont want to use emulated trigger, check if the trigger passses
              trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name);
            } //(close IF) loops that define the triggers decision for emulated and non emulated

            auto pt_ref  = Monitored::Scalar<int>("pt_"+ r, jet_pt["offline"]);
            auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision); 
            fill(m_packageName, pt_ref, passed_pt); 
            
            if (trig_of_interest_decision) { //only fill the histogram of pt if the trigger decision is passed 
              auto passed_pt_val = Monitored::Scalar<int>("pt:" + r +  "_" + trigger_name, jet_pt["offline"]);
              fill(m_packageName, passed_pt_val);  
            } //(close IF) loop that determines if we are filling the histogram 
          } //(close FOR) loop that iterates through triggers
        } //(close IF) refernce trigger decision is passed loop
      } //(close FOR) loop that iterates over reference triggers
    } //(close IF) loop if physical cuts passed 
  } //(close FOR) loop that iterates over SR and LR jet tobs




  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////// FILL EFFIENCY HISTOGRAMS FOR THE gFEX Triggers on gFex Jets /////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  for (auto & g : gFex_types){ //iterate through gfex trigger types (SR and LR)
    //define variables according to which gfex trigger type
    if (physical_cuts_passed_type[g]){ //check that the physical cuts are satisfied before flling anything
      for (auto & r : ref_trig){  //iterate through orthogoanl trigger as reference trigger, and no trigger for reference trigger
        if (ref_trigger_decision[r]){//if the reference trigger decision is true 
          for(unsigned int t=0; t< gFEX_triggers[g].size(); ++t){ // iterate through the triggers we want to fill histograms for
            std::string trigger_name = gFEX_triggers[g][t]; // define the trigger name from the triggers that were extracted
            bool trig_of_interest_decision = false;
            if (use_emulated_gfex_trig && jet_pt[g] > gFEX_triggers_val[g][trigger_name]) { //check if the emulated trigger would pass, if we want to use it
              trig_of_interest_decision = true;
            } else if (! use_emulated_gfex_trig) { //if we dont want to use emulated trigger, check if the trigger passses 
              trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); 
            } //(close IF) loops that define the triggers decision for emulated and non emulated

            auto pt_orthogonal_gfex_val  = Monitored::Scalar<int>("pt_gfex_" + g + "_" + r , jet_pt[g]);
            auto passed_pt_orth_gfex  = Monitored::Scalar<bool>("pt_" + r + "_" + g + "_" + trigger_name, trig_of_interest_decision);

            if (trig_of_interest_decision){ //only fill the histogram of pt if the trigger decision is passed 
              auto passed_pt_orth_gfex_val  = Monitored::Scalar<int>("pt:"+ r + "_" +g+"_" + trigger_name, jet_pt[g]);
              fill(m_packageName, passed_pt_orth_gfex_val); 
            } //(close IF) loop that determines if we are filling the histogram 
            fill(m_packageName, pt_orthogonal_gfex_val,  passed_pt_orth_gfex);  
          } //close for loop that iterates through triggers
        } //(close IF) refernce trigger decision is passed loop
      } //(close FOR) loop that iterates over reference triggers
    } //(close IF)loop if physical cuts passed 
  } //(close IF) loop that iterates over SR and LR jet tobs

  variables.clear();
  return StatusCode::SUCCESS;
}

