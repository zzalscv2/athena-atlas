/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetEfficiencyMonitorAlgorithm.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "FourMomUtils/P4Helpers.h"

#include <cstdlib>

using Athena::Units::GeV;

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
  ATH_CHECK(m_gFexLRJetContainerKey.initialize()); //initizlize gfex lr jets
  ATH_CHECK(m_gFexSRJetContainerKey.initialize()); //initizlize gfex sr jets

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
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //DEFINITIONS and extracting variables from the python config file!
  bool use_emulated_gfex_trig =  m_emulated;
  bool use_passed_before_prescale =  m_passedb4Prescale;
  std::string  bootstrap_trigger = m_bootstrap_trigger;
  std::string orthogonal_trigger = m_orthogonal_trigger; //use the same orthogonal trigger for all kinds of jets!
  std::vector<std::string> unbiased_triggers = {"L1_MU14FCH", "L1_MU18VFCH",
    "L1_MU8F_TAU20IM", "L1_2MU8F", "L1_MU8VF_2MU5VF", "L1_3MU3VF",
    "L1_MU5VF_3MU3VF", "L1_4MU3V", "L1_2MU5VF_3MU3V",
    "L1_RD0_FILLED"};
  std::vector<std::string> gFex_types {"SR", "LR"};
  
  
  //Define the various reference vector things!
  std::vector<std::string> ref_trig {"bs", "ortho", "none", "unbiased"};
  
  bool bs_decision = false;
  bool ortho_decision = false;
  bool unbiased_trig_decision = false;
  
  // if use pass before prescale, then we have to use this fun
  // is passed bits, l1 passed before prescale feature for extracting if the trigger passed
  if (use_passed_before_prescale) {
    const unsigned int bs_bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(bootstrap_trigger);
    bs_decision = bs_bits & TrigDefs::L1_isPassedBeforePrescale;
    
    const unsigned int ortho_bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(orthogonal_trigger);
    ortho_decision = ortho_bits & TrigDefs::L1_isPassedBeforePrescale;
    
    for (auto & u : unbiased_triggers) {
      const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(u);
      bool pass = bits & TrigDefs::L1_isPassedBeforePrescale;
      if (pass) {unbiased_trig_decision = true;}
    } // iterating through the list of unbaised triggers
  } //close if used pass before prescale
  // if not using pass before prescale, then is a more chill process to see if trigger passed
  else {
    bs_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(bootstrap_trigger);
    ortho_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(orthogonal_trigger);
    for (auto & u : unbiased_triggers) {
      if (AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(u)) {unbiased_trig_decision = true;}
    } //close iterating through the unbiased triggers
  } //close else
  
  std::map<std::string, bool> ref_trigger_decision {
    {"bs", bs_decision },
    {"ortho", ortho_decision},
    {"none", true},
    {"unbiased", unbiased_trig_decision}
  };
  
  
  //fill a simple histogram of just the run number
  auto run = Monitored::Scalar<int>("run",GetEventInfo(ctx)->runNumber());
  fill(m_packageName, run); //fill the run number histogram that was default
  
  constexpr int minPt = 10*GeV;
  constexpr float maxEta = 2.8;
  
  std::map<std::string, SG::ReadHandle<xAOD::gFexJetRoIContainer>> gFEX_Container {
    {"SR",gFexSRJetContainer}, {"LR", gFexLRJetContainer}
  };
  
  //fill maps that allow us to keep track of variables according to the different containers
  double offline_pt = 0, LR_pt = 0, gfex_SR_pt = 0, gfex_LR_pt=0;
  std::map<std::string, double> jet_pt {
    {"offline", offline_pt}, {"offline_LR", LR_pt}, {"SR", gfex_SR_pt}, {"LR", gfex_LR_pt}
  };
  
  double offline_eta = 0, LR_eta = 0, gfex_SR_eta = 0, gfex_LR_eta=0;
  std::map<std::string, double> jet_eta {
    {"offline", offline_eta}, {"offline_LR", LR_eta},{"SR", gfex_SR_eta}, {"LR", gfex_LR_eta}
  };
  
  double offline_phi = 0, LR_phi = 0, gfex_SR_phi = 0, gfex_LR_phi=0;
  std::map<std::string, double> jet_phi {
    {"offline", offline_phi}, {"offline_LR", LR_phi}, {"SR", gfex_SR_phi}, {"LR", gfex_LR_phi}
  };
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Fill pt, eta and phi vals for all the containers!
  //offline jet containers
  if (!jets->empty()) { //check that there are jets before accessing the container
    xAOD::JetContainer::const_iterator jet_itr = jets->begin();
    jet_pt["offline"] = (*jet_itr)->pt();
    jet_eta["offline"] = (*jet_itr)->eta();
    jet_phi["offline"] = (*jet_itr)->phi();
  } //(close IF) jets size > 0 loop
  //LR jet containers
  if (!LRjets->empty()) { //check that there are jets before accessing the container
    xAOD::JetContainer::const_iterator LRjets_itr = LRjets->begin();
    jet_pt["offline_LR"] = (*LRjets_itr)->pt();
    jet_eta["offline_LR"] = (*LRjets_itr)->eta();
    jet_phi["offline_LR"] = (*LRjets_itr)->phi();
  } //(close IF) LRjets size > 0 loop
  
  //gFex SR and LR TOB containers
  // gfex tobs are not pt sorted, so if we want to emulate the triggers we need to find the largest first
  for (auto & g : gFex_types){ //iterate through SR and LR gfex jets.
    if (!gFEX_Container[g]->empty()) { //check that there are jets before accessing the container
      // gfex tobs are not sorted according to pt, so we need to look for the leading tobs manually
      double max_pt = 0.0; //inital value of zero
      const xAOD::gFexJetRoI* lead_gfex_jet = nullptr; //inital pointer of null
      //iterate through all the gfex tobs
      for (const auto* gfex_jet : *gFEX_Container[g]) {
        //extract gfex tob values
        const double check_jet_pt = gfex_jet->et(), check_jet_eta = gfex_jet->eta();
        //check if this gfex tob is more PT than the current max
        if (check_jet_pt > max_pt && std::abs(check_jet_eta) < maxEta){
          max_pt = check_jet_pt; //update maximums!
          lead_gfex_jet = gfex_jet;
        } //(close IF) loop if gfex jet satisfies pt and eta conditions
      } //(close FOR) loop that iterates through gfex tobs
      //if we successfully found a leading gfex tob, then we can save its physics properties
      if (lead_gfex_jet != nullptr) {
        jet_eta[g] = lead_gfex_jet->eta(), jet_phi[g] = lead_gfex_jet->phi(), jet_pt[g] = lead_gfex_jet->et();
      } // (close IF) loop that checks if there is a leading gfex jet
    } // (close IF) loop that checks that there are gfex tobs
  }// (close FOR) that loops through sr and lr gfex tobs
  
  
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //DEFINE PHYSICAL CUTS
  //offline SR
  bool physical_cuts_passed = false;
  if(std::abs(jet_eta["offline"])<maxEta && (jet_pt["offline"] > minPt)) { physical_cuts_passed = true; }
  
  // offline LR
  bool LR_physical_cuts_passed = false;
  if(std::abs(jet_eta["offline_LR"])<maxEta && (jet_pt["offline_LR"] > minPt)) { LR_physical_cuts_passed = true; }
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //PREP LISTS OF TRIGGERS
  
  
  //Create and clean up the list of L1Triggers
  std::vector<std::string> all_multi_triggers = m_multi_jet_TriggerList;
  std::vector<std::string> all_triggers_for_SR = m_all_triggers_for_SR;
  std::vector<std::string> all_triggers_for_LR = m_all_triggers_for_LR;
  
  std::map<std::string, int> l1_trigger_flatline_vals {
    //this is around where the l1trigger pt effiencies flatten out
    //in order to make eta effiency curves, its useful to isolate the pt behavior,
    // so we want 100% pt effiency, or after 'flattening out'
    {"L1_J15", 35*GeV}, {"L1_J20", 40*GeV},  {"L1_J30", 60*GeV},
    {"L1_J40", 80*GeV}, {"L1_J50", 90*GeV}, {"L1_J75", 120*GeV},
    {"L1_J100", 140*GeV}
  };
  
  /////////////////////////////////////////////////////////////
  std::map<std::string, double> gFexTriggers_val {
    //trigger val corresponding to the gfex triggers
    {"L1_gJ20", 20*GeV}, {"L1_gJ30", 30*GeV},  {"L1_gJ40", 40*GeV},
    {"L1_gJ50", 50*GeV}, {"L1_gJ60", 60*GeV}, {"L1_gJ100", 100*GeV},
    {"L1_gJ160", 160*GeV}, {"L1_gLJ80", 80*GeV}, {"L1_gLJ100", 100*GeV},
    {"L1_gLJ140", 140*GeV},  {"L1_gLJ160", 160*GeV}
  };
  
  std::map<std::string, std::vector<double>> multijet_triggers_val {
    {"L1_3J50", {50*GeV, 50*GeV, 50*GeV}}, {"L1_4J15", {15*GeV, 15*GeV, 15*GeV, 15*GeV}},
    {"L1_4J20", {20*GeV, 20*GeV, 20*GeV, 20*GeV}}, {"L1_J85_3J30", {85*GeV, 30*GeV, 30*GeV, 30*GeV}},
    {"L1_2J15_XE55", {15*GeV, 15*GeV}}, {"L1_2J50_XE40", {50*GeV, 50*GeV}}
  };
  
  //define emulatedString variable so that we can have emulated in the title
  //(or not) according to the status of the gfex triggers
  std::string emulatedString = " ";
  if (use_emulated_gfex_trig) {emulatedString = " Emulated "; }
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Fill some useful sample histograms
  auto raw_pt  = Monitored::Scalar<int>("raw_pt", 0.);
  raw_pt = jet_pt["offline"];
  fill(m_packageName, raw_pt);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // FILL EFFIENCY HISTOGRAMS INVOLVING SR OFFLINE JETS
  
  for (auto & r : ref_trig){ //iterate through the refernce triggers
    if (physical_cuts_passed && ref_trigger_decision[r]) { //check that the physical cuts and reference trigger is passed
      for(unsigned int t=0; t< all_triggers_for_SR.size(); ++t) {//iterate through all of the useful triggers list (that we make effiency curves for)
        const std::string& trigger_name = all_triggers_for_SR[t]; // define the trigger name and get its decision
        
        bool trig_of_interest_decision = false;
        
        //check if we are looking at a gfex trigger and if we want to emulate the gfex trigger
        if ((gFexTriggers_val.find(trigger_name) != gFexTriggers_val.end()) && use_emulated_gfex_trig) {
          if (jet_pt["SR"] > gFexTriggers_val[trigger_name]) { //the qualifier for emulating trigger passing
            trig_of_interest_decision = true;
          }
        } else if (use_passed_before_prescale) {
          //We can choose if we want to use pass before prescale, or not when defining our trigger efficiency
          //this boolean is defiend in the jeteffmonalg.py file
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        //get values and fill the histogram of offline jet pt and boolean of trigger passing
        auto pt_ref  = Monitored::Scalar<int>("pt_"+ r, jet_pt["offline"]);
        auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt);
        
        //if loop to see if trigger of interest passed. If yes we want to fill a histogram with the pt value
        if (trig_of_interest_decision) {
          auto passed_pt_val = Monitored::Scalar<int>("pt:" + r +  "_" + trigger_name, jet_pt["offline"]);
          fill(m_packageName, passed_pt_val);
        } //(close IF) trig_of_interest_decision loop
        
        //filling histograms that are effiency curves as a funciton of eta
        //in order to ensure that we are isolating only the eta behavior, we have a
        // flatline value where the pt effiencies aproximtley flatten out to 1
        //these are hard coded, and saved for only a few of the triggers!
        if (l1_trigger_flatline_vals.find(trigger_name) != l1_trigger_flatline_vals.end()) {
          if(jet_pt["offline"]>l1_trigger_flatline_vals[trigger_name]) { //is jet pt greater than the flatline value?
            //get value of eta, and histogram passing boolean and fill
            auto eta_ref  = Monitored::Scalar<int>("eta_" + r, jet_eta["offline"]);
            auto passed_eta = Monitored::Scalar<bool>("eta_" + r + "_" + trigger_name, trig_of_interest_decision);
            fill(m_packageName, eta_ref, passed_eta);
            //if the trigger passes, we can also add the eta value to a stand alone histogram
            if (trig_of_interest_decision) {
              auto passed_eta_val  = Monitored::Scalar<int>("eta:" + r +"_" + trigger_name, jet_eta["offline"]);
              fill(m_packageName, passed_eta_val);
            } //(close IF) passed eta if loop
          } //(close IF) jet pt is greater than pt flatline vlaue loop
        } //(close IF) loop that checks if the trigger of interest is int eh
      } //(close FOR) loop that iterates through all of L1 single jet triggers we make effiency curves for
      
      
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      //now lets iterate through the multi jet triggers to make efficiency curves!
      for(unsigned int t=0; t< all_multi_triggers.size(); ++t) {
        const std::string& trigger_name = all_multi_triggers[t];
        
        //determine if the trigger passed
        bool trig_of_interest_decision = false;
        if (use_passed_before_prescale) {
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        // we only want to fill the "last" jet of the trigger qualification
        // example:
        ///// a 4J100 jet trigger?
        ///// we want to only plot the pt of the 4th leading jet
        int multijet_num = multijet_triggers_val[trigger_name].size(); // number of jets we expect
        int jets_num = jets->size(); //number of jets in the event
        if (jets_num >= multijet_num) {
          //iterate through each jet required by trigger and check that the value is satisfied
          int jet_count = 0;
          for (const auto* j : *jets) {
            jet_count += 1;
            if(jet_count == multijet_num) { //only want to fill histogram on the last jet of the multijet
              const double jet_pt_loop = j->pt();
              auto pt_ref  = Monitored::Scalar<int>("pt_"+ r, jet_pt_loop);
              auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
              fill(m_packageName, pt_ref, passed_pt);
              if (trig_of_interest_decision) {
                auto passed_pt_val = Monitored::Scalar<int>("pt:" + r +  "_" + trigger_name, jet_pt_loop);
                fill(m_packageName, passed_pt_val);
              }  //(close IF) passed_pt if loop
            } //(close IF) loop that checks we are filling only the last jet of the multijet
          } //(close FOR) loop that iterates trhrough each jet required by the multijet trigger
        } //(close IF) loop that checks if the number of jets preseent agrees with the multijet trigger
      } //(close FOR) loop that iterates through the multijet triggers
      
    } //(close IF) loop that checks if the reference trigger and physical property pass is passed
  } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //FILL EFFIENCY HISTOGRAMS INVOLVING LR OFFLINE JETS
  
  for (auto & r : ref_trig){ //iterate through the refernce triggers
    if (LR_physical_cuts_passed && ref_trigger_decision[r]) { //check that the physical cuts and reference trigger is passed
      for(unsigned int t=0; t< all_triggers_for_LR.size(); ++t){ //iterate through all of the useful LR triggers list (that we make effiency curves for)
        const std::string& trigger_name = all_triggers_for_LR[t];
        
        bool trig_of_interest_decision = false;
        
        if ((gFexTriggers_val.find(trigger_name) != gFexTriggers_val.end()) && use_emulated_gfex_trig) {
          if (jet_pt["LR"] > gFexTriggers_val[trigger_name]) { //the qualifier for emulating trigger passing
            trig_of_interest_decision = true;
          }
        } else if (use_passed_before_prescale) {
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        auto pt_ref  = Monitored::Scalar<int>("pt_"+ r, jet_pt["offline_LR"]);
        auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt);
        
        if (trig_of_interest_decision) { //if loop to see if trigger of interest passed. If yes we want to fill a histogram with the value
          auto passed_pt_val = Monitored::Scalar<int>("pt:" + r +  "_" + trigger_name, jet_pt["offline_LR"]);
          fill(m_packageName, passed_pt_val);
        } //(close IF) trig_of_interest_decision loop
        
      } //(close FOR) loop that iterates through all of the triggers we make effiency curves for
    } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  } //(close IF) loop that checks if the physical properties were passed for the jet
  
  variables.clear();
  return StatusCode::SUCCESS;
}
