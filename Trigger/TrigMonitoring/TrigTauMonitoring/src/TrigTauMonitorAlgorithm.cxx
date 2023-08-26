/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "StoreGate/ReadDecorHandle.h"
#include "LArRecEvent/LArEventBitInfo.h"

#include "TrigTauMonitorAlgorithm.h"

TrigTauMonitorAlgorithm::TrigTauMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{}


TrigTauMonitorAlgorithm::~TrigTauMonitorAlgorithm() {}


StatusCode TrigTauMonitorAlgorithm::initialize() {

  ATH_CHECK( AthMonitorAlgorithm::initialize() );
  ATH_CHECK( m_offlineTauJetKey.initialize() );
  ATH_CHECK( m_offlineElectronKey.initialize() );
  ATH_CHECK( m_offlineMuonKey.initialize() );
  ATH_CHECK( m_legacyl1TauRoIKey.initialize() );
  ATH_CHECK( m_phase1l1eTauRoIKey.initialize() );
  ATH_CHECK( m_phase1l1jTauRoIKey.initialize() );
  ATH_CHECK( m_phase1l1cTauRoIKey.initialize() );
  ATH_CHECK( m_hltTauJetKey.initialize() );
  ATH_CHECK( m_hltTauJetCaloMVAOnlyKey.initialize() );
  ATH_CHECK( m_trigDecTool.retrieve() );
  ATH_CHECK( m_truthParticleKey.initialize(m_isMC) );
  ATH_CHECK( m_eventInfoDecorKey.initialize() );

  for(const auto& trigName:m_trigInputList)
  {
     if(getTrigInfoMap().count(trigName) != 0){
       ATH_MSG_WARNING("Trigger already booked, removing from trigger list " << trigName);
     }else {
       m_trigList.push_back(trigName);
       setTrigInfo(trigName);
     }
  }

  return StatusCode::SUCCESS;
}


StatusCode TrigTauMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  //using namespace Monitored;

  ATH_MSG_DEBUG("Executing TrigTauMonitorAlgorithm");

  if(m_trigDecTool->ExperimentalAndExpertMethods().isHLTTruncated()){
    ATH_MSG_WARNING("HLTResult truncated, skip trigger analysis");
    return StatusCode::SUCCESS; 
  }
 

  // Protect against noise bursts
  SG::ReadHandle<xAOD::EventInfo> currEvent(GetEventInfo(ctx));
  ATH_CHECK(currEvent.isValid());
  if ( currEvent->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO)) 
    return StatusCode::SUCCESS;

 
  ATH_MSG_DEBUG("Chains for Analysis " << m_trigList);

  std::vector< std::pair<const xAOD::TauJet*, const TrigCompositeUtils::Decision*>> pairObjs;
 
  std::vector<std::string> L1seed_list;
  bool fill_l1 = false;

  for(const auto& trigger : m_trigList){

    const TrigInfo info = getTrigInfo(trigger);

    if ( executeNavigation( ctx, info.trigName,pairObjs).isFailure() || pairObjs.empty())                
    {            
       ATH_MSG_DEBUG("executeNavigation failed");                                                                                                       
       return StatusCode::SUCCESS;                                                                                                                         
    }  

    if(info.trigL1Item.empty()){ // L1 item not found
       fill_l1 = false;
    }
    else if (std::find(L1seed_list.begin(), L1seed_list.end(), info.trigL1Item) == L1seed_list.end())
    {
        // check if L1 seed has been already filled -> L1 seed must be filled only once for triggers with same L1 seed
        fill_l1 = true;
        L1seed_list.push_back(info.trigL1Item);
        
    } else {
        fill_l1 = false;
    }
 
    fillDistributions( ctx, pairObjs, trigger, info.trigL1Item, info.HLTthr, info.L1thr); 
    if(fill_l1){
      fillL1Distributions( ctx, pairObjs, trigger ,info.trigL1Item, info.L1thr);  
    }   
    pairObjs.clear();
                                                            
  }
   
  L1seed_list.clear();
  
  return StatusCode::SUCCESS;
}


StatusCode TrigTauMonitorAlgorithm::executeNavigation( const EventContext& ctx, 
                                                       const std::string& trigItem,
                                                       std::vector<std::pair<const xAOD::TauJet*, const TrigCompositeUtils::Decision* >> &pairObjs) const
{

  ATH_MSG_DEBUG("Apply navigation selection "); 

  SG::ReadHandle<xAOD::TauJetContainer> offTaus(m_offlineTauJetKey, ctx);

  if(!offTaus.isValid())
  {
     ATH_MSG_WARNING("Failed to retrieve offline Taus ");
     return StatusCode::FAILURE;
  }

  std::string tauContainerName = "HLT_TrigTauRecMerged_MVA";
  if(trigItem.find("LLP_") != std::string::npos){
     tauContainerName="HLT_TrigTauRecMerged_LLP";
  } else if(trigItem.find("LRT_") != std::string::npos){
     tauContainerName="HLT_TrigTauRecMerged_LRT";
  } else if(trigItem.find("ptonly") != std::string::npos) 
     tauContainerName="HLT_TrigTauRecMerged_CaloMVAOnly";

  auto vec =  m_trigDecTool->features<xAOD::TauJetContainer>(trigItem,TrigDefs::Physics , tauContainerName );

  for(const auto *const Tau : *offTaus ){

    const TrigCompositeUtils::Decision *dec=nullptr; 

    // consider only offline taus outside of the crack region
    if( std::abs(Tau->eta()) > 1.37 &&  std::abs(Tau->eta()) < 1.52) continue;

    // consider only offline taus which pass RNN medium WP 
    if( !Tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium)) continue;

    // consider only offline taus which pass thinning 
    if( Tau->isAvailable<char>("passThinning") && !Tau->auxdata<char>("passThinning") ) continue;

    for( auto &featLinkInfo : vec ){                                             
      if(! featLinkInfo.isValid() ) continue;
      const auto *feat = *(featLinkInfo.link);                   
      if(!feat) continue;
      float deltaR = dR(Tau->eta(),Tau->phi(), feat->eta(),feat->phi());                                
      if(deltaR < 0.4){            
         dec = featLinkInfo.source;                                                
      }                     
    }

    std::pair< const xAOD::TauJet*, const TrigCompositeUtils::Decision * > pair(Tau,dec);
    pairObjs.push_back(pair);

  }

  ATH_MSG_DEBUG("BaseToolMT::Tau TEs " << pairObjs.size() << " found.");
  return StatusCode::SUCCESS;
}

void TrigTauMonitorAlgorithm::fillDistributions(const EventContext& ctx, const std::vector< std::pair< const xAOD::TauJet*, const TrigCompositeUtils::Decision * >>& pairObjs, const std::string& trigger, const std::string& trigL1Item, float HLTthr, float L1thr) const
{
  ATH_MSG_DEBUG ("TrigTauMonitorAlgorithm::fillDistributions for trigger " << trigger);

  const double thresholdOffset{10.0};

  std::vector<const xAOD::TauJet*> offline_for_hlt_tau_vec_all; // offline taus used for studying HLT performance
  std::vector<const xAOD::TauJet*> offline_for_hlt_tau_vec_1p; // offline 1p taus used for studying HLT performance
  std::vector<const xAOD::TauJet*> offline_for_hlt_tau_vec_3p; // offline mp taus used for studying HLT performance
  std::vector<const xAOD::TauJet*> online_tau_vec_0p; // online 0p taus used for studying HLT performance
  std::vector<const xAOD::TauJet*> online_tau_vec_1p; // online 1p taus used for studying HLT performance
  std::vector<const xAOD::TauJet*> online_tau_vec_mp; // online mp taus used for studying HLT performance
  std::vector<const xAOD::TauJet*> online_tau_vec_all; // online hlt taus used for HLT efficiency studies
  std::vector<TLorentzVector> online_tau_vec;   // online hlt taus used for HLT efficiency studies (TLorentzVector)
  std::vector<TLorentzVector> online_electrons; // online hlt electrons used for HLT efficiency studies in T&P chains
  std::vector<TLorentzVector> online_muons; // online hlt electrons used for HLT efficiency studies in T&P \chains
  std::vector<TLorentzVector> offElec_vec;  //offline electrons used for studying HLT performance 
  std::vector<TLorentzVector> offMuon_vec;  //offline muons used for studying HLT performance 

  const TrigInfo info = getTrigInfo(trigger);

  // Offline
  std::vector<const xAOD::TauJet*> tau_vec;
  for( auto pairObj: pairObjs )
  {
    int nTracks=-1;
    pairObj.first->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);
    ATH_MSG_DEBUG("NTracks Offline: " << nTracks);
    // filling vectors for studying HLT performance
    if(nTracks==1 && ( pairObj.first->pt() > (HLTthr-thresholdOffset)*1.e3)){
       offline_for_hlt_tau_vec_1p.push_back(pairObj.first);
       offline_for_hlt_tau_vec_all.push_back(pairObj.first);
    }else if(nTracks==3 && ( pairObj.first->pt() > (HLTthr-thresholdOffset)*1.e3)){
       offline_for_hlt_tau_vec_3p.push_back(pairObj.first);
       offline_for_hlt_tau_vec_all.push_back(pairObj.first);
    }
  }

  // Online
  std::string tauContainerName = "HLT_TrigTauRecMerged_MVA";
  if(trigger.find("LLP_") != std::string::npos){
     tauContainerName="HLT_TrigTauRecMerged_LLP";
  } else if (trigger.find("LRT_") != std::string::npos){
     tauContainerName="HLT_TrigTauRecMerged_LRT";
  } else if(trigger.find("ptonly") != std::string::npos) 
     tauContainerName="HLT_TrigTauRecMerged_CaloMVAOnly";

  ATH_MSG_DEBUG("Tau ContainerName is: " << tauContainerName);

  auto vec =  m_trigDecTool->features<xAOD::TauJetContainer>(trigger,TrigDefs::Physics , tauContainerName );
  for( auto &featLinkInfo : vec ){

    const auto *feat = *(featLinkInfo.link);
    if(!feat) continue;
    // If not pass, continue
    int nTracks=-1;
    feat->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);

    ATH_MSG_DEBUG("NTracks Online: " << nTracks);
    online_tau_vec_all.push_back(feat);online_tau_vec.push_back(feat->p4());
    if(nTracks==0){
      online_tau_vec_0p.push_back(feat);
    } else if(nTracks==1){
      online_tau_vec_1p.push_back(feat);
    }else{
      online_tau_vec_mp.push_back(feat);
    }
  }

  if(info.isDiTau){
     // fill ditau information 
     fillDiTauVars(trigger, online_tau_vec_all);

     auto passBits=m_trigDecTool->isPassedBits(trigger);
     if((passBits & TrigDefs::L1_isPassedAfterVeto)  && ((passBits & TrigDefs::EF_prescaled)==0)){
        fillDiTauHLTEfficiencies(ctx, trigger, offline_for_hlt_tau_vec_all, online_tau_vec_all);
     }
  } else if (info.isTAndP){
    // fill T&P chains info                                                                                             
    if(info.hasElectron){
      SG::ReadHandle<xAOD::ElectronContainer> offElec(m_offlineElectronKey, ctx);
      if(!offElec.isValid())
      {
        ATH_MSG_WARNING("Failed to retrieve offline electrons ");
        return;
      }
      for ( const auto * const part : *offElec) 
      {
         if(part->p4().Pt()/1000 < info.electhr+1.) continue; 
         // select offline electrons passing good quality cuts
         if( ! ( part->passSelection("LHMedium") && part->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON))) continue;

         offElec_vec.push_back(part->p4());
      }
      for ( unsigned int i=0;i<offline_for_hlt_tau_vec_all.size();i++) {
         bool Ismatch = false; 
         Ismatch = HLTMatching(offline_for_hlt_tau_vec_all[i]->p4(),offElec_vec,0.2);
         if(Ismatch) offline_for_hlt_tau_vec_all.erase(offline_for_hlt_tau_vec_all.begin()+i);
      } 
      auto vec =  m_trigDecTool->features<xAOD::ElectronContainer>(trigger,TrigDefs::Physics , "HLT_egamma_Electrons_GSF" );
      for( auto &featLinkInfo : vec ){
        const auto *feat = *(featLinkInfo.link);
        if(!feat) continue;
        online_electrons.push_back(feat->p4());
      }

      fillTagAndProbeVars(trigger, online_tau_vec, online_electrons);

      auto passBits=m_trigDecTool->isPassedBits(trigger);
      if((passBits & TrigDefs::L1_isPassedAfterVeto)  && ((passBits & TrigDefs::EF_prescaled)==0)){
         fillTAndPHLTEfficiencies(ctx, trigger, offElec_vec, online_electrons, offline_for_hlt_tau_vec_all, online_tau_vec_all);
      }

    } else if(info.hasMuon){
      SG::ReadHandle<xAOD::MuonContainer> offMuon(m_offlineMuonKey, ctx);
      if(!offMuon.isValid())
      {
        ATH_MSG_WARNING("Failed to retrieve offline muons ");
        return;
      }
      for( const auto * const part : *offMuon) 
      {
         if(part->p4().Pt()/1000 < info.muthr+1.) continue;
         // select offline muons passing good quality cuts
         if( ! (part->quality()<= xAOD::Muon::Medium && part->passesIDCuts())) continue;

         offMuon_vec.push_back(part->p4());
      }
      for ( unsigned int i=0;i<offline_for_hlt_tau_vec_all.size();i++) {
         bool Ismatch = false;
         Ismatch = HLTMatching(offline_for_hlt_tau_vec_all[i]->p4(),offMuon_vec,0.2);
         if(Ismatch) offline_for_hlt_tau_vec_all.erase(offline_for_hlt_tau_vec_all.begin()+i);
      }

      auto vec =  m_trigDecTool->features<xAOD::MuonContainer>(trigger,TrigDefs::Physics , "HLT_MuonsIso" );
      for( auto &featLinkInfo : vec ){
        const auto *feat = *(featLinkInfo.link);
        if(!feat) continue;
        online_muons.push_back(feat->p4());
      }

      fillTagAndProbeVars(trigger, online_tau_vec, online_muons);

      auto passBits=m_trigDecTool->isPassedBits(trigger);
      if((passBits & TrigDefs::L1_isPassedAfterVeto)  && ((passBits & TrigDefs::EF_prescaled)==0)){
         fillTAndPHLTEfficiencies(ctx, trigger, offMuon_vec, online_muons, offline_for_hlt_tau_vec_all, online_tau_vec_all); 
      }
    }
  } else { // single leg triggers

    // Offline
    if( !offline_for_hlt_tau_vec_1p.empty()){
      fillRNNInputVars( trigger, offline_for_hlt_tau_vec_1p,"1P", false );
      fillRNNTrack( trigger, offline_for_hlt_tau_vec_1p, false );
      fillRNNCluster( trigger, offline_for_hlt_tau_vec_1p, false );
      fillbasicVars( ctx, trigger, offline_for_hlt_tau_vec_1p, "1P", false);
    }

    if( !offline_for_hlt_tau_vec_3p.empty()){
      fillRNNInputVars( trigger, offline_for_hlt_tau_vec_3p,"3P", false );
      fillRNNTrack( trigger, offline_for_hlt_tau_vec_3p, false );
      fillRNNCluster( trigger, offline_for_hlt_tau_vec_3p, false );
      fillbasicVars( ctx, trigger, offline_for_hlt_tau_vec_3p, "3P", false);
    }

    // fill information for online 0 prong taus
    if(!online_tau_vec_0p.empty()){
      fillbasicVars( ctx, trigger, online_tau_vec_0p, "0P", true);
      fillRNNInputVars( trigger, online_tau_vec_0p,"0P", true );
      fillRNNTrack( trigger, online_tau_vec_0p, true );
      fillRNNCluster( trigger, online_tau_vec_0p, true );
    }

    // fill information for online 1 prong taus
    if(!online_tau_vec_1p.empty()){
      fillbasicVars( ctx, trigger, online_tau_vec_1p, "1P", true);
      fillRNNInputVars( trigger, online_tau_vec_1p,"1P", true );
      fillRNNTrack( trigger, online_tau_vec_1p, true );
      fillRNNCluster( trigger, online_tau_vec_1p, true );
    }

    // fill information for online multiprong prong taus 
    if(!online_tau_vec_mp.empty()){
      fillbasicVars( ctx, trigger, online_tau_vec_mp, "MP", true);
      fillRNNInputVars( trigger, online_tau_vec_mp,"MP", true );
      fillRNNTrack( trigger, online_tau_vec_mp, true );
      fillRNNCluster( trigger, online_tau_vec_mp, true );
    }

    auto passBits=m_trigDecTool->isPassedBits(trigger);
    if((passBits & TrigDefs::L1_isPassedAfterVeto)  && ((passBits & TrigDefs::EF_prescaled)==0)){
       fillHLTEfficiencies(ctx, trigger, trigL1Item, L1thr, offline_for_hlt_tau_vec_1p, online_tau_vec_all, "1P");
       fillHLTEfficiencies(ctx, trigger, trigL1Item, L1thr, offline_for_hlt_tau_vec_3p, online_tau_vec_all, "3P");
    }
  }

  // true_taus
  std::vector<const xAOD::TruthParticle*> true_taus_1p;
  std::vector<const xAOD::TruthParticle*> true_taus_3p;
  if(m_isMC){
    SG::ReadHandle<xAOD::TruthParticleContainer> truth_cont(m_truthParticleKey, ctx); 
    if(!truth_cont.isValid())
      {
          ATH_MSG_WARNING("Failed to retrieve truth_cont");
          return;
      }
    for(const auto xTruthParticle : *truth_cont)
    {
      if(xTruthParticle->isTau()){
        ATH_MSG_DEBUG("Tau with status " << xTruthParticle->status() << " and charge " << xTruthParticle->charge());
        xAOD::TruthParticle* xTruthTau = new xAOD::TruthParticle();
        xTruthTau->makePrivateStore( *xTruthParticle );

        if(examineTruthTau(*xTruthTau).isFailure()){
          delete xTruthTau;
          continue;
        }

        float pt  = xTruthTau->auxdata<double>("pt_vis");
        ATH_MSG_DEBUG("True Tau visible pt: " << pt);
        float eta = xTruthTau->auxdata<double>("eta_vis");
        bool lep = xTruthTau->auxdata<char>("IsLeptonicTau");
        if(pt < 20. || lep || std::abs(eta) > 2.47 ) {
          delete xTruthTau;
          continue;
        } 

        if(xTruthTau->auxdata<int>("nTracks") == 1){
          true_taus_1p.push_back(xTruthTau);
        } else if(xTruthTau->auxdata<int>("nTracks") == 3){
          true_taus_3p.push_back(xTruthTau);
        } else {
          delete xTruthTau;
        }
      }
    }
  }

  if(true_taus_1p.size()>0){
    fillTruthEfficiency(online_tau_vec_all, true_taus_1p, trigger, "1P");
    fillEFTauVsTruth(online_tau_vec_all, true_taus_1p, trigger, "1P");
  } 

  if(true_taus_3p.size()>0){
    fillTruthEfficiency(online_tau_vec_all, true_taus_3p, trigger, "3P");
    fillEFTauVsTruth(online_tau_vec_all, true_taus_3p, trigger, "3P");
  } 

  // remove all truth particles from the vector
  for(auto * true_tau_1p : true_taus_1p){
    delete true_tau_1p;
  }

  for(auto * true_tau_3p : true_taus_3p){
    delete true_tau_3p;
  }
  

  offline_for_hlt_tau_vec_all.clear();
  offline_for_hlt_tau_vec_1p.clear();
  offline_for_hlt_tau_vec_3p.clear();
  online_tau_vec_0p.clear();
  online_tau_vec_1p.clear();
  online_tau_vec_mp.clear();
  online_tau_vec_all.clear();
  true_taus_1p.clear();
  true_taus_3p.clear();
  online_tau_vec.clear();
  online_electrons.clear();
  online_muons.clear();
  offElec_vec.clear();
  offMuon_vec.clear();

}


std::vector<const xAOD::eFexTauRoI*> TrigTauMonitorAlgorithm::getL1items_efex(const EventContext& ctx, float L1thr) const
{
 
    std::vector<const xAOD::eFexTauRoI*> eFexphase1L1rois; //  used for studying legacy L1 performance

    SG::ReadHandle<xAOD::eFexTauRoIContainer> eFexTauRoIs(m_phase1l1eTauRoIKey, ctx);
    if(!eFexTauRoIs.isValid())
    {
       ATH_MSG_WARNING("Failed to retrieve eFexTauRoI for L1eTAU ");
       return eFexphase1L1rois;
    }
   
    for(const auto *eFexTauRoI : *eFexTauRoIs){
       if( eFexTauRoI->et()/1e3 > L1thr){
          eFexphase1L1rois.push_back(eFexTauRoI);
       }
    }   
    return eFexphase1L1rois;
}

std::vector<const xAOD::jFexTauRoI*> TrigTauMonitorAlgorithm::getL1items_jfex(const EventContext& ctx, float L1thr) const
{
    std::vector<const xAOD::jFexTauRoI*> jFexphase1L1rois; // used for studying phase1 L1 performance 

    SG::ReadHandle<xAOD::jFexTauRoIContainer> jFexTauRoIs(m_phase1l1jTauRoIKey, ctx);
    if(!jFexTauRoIs.isValid())
    {
       ATH_MSG_WARNING("Failed to retrieve jFexTauRoI for L1jTAU ");
       return jFexphase1L1rois;
    }

    for(const auto *jFexTauRoI : *jFexTauRoIs){
       if( jFexTauRoI->et()/1e3 > L1thr){
          jFexphase1L1rois.push_back(jFexTauRoI);
       }
    }
    return jFexphase1L1rois;
}

std::vector<const xAOD::EmTauRoI*> TrigTauMonitorAlgorithm::getL1items_legacy(const std::string& trigL1Item, const EventContext& ctx) const
{

    std::vector<const xAOD::EmTauRoI*> legacyL1rois; //  used for studying legacy L1 performance

    SG::ReadHandle<xAOD::EmTauRoIContainer> EmTauRoIs(m_legacyl1TauRoIKey, ctx);
    if(!EmTauRoIs.isValid())
    {
       ATH_MSG_WARNING("Failed to retrieve EmTauRoI ");
       return legacyL1rois;
    }

    for(const auto *EmTauRoI : *EmTauRoIs){
       // check which threshold has passed based on the current L1 item under monitoring
       // reference : https://gitlab.cern.ch/atlas/athena/-/blob/master/Trigger/TriggerCommon/TriggerMenuMT/python/L1/Config/ItemDef.py
       bool found_thrName= false;
       for(const auto& item : EmTauRoI->thrNames()){
          if( trigL1Item.find("L1TAU8") != std::string::npos && item.find("HA8") != std::string::npos)
          {
             found_thrName = true;
             break;
          }
          else if( trigL1Item.find("L1TAU12IM") != std::string::npos && item.find("HA12IM") != std::string::npos)
          {
             found_thrName = true;
             break;
          }
          else if( trigL1Item.find("L1TAU20IM") != std::string::npos && item.find("HA20IM") != std::string::npos)
          {
             found_thrName = true;
             break;
          }
          else if( trigL1Item.find("L1TAU40") != std::string::npos && item.find("HA40") != std::string::npos)
          {
             found_thrName = true;
             break;
          }
          else if ( trigL1Item.find("L1TAU60") != std::string::npos && item.find("HA60") != std::string::npos)
          {
             found_thrName = true;
             break;
          }
          else if ( trigL1Item.find("L1TAU100") != std::string::npos && item.find("HA100") != std::string::npos)
          {
             found_thrName = true;
             break;
          }
       }
       if(found_thrName) legacyL1rois.push_back(EmTauRoI);
    }
 
    return legacyL1rois;
}

void TrigTauMonitorAlgorithm::fillL1Distributions(const EventContext& ctx, const std::vector< std::pair< const xAOD::TauJet*, const TrigCompositeUtils::Decision * >>& pairObjs, const std::string& trigger,  const std::string& trigL1Item, float L1thr) const
{
    ATH_MSG_DEBUG ("TrigTauMonitorAlgorithm::fillL1Distributions");

    const double thresholdOffset{10.0};
    const TrigInfo info = getTrigInfo(trigger);
    std::vector<const xAOD::TauJet*> offline_for_l1_tau_vec_1p; // offline 1p taus
    std::vector<const xAOD::TauJet*> offline_for_l1_tau_vec_3p; // offline 3p taus
    std::vector<const xAOD::EmTauRoI*> legacyL1rois; //  used for studying legacy L1 performance
    std::vector<const xAOD::eFexTauRoI*> eFexphase1L1rois; // used for studying phase1 L1 performance 
    std::vector<const xAOD::jFexTauRoI*> jFexphase1L1rois; // used for studying phase1 L1 performance 

    for( auto pairObj: pairObjs )
    {
      int nTracks=-1;
      pairObj.first->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);
      ATH_MSG_DEBUG("NTracks Offline: " << nTracks);
      // filling vectors 1p and mp
      if(nTracks==1 && ( pairObj.first->pt() > (L1thr-thresholdOffset)*1.e3)){
        offline_for_l1_tau_vec_1p.push_back(pairObj.first);
      }else if(nTracks==3 && ( pairObj.first->pt() > (L1thr-thresholdOffset)*1.e3)){
        offline_for_l1_tau_vec_3p.push_back(pairObj.first); 
      }
    }

    // retrieve L1 items 
    if(trigL1Item.find("L1eTAU") != std::string::npos){
      eFexphase1L1rois = getL1items_efex(ctx,L1thr);
    } else if (trigL1Item.find("L1jTAU") != std::string::npos){
      jFexphase1L1rois = getL1items_jfex(ctx,L1thr);
    } else if (trigL1Item.find("L1cTAU") != std::string::npos){
      eFexphase1L1rois = getL1items_efex(ctx,L1thr);
    } else{
      legacyL1rois = getL1items_legacy(trigL1Item,ctx);
    }    
     
    fillL1(trigL1Item, legacyL1rois, eFexphase1L1rois, jFexphase1L1rois);

    fillL1Efficiencies(ctx, offline_for_l1_tau_vec_1p, "1P", trigL1Item, legacyL1rois, eFexphase1L1rois, jFexphase1L1rois);
    fillL1Efficiencies(ctx, offline_for_l1_tau_vec_3p, "3P", trigL1Item, legacyL1rois, eFexphase1L1rois, jFexphase1L1rois);
  
    offline_for_l1_tau_vec_1p.clear();
    offline_for_l1_tau_vec_3p.clear();
    legacyL1rois.clear();
    eFexphase1L1rois.clear();
    jFexphase1L1rois.clear();
}

void TrigTauMonitorAlgorithm::fillHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const std::string& trigL1Item, float L1thr, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec, const std::string& nProng) const
{
  ATH_MSG_DEBUG("Fill HLT efficiencies: " << trigger);

  /* Efficiency for single leg tau triggers:
   * denominator = offline tau + matching with L1 object with dR(offline tau,L1 item) < 0.3
   * numerator = denominator + hlt fires + matching with HLT tau with dR(offline tau, HLT tau) < 0.2
  */

  std::vector<const xAOD::EmTauRoI*> legacyL1rois; //  used for studying legacy L1 performance
  std::vector<const xAOD::eFexTauRoI*> eFexphase1L1rois; // used for studying phase1 L1 performance 
  std::vector<const xAOD::jFexTauRoI*> jFexphase1L1rois; // used for studying phase1 L1 performance 

  // retrieve L1 items 
  if(trigL1Item.find("L1eTAU") != std::string::npos){
    eFexphase1L1rois = getL1items_efex(ctx,L1thr);
  } else if (trigL1Item.find("L1jTAU") != std::string::npos){
    jFexphase1L1rois = getL1items_jfex(ctx,L1thr);
  } else if (trigL1Item.find("L1cTAU") != std::string::npos){
    eFexphase1L1rois = getL1items_efex(ctx,L1thr);
  } else{
    legacyL1rois = getL1items_legacy(trigL1Item,ctx);
  }

  std::string monGroupName = trigger+"_HLT_Efficiency_"+nProng;

  auto monGroup = getGroup(monGroupName);

  auto tauPt = Monitored::Scalar<float>(monGroupName+"_tauPt",0.0);
  auto tauPt_coarse = Monitored::Scalar<float>(monGroupName+"_tauPt_coarse",0.0);
  auto tauEta = Monitored::Scalar<float>(monGroupName+"_tauEta",0.0);
  auto tauPhi = Monitored::Scalar<float>(monGroupName+"_tauPhi",0.0);
  auto averageMu = Monitored::Scalar<float>(monGroupName+"_averageMu",0.0); 
  auto HLT_match = Monitored::Scalar<bool>(monGroupName+"_HLTpass",false);

  bool L1_match = false;
  bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics | TrigDefs::allowResurrectedDecision);

  for(const auto *offline_tau : offline_tau_vec){

       L1_match = false;
 
       // check the matching offline tau with L1 item -> depending on the L1 type (legacy, phase-1 eTAU, jTAU, cTAU)
       if((trigL1Item.find("L1eTAU") != std::string::npos)  || (trigL1Item.find("L1cTAU") != std::string::npos)){ 
          for( const auto *L1roi : eFexphase1L1rois){
             L1_match = eFexphase1L1Matching(offline_tau, L1roi, 0.3 );
             if( L1_match ){
               break;
             }
          }
       } else if(trigL1Item.find("L1jTAU") != std::string::npos){ 
          for( const auto *L1roi : jFexphase1L1rois){
             L1_match = jFexphase1L1Matching(offline_tau, L1roi, 0.3 );
             if( L1_match ){
                break;
              }
          }
       } else {
          for( const auto *L1roi : legacyL1rois){
             L1_match = legacyL1Matching(offline_tau, L1roi, 0.3 );
             if( L1_match ){
                break;
              }
           }
       }

       if(!L1_match) continue; // skip this offline tau since not matched with L1 item   
 
       tauPt = offline_tau->pt()/1e3;
       tauPt_coarse = offline_tau->pt()/1e3;
       tauEta = offline_tau->eta();
       tauPhi = offline_tau->phi();
       averageMu = lbAverageInteractionsPerCrossing(ctx);
       // HLT matching  : dR matching + HLT fires
       HLT_match = HLTMatching(offline_tau, online_tau_vec, 0.2) && hlt_fires;

       fill(monGroup, tauPt_coarse, tauPt, tauEta, tauPhi, averageMu, HLT_match);
  }

  legacyL1rois.clear();
  eFexphase1L1rois.clear();
  jFexphase1L1rois.clear();

  ATH_MSG_DEBUG("After fill HLT efficiencies: " << trigger);


}

void TrigTauMonitorAlgorithm::fillDiTauHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const
{
  ATH_MSG_DEBUG("Fill DiTau HLT efficiencies: " << trigger);

  // require 2 offline taus and 2 online taus
  if(online_tau_vec.size() != 2 || offline_tau_vec.size() != 2) return;
 
  std::string monGroupName = trigger+"_DiTauHLT_Efficiency";
 
  auto monGroup = getGroup(monGroupName);

  auto dR = Monitored::Scalar<float>(monGroupName+"_dR",0.0);
  auto dEta = Monitored::Scalar<float>(monGroupName+"_dEta",0.0);
  auto dPhi = Monitored::Scalar<float>(monGroupName+"_dPhi",0.0);
  auto averageMu = Monitored::Scalar<float>(monGroupName+"_averageMu",0.0);
  auto HLT_match = Monitored::Scalar<bool>(monGroupName+"_DiTauHLTpass",false);

  // efficiency numerator : hlt fires + two offline taus matched with online taus
  bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics);
  bool tau0_match = HLTMatching(offline_tau_vec.at(0), online_tau_vec, 0.2);
  bool tau1_match = HLTMatching(offline_tau_vec.at(1), online_tau_vec, 0.2);

  dR   = offline_tau_vec.at(0)->p4().DeltaR(offline_tau_vec.at(1)->p4());
  dEta = std::abs(offline_tau_vec.at(0)->p4().Eta() - offline_tau_vec.at(1)->p4().Eta());
  dPhi = offline_tau_vec.at(0)->p4().DeltaPhi(offline_tau_vec.at(1)->p4());
  averageMu = lbAverageInteractionsPerCrossing(ctx);
  HLT_match = hlt_fires && tau0_match && tau1_match;

  fill(monGroup, dR, dEta, dPhi, averageMu, HLT_match);

  ATH_MSG_DEBUG("After fill DiTau HLT efficiencies: " << trigger);
 
}

void TrigTauMonitorAlgorithm::fillTAndPHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const std::vector<TLorentzVector>& offline_lep_vec, const std::vector<TLorentzVector>& online_lep_vec, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const
{
  ATH_MSG_DEBUG("Fill Tag and Probe HLT efficiencies: " << trigger);

  // require 1 offline taus and 1 online taus
  if(online_tau_vec.size() != 1 || offline_tau_vec.size() != 1) {
    return;
  }
  // ...and require 1 offline lepton and 1 online lepton                                                                        
  if(online_lep_vec.size() != 1 || offline_lep_vec.size() != 1) {
    return;
  }
 
  std::string monGroupName = trigger+"_TAndPHLT_Efficiency";
 
  auto monGroup = getGroup(monGroupName);

  auto tauPt = Monitored::Scalar<float>(monGroupName+"_tauPt",0.0);
  auto tauPt_coarse = Monitored::Scalar<float>(monGroupName+"_tauPt_coarse",0.0);
  auto tauEta = Monitored::Scalar<float>(monGroupName+"_tauEta",0.0);
  auto tauPhi = Monitored::Scalar<float>(monGroupName+"_tauPhi",0.0);
  auto dR = Monitored::Scalar<float>(monGroupName+"_dR",0.0);
  auto dEta = Monitored::Scalar<float>(monGroupName+"_dEta",0.0);
  auto dPhi = Monitored::Scalar<float>(monGroupName+"_dPhi",0.0);
  auto averageMu = Monitored::Scalar<float>(monGroupName+"_averageMu",0.0);
  auto HLT_match = Monitored::Scalar<bool>(monGroupName+"_TAndPHLTpass",false);

  // efficiency numerator : hlt fires + offline and online tau matched
  // plus offline and online lepton matched
  bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics);
  bool tau1_match = HLTMatching(offline_tau_vec[0], online_tau_vec, 0.2);
  bool lep1_match = HLTMatching(offline_lep_vec[0], online_lep_vec, 0.2);

  tauPt = offline_tau_vec[0]->p4().Pt()/1000;
  tauPt_coarse = offline_tau_vec[0]->p4().Pt()/1000;
  tauEta = offline_tau_vec[0]->p4().Eta();
  tauPhi = offline_tau_vec[0]->p4().Phi();
  dR   = offline_tau_vec[0]->p4().DeltaR(offline_lep_vec[0]);
  dEta = std::abs(offline_tau_vec[0]->p4().Eta() - offline_lep_vec[0].Eta());
  dPhi = offline_tau_vec[0]->p4().DeltaPhi(offline_lep_vec[0]);
  averageMu = lbAverageInteractionsPerCrossing(ctx);
  HLT_match = hlt_fires && tau1_match && lep1_match;

  fill(monGroup, tauPt, tauPt_coarse, tauEta, tauPhi, dR, dEta, dPhi, averageMu, HLT_match);

  ATH_MSG_DEBUG("After fill Tag and Probe HLT efficiencies: " << trigger);
  
}

void TrigTauMonitorAlgorithm::fillL1Efficiencies( const EventContext& ctx , const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::string& nProng, const std::string& trigL1Item, const std::vector<const xAOD::EmTauRoI*>& legacyL1rois, const std::vector<const xAOD::eFexTauRoI*>& eFexphase1L1rois, const std::vector<const xAOD::jFexTauRoI*>& jFexphase1L1rois) const
{
  ATH_MSG_DEBUG("Fill L1 efficiencies: " << trigL1Item);
  std::string monGroupName = trigL1Item+"_L1_Efficiency_"+nProng;

  auto monGroup = getGroup(monGroupName);

  auto tauPt = Monitored::Scalar<float>(monGroupName+"_tauPt",0.0);
  auto tauEta = Monitored::Scalar<float>(monGroupName+"_tauEta",0.0);
  auto tauPhi = Monitored::Scalar<float>(monGroupName+"_tauPhi",0.0);
  auto averageMu = Monitored::Scalar<float>(monGroupName+"_averageMu",0.0);
  auto L1_match = Monitored::Scalar<bool>(monGroupName+"_L1pass",false);

  for(const auto *offline_tau : offline_tau_vec){
       tauPt = offline_tau->pt()/1e3;
       tauEta = offline_tau->eta();
       tauPhi = offline_tau->phi();
       averageMu = lbAverageInteractionsPerCrossing(ctx);
  
       L1_match = false;

       if(trigL1Item.find("L1eTAU") != std::string::npos || trigL1Item.find("L1cTAU") != std::string::npos){
          for( const auto *L1roi : eFexphase1L1rois){
             L1_match = eFexphase1L1Matching(offline_tau, L1roi, 0.3 );
             if( L1_match ){
                break;
              }
          }
       } else if(trigL1Item.find("L1jTAU") != std::string::npos){ 
          for( const auto *L1roi : jFexphase1L1rois){
             L1_match = jFexphase1L1Matching(offline_tau, L1roi, 0.3 );
             if( L1_match ){
                break;
              }
          }
       } else {
          for( const auto *L1roi : legacyL1rois){
             L1_match = legacyL1Matching(offline_tau, L1roi, 0.3 );
             if( L1_match ){
                break;
              }
           }
       }
    
       fill(monGroup, tauPt, tauEta, tauPhi, averageMu, L1_match);
  }
} 

void TrigTauMonitorAlgorithm::fillL1(const std::string& trigL1Item, const std::vector<const xAOD::EmTauRoI*>& legacyL1rois, const std::vector<const xAOD::eFexTauRoI*>& eFexphase1L1rois, const std::vector<const xAOD::jFexTauRoI*>& jFexphase1L1rois)  const
{
   ATH_MSG_DEBUG("Fill L1: " << trigL1Item);

   std::string monGroupName = trigL1Item+"_L1";
    
   auto monGroup = getGroup(monGroupName);

   if(trigL1Item.find("L1eTAU") != std::string::npos || trigL1Item.find("L1cTAU") != std::string::npos){

       auto L1RoIEt           = Monitored::Collection("L1RoIEt"     , eFexphase1L1rois,  [] (const xAOD::eFexTauRoI* L1roi){ return L1roi->et()/1e3;});
       auto L1RoIEta          = Monitored::Collection("L1RoIEta"    , eFexphase1L1rois,  [] (const xAOD::eFexTauRoI* L1roi){ return L1roi->eta();});
       auto L1RoIPhi          = Monitored::Collection("L1RoIPhi"    , eFexphase1L1rois,  [] (const xAOD::eFexTauRoI* L1roi){ return L1roi->phi();});
       auto L1RoIRCore        = Monitored::Collection("L1RoIRCore"  , eFexphase1L1rois,  [] (const xAOD::eFexTauRoI* L1roi){ return L1roi->rCore();});
       auto L1RoIRHad         = Monitored::Collection("L1RoIRHad"   , eFexphase1L1rois,  [] (const xAOD::eFexTauRoI* L1roi){ return L1roi->rHad();});
   
       fill(monGroup,L1RoIEt,L1RoIEta,L1RoIPhi,L1RoIRCore,L1RoIRHad);
   } else if (trigL1Item.find("L1jTAU") != std::string::npos){
  
       auto L1RoIEt           = Monitored::Collection("L1RoIEt"     , jFexphase1L1rois,  [] (const xAOD::jFexTauRoI* L1roi){ return L1roi->et()/1e3;});
       auto L1RoIEta          = Monitored::Collection("L1RoIEta"    , jFexphase1L1rois,  [] (const xAOD::jFexTauRoI* L1roi){ return L1roi->eta();});
       auto L1RoIPhi          = Monitored::Collection("L1RoIPhi"    , jFexphase1L1rois,  [] (const xAOD::jFexTauRoI* L1roi){ return L1roi->phi();});
       auto L1RoIIso          = Monitored::Collection("L1RoIIso"    , jFexphase1L1rois,  [] (const xAOD::jFexTauRoI* L1roi){ return L1roi->iso()/1e3;});

       fill(monGroup,L1RoIEt,L1RoIEta,L1RoIPhi,L1RoIIso);

   } else {

       auto L1RoIEt           = Monitored::Collection("L1RoIEt"     , legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->eT()/1e3;});
       auto L1RoIEta          = Monitored::Collection("L1RoIEta"    , legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->eta();});
       auto L1RoIPhi          = Monitored::Collection("L1RoIPhi"    , legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->phi();});
       auto L1RoITauClus      = Monitored::Collection("L1RoITauClus", legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->tauClus()/1e3;});
       auto L1RoIEMIsol       = Monitored::Collection("L1RoIEMIsol" , legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->emIsol()/1e3;});
       auto L1RoIHadCore      = Monitored::Collection("L1RoIHadCore", legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->hadCore()/1e3;});
       auto L1RoIHadIsol      = Monitored::Collection("L1RoIHadIsol", legacyL1rois,  [] (const xAOD::EmTauRoI* L1roi){ return L1roi->hadIsol()/1e3;});

       fill(monGroup,L1RoIEt,L1RoIEta,L1RoIPhi,L1RoITauClus,L1RoIEMIsol,L1RoIHadCore,L1RoIHadIsol);
   }

   ATH_MSG_DEBUG("AFTER L1: " << trigL1Item);

}

void TrigTauMonitorAlgorithm::fillRNNInputVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec,const std::string& nProng, bool online) const
{
  ATH_MSG_DEBUG("Fill RNN input variables: " << trigger);

  auto monGroup = getGroup(trigger+( online ? "_RNN_HLT_InputScalar_"+nProng : "_RNN_Offline_InputScalar_"+nProng));  

  auto centFrac           = Monitored::Collection("centFrac", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::centFrac, detail)){
                                                      detail = std::min(detail, 1.0f);    
                                                    } return detail;});
  auto etOverPtLeadTrk    = Monitored::Collection("etOverPtLeadTrk", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::etOverPtLeadTrk, detail)){
                                                      detail = std::log10(std::max(detail, 0.1f));
                                                    } return detail;});
  auto dRmax              = Monitored::Collection("dRmax", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::dRmax, detail)){
                                                    } return detail;});
  
  auto absipSigLeadTrk    = Monitored::Collection("absipSigLeadTrk", tau_vec,  [] (const xAOD::TauJet* tau){
                                                        float detail = (tau->nTracks()>0) ? std::abs(tau->track(0)->d0SigTJVA()) : 0.;
                                                        detail = std::min(std::abs(detail), 30.0f);
                                                        return detail;});
   
  auto sumPtTrkFrac       = Monitored::Collection("sumPtTrkFrac", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::SumPtTrkFrac, detail)){
                                                    } return detail;});
  auto emPOverTrkSysP     = Monitored::Collection("emPOverTrkSysP", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::EMPOverTrkSysP, detail)){
                                                      detail = std::log10(std::max(detail, 1e-3f));
                                                    } return detail;});
  auto ptRatioEflowApprox = Monitored::Collection("ptRatioEflowApprox", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::ptRatioEflowApprox, detail)){
                                                      detail = std::min(detail, 4.0f);
                                                    } return detail;});
  auto mEflowApprox       = Monitored::Collection("mEflowApprox", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if (tau->detail(xAOD::TauJetParameters::mEflowApprox, detail)){
                                                      detail = std::log10(std::max(detail, 140.0f));
                                                    }return detail;});
  auto ptDetectorAxis     = Monitored::Collection("ptDetectorAxis", tau_vec,  [] (const xAOD::TauJet* tau){
                                                    float detail = -999;
                                                    if( tau->ptDetectorAxis() > 0){
                                                       detail = std::log10(std::min(tau->ptDetectorAxis() / 1000.0, 100.0));
                                                    } return detail;});
  auto massTrkSys         = Monitored::Collection("massTrkSys", tau_vec,  [&nProng] (const xAOD::TauJet* tau){
                                                float detail = -999;
                                                if ( tau->detail(xAOD::TauJetParameters::massTrkSys, detail) && (nProng.find("MP") != std::string::npos || nProng.find("3P") != std::string::npos)){
                                                  detail = std::log10(std::max(detail, 140.0f));
                                                }return detail;});
  
  auto trFlightPathSig    = Monitored::Collection("trFlightPathSig", tau_vec,  [&nProng] (const xAOD::TauJet* tau){
                                                float detail = -999;
                                                if ( nProng.find("MP") != std::string::npos || nProng.find("3P") != std::string::npos ) {
                                                   tau->detail(xAOD::TauJetParameters::trFlightPathSig, detail);
                                                } return detail;});
 
  fill(monGroup, centFrac,etOverPtLeadTrk,dRmax,absipSigLeadTrk,sumPtTrkFrac,emPOverTrkSysP,ptRatioEflowApprox,mEflowApprox,ptDetectorAxis,massTrkSys,trFlightPathSig);     

  ATH_MSG_DEBUG("After fill RNN input variables: " << trigger);
  
}

void TrigTauMonitorAlgorithm::fillRNNTrack(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const
{
  ATH_MSG_DEBUG("Fill RNN input Track: " << trigger);
  

  auto monGroup = getGroup(trigger+( online ? "_RNN_HLT_InputTrack" : "_RNN_Offline_InputTrack"));

  auto track_pt_jetseed_log           = Monitored::Collection("track_pt_jetseed_log", tau_vec,  [] (const xAOD::TauJet* tau){ return std::log10( tau->ptJetSeed());});
  fill(monGroup,track_pt_jetseed_log);
  
    for(const auto *tau: tau_vec){
      // Don't call ->allTracks() unless the element links are valid
      static const SG::AuxElement::ConstAccessor< std::vector<ElementLink<xAOD::TauTrackContainer>> > tauTrackAcc("tauTrackLinks");
      bool linksValid = true;
      for (const ElementLink<xAOD::TauTrackContainer>& trackEL : tauTrackAcc(*tau)) {
        if (!trackEL.isValid()) {
          linksValid = false;
        }
        break;
      }
      if (!linksValid) {
      	ATH_MSG_WARNING("Invalid track element links from TauJet in " << trigger);
      	continue;
      }

      auto tracks = tau->allTracks();
  
                                 
      auto cmp_pt = [](const xAOD::TauTrack *lhs, const xAOD::TauTrack *rhs) {
        return lhs->pt() > rhs->pt();
      };
      std::sort(tracks.begin(), tracks.end(), cmp_pt);
                              
      /*                                                                                                         
      unsigned int max_tracks = 10;
      if (tracks.size() > max_tracks) {
        tracks.resize(max_tracks);
      }
      */

      auto n_track = Monitored::Scalar<int>("n_track",0);
      n_track = tracks.size();

      auto track_pt_log = Monitored::Collection("track_pt_log", tracks, [](const xAOD::TauTrack *track){return std::log10( track->pt()); }); 
 
      auto track_eta = Monitored::Collection("track_eta", tracks, [](const xAOD::TauTrack *track){return track->eta(); });
   
      auto track_phi = Monitored::Collection("track_phi", tracks, [](const xAOD::TauTrack *track){return track->phi(); }); 

      auto track_dEta = Monitored::Collection("track_dEta", tracks, [&tau](const xAOD::TauTrack *track){auto ddeta=track->eta()- tau->eta();return ddeta; });

      auto track_dPhi = Monitored::Collection("track_dPhi", tracks, [&tau](const xAOD::TauTrack *track){return track->p4().DeltaPhi(tau->p4());}); 

      auto track_z0sinthetaTJVA_abs_log = Monitored::Collection("track_z0sinthetaTJVA_abs_log", tracks, [](const xAOD::TauTrack *track){return track->z0sinthetaTJVA(); }); 

      auto track_d0_abs_log = Monitored::Collection("track_d0_abs_log", tracks, [](const xAOD::TauTrack *track){return  std::log10( std::abs(track->track()->d0()) + 1e-6); }); 

      auto track_nIBLHitsAndExp = Monitored::Collection("track_nIBLHitsAndExp", tracks, [](const xAOD::TauTrack *track){
                                                    uint8_t inner_pixel_hits, inner_pixel_exp;
                                                    const auto success1_innerPixel_hits = track->track()->summaryValue(inner_pixel_hits, xAOD::numberOfInnermostPixelLayerHits);
                                                    const auto success2_innerPixel_exp = track->track()->summaryValue(inner_pixel_exp, xAOD::expectInnermostPixelLayerHit);
                                                    float detail = -999;
                                                    if (success1_innerPixel_hits && success2_innerPixel_exp) {detail=inner_pixel_exp ? inner_pixel_hits : 1.;};
                                                    return   detail; });

      auto track_nPixelHitsPlusDeadSensors = Monitored::Collection("track_nPixelHitsPlusDeadSensors", tracks, [](const xAOD::TauTrack *track){
                                                    uint8_t pixel_hits, pixel_dead;
                                                    const auto success1_pixel_hits = track->track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
                                                    const auto success2_pixel_dead = track->track()->summaryValue(pixel_dead, xAOD::numberOfPixelDeadSensors);
                                                    float detail = -999;
                                                    if (success1_pixel_hits && success2_pixel_dead) {detail=pixel_hits + pixel_dead;};
                                                    return   detail; });

      auto track_nSCTHitsPlusDeadSensors = Monitored::Collection("track_nSCTHitsPlusDeadSensors", tracks, [](const xAOD::TauTrack *track){
                                                    uint8_t sct_hits, sct_dead;
                                                    const auto success1_sct_hits = track->track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
                                                    const auto success2_sct_dead = track->track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
                                                    float detail = -999;
                                                    if (success1_sct_hits && success2_sct_dead) {detail=sct_hits + sct_dead;};
                                                    return   detail; });
                                                  

      fill(monGroup,n_track,track_pt_log,track_eta,track_phi,track_dEta,track_dPhi,track_z0sinthetaTJVA_abs_log,track_d0_abs_log,track_nIBLHitsAndExp,track_nPixelHitsPlusDeadSensors,track_nSCTHitsPlusDeadSensors);
    }

  ATH_MSG_DEBUG("After fill  RNN input Track: " << trigger);
  
}

void TrigTauMonitorAlgorithm::fillRNNCluster(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const
{
  ATH_MSG_DEBUG("Fill RNN input Cluster: " << trigger << " for online/offline " << online);
  
  auto monGroup = getGroup(trigger+( online ? "_RNN_HLT_InputCluster" : "_RNN_Offline_InputCluster"));
  
  for(const auto *tau: tau_vec){

    auto cluster_pt_jetseed_log = Monitored::Collection("cluster_pt_jetseed_log", tau_vec,  [] (const xAOD::TauJet* tau){ return std::log10( tau->ptJetSeed());});

    std::vector<const xAOD::CaloCluster *> clusters;

    for (const xAOD::IParticle* particle : tau->clusters()) {
      const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>(particle);
      clusters.push_back(cluster); 
    } 
  
    auto et_cmp = [](const xAOD::CaloCluster *lhs,
		     const xAOD::CaloCluster *rhs) {
      return lhs->et() > rhs->et();
    };
    std::sort(clusters.begin(), clusters.end(), et_cmp);

    /*
    unsigned int max_clusters = 6;
    if (clusters.size() > max_clusters) {
      clusters.resize(max_clusters);
    }
    */

    auto n_cluster = Monitored::Scalar<int>("n_cluster",0); 
    n_cluster = clusters.size();

    auto cluster_et_log = Monitored::Collection("cluster_et_log",clusters, [](const xAOD::CaloCluster *cluster){return std::log10( cluster->et()); });
    auto cluster_eta =  Monitored::Collection("cluster_eta", clusters, [](const xAOD::CaloCluster *cluster){return cluster->eta();});
    auto cluster_phi =  Monitored::Collection("cluster_phi", clusters, [](const xAOD::CaloCluster *cluster){return cluster->phi();});
    auto cluster_dEta = Monitored::Collection("cluster_dEta", clusters, [&tau](const xAOD::CaloCluster *cluster){auto ddeta=cluster->eta()- tau->eta();return ddeta; });
    auto cluster_dPhi = Monitored::Collection("cluster_dPhi", clusters, [&tau](const xAOD::CaloCluster *cluster){return cluster->p4().DeltaPhi(tau->p4()); }); 
    auto cluster_SECOND_R_log10 = Monitored::Collection("cluster_SECOND_R_log10", clusters, [](const xAOD::CaloCluster *cluster){
                                              double detail = -999.;
                                              const auto success_SECOND_R = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_R,detail);
                                              if (success_SECOND_R) detail = std::log10(detail + 0.1);
                                              return detail;});

    auto cluster_SECOND_LAMBDA_log10 = Monitored::Collection("cluster_SECOND_LAMBDA_log10", clusters, [](const xAOD::CaloCluster *cluster){
                                              double detail = -999.;
                                              const auto success_SECOND_LAMBDA = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_LAMBDA, detail);
                                              if (success_SECOND_LAMBDA) detail = std::log10(detail + 0.1); 
                                              return detail;});

    auto cluster_CENTER_LAMBDA_log10 = Monitored::Collection("cluster_CENTER_LAMBDA_log10", clusters, [](const xAOD::CaloCluster *cluster){
                                              double detail = -999.;
                                              const auto success_CENTER_LAMBDA = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::CENTER_LAMBDA, detail);
                                              if (success_CENTER_LAMBDA) detail = std::log10(detail + 1e-6); 
                                              return detail;});                                                  

    fill(monGroup,n_cluster,cluster_pt_jetseed_log,cluster_et_log,cluster_eta,cluster_phi,cluster_dEta,cluster_dPhi,cluster_SECOND_R_log10,cluster_SECOND_LAMBDA_log10,cluster_CENTER_LAMBDA_log10);
  }

  ATH_MSG_DEBUG("After fill  RNN input Cluster: " << trigger);
}

void TrigTauMonitorAlgorithm::fillbasicVars(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec,const std::string& nProng, bool online) const
{
  ATH_MSG_DEBUG("Fill Basic Variables: " << trigger); 

  auto monGroup = getGroup(trigger+( online ? "HLT_basicVars_"+nProng : "Offline_basicVars_"+nProng));  

  auto hEFEt           = Monitored::Collection("hEFEt", tau_vec,  [] (const xAOD::TauJet* tau){return tau->pt()/1000; });
  auto hEFEta          = Monitored::Collection("hEFEta", tau_vec,  [] (const xAOD::TauJet* tau){return tau->eta(); });                                                     

  auto hEFPhi          = Monitored::Collection("hEFPhi", tau_vec,  [] (const xAOD::TauJet* tau){return tau->phi(); });
  auto hEFnTrack       = Monitored::Collection("hEFnTrack", tau_vec,  [] (const xAOD::TauJet* tau){
      int EFnTrack=-1;
      tau->detail(xAOD::TauJetParameters::nChargedTracks, EFnTrack);
      return EFnTrack; }); 
  auto hEFnWideTrack   = Monitored::Collection("hEFnWideTrack", tau_vec,  [] (const xAOD::TauJet* tau){
      int EFWidenTrack(-1);
      EFWidenTrack = tau->nTracksIsolation();
      return EFWidenTrack; }); 

  const TrigInfo info = getTrigInfo(trigger);

  auto hRNNScore = Monitored::Collection("hRNNScore", tau_vec, [] (const xAOD::TauJet* tau){ return tau->discriminant(xAOD::TauJetParameters::RNNJetScore);});
  auto hRNNScoreSigTrans = Monitored::Collection("hRNNScoreSigTrans", tau_vec, [] (const xAOD::TauJet* tau){ return tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);});

  auto haverageMu = Monitored::Scalar<float>("haverageMu",0.0); 
  haverageMu = lbAverageInteractionsPerCrossing(ctx);
 
  auto hTauVertexX =  Monitored::Collection("hTauVertexX", tau_vec,  [] (const xAOD::TauJet* tau){ double vtx = -999.;
                                                                                             if( tau->vertex() != nullptr ) {vtx = tau->vertex()->x();}
                                                                                             return vtx;});

  auto hTauVertexY =  Monitored::Collection("hTauVertexY", tau_vec,  [] (const xAOD::TauJet* tau){ double vty = -999.;
                                                                                             if( tau->vertex() != nullptr ) {vty = tau->vertex()->y();}
                                                                                             return vty;});
  
  auto hTauVertexZ =  Monitored::Collection("hTauVertexZ", tau_vec,  [] (const xAOD::TauJet* tau){ double vtz = -999.;
                                                                                             if( tau->vertex() != nullptr ) {vtz = tau->vertex()->z();}
                                                                                             return vtz;});

  fill(monGroup, hEFEt,hEFEta,hEFPhi,hEFnTrack,hEFnWideTrack, hRNNScore, hRNNScoreSigTrans, haverageMu, hTauVertexX, hTauVertexY, hTauVertexZ);

  ATH_MSG_DEBUG("After fill Basic variables: " << trigger);

}

void TrigTauMonitorAlgorithm::fillDiTauVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec) const
{
  ATH_MSG_DEBUG("Fill DiTau Variables: " << trigger); 

  auto monGroup = getGroup(trigger+"_DiTauVars");

  if(tau_vec.size() != 2) return; 
   
  auto leadEFEt = Monitored::Scalar<float>("hleadEFEt",0.0);
  auto subleadEFEt = Monitored::Scalar<float>("hsubleadEFEt",0.0);
  auto leadEFEta = Monitored::Scalar<float>("hleadEFEta",0.0);
  auto subleadEFEta = Monitored::Scalar<float>("hsubleadEFEta",0.0);
  auto leadEFPhi = Monitored::Scalar<float>("hleadEFPhi",0.0);
  auto subleadEFPhi = Monitored::Scalar<float>("hsubleadEFPhi",0.0);
  auto dR = Monitored::Scalar<float>("hdR",0.0);
  auto dEta = Monitored::Scalar<float>("hdEta",0.0);  
  auto dPhi = Monitored::Scalar<float>("hdPhi",0.0);
  
  auto Pt = Monitored::Scalar<float>("Pt",0.0);
  auto Eta = Monitored::Scalar<float>("Eta",0.0);
  auto Phi = Monitored::Scalar<float>("Phi",0.0); 
  auto M = Monitored::Scalar<float>("M",0.0);
  auto dPt = Monitored::Scalar<float>("dPt",0.0); 

  // get the index of the leading and the subleading tau
  unsigned int index0=0, index1=1;
  if(tau_vec.at(1)->p4().Pt() > tau_vec.at(0)->p4().Pt()){
    index0=1;
    index1=0;
  } 

  TLorentzVector leadTau4V, subleadTau4V, diTau4V;

  leadTau4V.SetPtEtaPhiM(0,0,0,0);
  subleadTau4V.SetPtEtaPhiM(0,0,0,0);

  leadTau4V = tau_vec.at(index0)->p4();
  subleadTau4V = tau_vec.at(index1)->p4();

  diTau4V = leadTau4V + subleadTau4V;

  leadEFEt = leadTau4V.Pt()/1000;
  subleadEFEt = subleadTau4V.Pt()/1000;
  leadEFEta = leadTau4V.Eta();
  subleadEFEta = subleadTau4V.Eta();
  leadEFPhi = leadTau4V.Phi();
  subleadEFPhi = subleadTau4V.Phi();
  dR = leadTau4V.DeltaR(subleadTau4V);
  dEta = std::abs(leadTau4V.Eta() - subleadTau4V.Eta());
  dPhi = leadTau4V.DeltaPhi(subleadTau4V);

  dPt = std::abs((leadTau4V.Pt() - subleadTau4V.Pt())/1000);
  Pt = diTau4V.Pt()/1000;
  Eta = diTau4V.Eta();
  Phi = diTau4V.Phi();
  M = diTau4V.M()/1000;
  
  fill(monGroup, leadEFEt, subleadEFEt, leadEFEta, subleadEFEta, leadEFPhi, subleadEFPhi, dR, dEta, dPhi, dPt, Pt, Eta, Phi, M);

  ATH_MSG_DEBUG("After fill DiTau variables: " << trigger); 
}


void TrigTauMonitorAlgorithm::fillTagAndProbeVars(const std::string& trigger, const std::vector<TLorentzVector>& tau_vec, const std::vector<TLorentzVector>& lep_vec) const
{
  ATH_MSG_DEBUG("Fill Tag & Probe Variables: " << trigger); 

  auto monGroup = getGroup(trigger+"_TAndPVars");

  if(tau_vec.empty() || lep_vec.empty() ) return; 
   
  auto dR = Monitored::Scalar<float>("hdR",0.0);
  auto dEta = Monitored::Scalar<float>("hdEta",0.0);  
  auto dPhi = Monitored::Scalar<float>("hdPhi",0.0);
  auto dPt = Monitored::Scalar<float>("dPt",0.0); 
  
  auto Pt = Monitored::Scalar<float>("Pt",0.0);
  auto Eta = Monitored::Scalar<float>("Eta",0.0);
  auto Phi = Monitored::Scalar<float>("Phi",0.0); 
  auto M = Monitored::Scalar<float>("M",0.0);

  // choose a tau lepton pair with dR > 0.3 to fill the plot
  // note : there must be always at least one  pair with dR > 0.3 if the trigger fires
  uint index_tau = 0;
  uint index_lep = 0;
  
  for(uint i=0; i < tau_vec.size(); i++) {
      for(uint j=0; j< lep_vec.size(); j++){
          if( tau_vec[i].DeltaR(lep_vec[j]) >= 0.3){
              index_tau = i;
              index_lep = j;
          }  
      }
  }

  dR = tau_vec[index_tau].DeltaR(lep_vec[index_lep]);
  dEta = std::abs(tau_vec[index_tau].Eta() - lep_vec[index_lep].Eta());
  dPhi = tau_vec[index_tau].DeltaPhi(lep_vec[index_lep]);
  dPt = std::abs((tau_vec[index_tau].Pt() - lep_vec[index_lep].Pt())/1000);

  TLorentzVector diTau4V = tau_vec[index_tau] + lep_vec[index_lep];

  Pt = diTau4V.Pt()/1000;
  Eta = diTau4V.Eta();
  Phi = diTau4V.Phi();
  M = diTau4V.M()/1000;
  
  fill(monGroup, dR, dEta, dPhi, dPt, Pt, Eta, Phi, M);

  ATH_MSG_DEBUG("After fill Tag & Probe variables: " << trigger); 
}


TrigInfo TrigTauMonitorAlgorithm::getTrigInfo(const std::string& trigger) const{ 
  return m_trigInfo.at(trigger); 
}


void TrigTauMonitorAlgorithm::setTrigInfo(const std::string& trigger)
{ 

  std::string type="",l1item="";
  float hlthr=0.,electhr=0.,muthr=0.,l1thr=0.;
  bool isL1=false,isDiTau=false,isTAndP=false,hasElectron=false,hasMuon=false;

  size_t l=trigger.length();
  size_t pos=trigger.find('_');
  std::string substr =trigger.substr(pos+1,l);
  std::vector<std::string> names;
  names.push_back(trigger.substr(0,pos));

  while(substr.find('_')!=std::string::npos)
  {
    pos = substr.find('_');
    names.push_back(substr.substr(0,pos));
    substr = substr.substr(pos+1,substr.length());    
  }

  names.push_back(substr);

  if(trigger.find("ivarloose")!=std::string::npos) hasElectron = true;
  else if(trigger.find("ivarmedium")!=std::string::npos) hasMuon = true;

  if(!hasElectron && !hasMuon) hlthr = std::stof(names[1].substr(3,names[1].length()));
  //If lepton+tau trigger use tau threshold
  else if(hasElectron) {hlthr = std::stof(names[4].substr(3,names[4].length()));electhr = std::stof(names[1].substr(1,names[1].length()));}
  else if(hasMuon) {hlthr = std::stof(names[3].substr(3,names[4].length()));muthr = std::stof(names[1].substr(2,names[1].length()));}

  if(names[0].find("L1")!=std::string::npos) isL1=true;

  if(names.size() <= 4)
  {
    type=names[2];
    l1item =names[3];
     
    if(names[3].find("L1TAU") !=std::string::npos){
      l1thr = std::stof(names[3].substr(5,names[3].length()));
    } 
    else if (names[3].find("L1eTAU") !=std::string::npos){
      l1thr = std::stof(names[3].substr(6,names[3].length()));
    } 
    else if (names[3].find("L1jTAU") !=std::string::npos){
      l1thr = std::stof(names[3].substr(6,names[3].length()));
    }
    else if (names[3].find("L1cTAU") !=std::string::npos){
      l1thr = std::stof(names[3].substr(6,names[3].length()));
    }
   
  } 
  else if ( names[4].find("L1TAU") !=std::string::npos || names[4].find("L1eTAU") !=std::string::npos || names[4].find("L1jTAU") !=std::string::npos || names[4].find("L1cTAU") !=std::string::npos )  {
    type=names[3];
    l1item =names[4];

    if(names[4].find("L1TAU") !=std::string::npos){
      l1thr = std::stof(names[4].substr(5,names[4].length())); 
    }
    else if(names[4].find("L1eTAU") !=std::string::npos || names[4].find("L1jTAU") !=std::string::npos || names[4].find("L1cTAU") !=std::string::npos){
      l1thr = std::stof(names[4].substr(6,names[4].length()));
    }
  }else l1thr = -1.; //This applies to T&P chains

  // check if it's ditau trigger -> 2 times 'tau' + '03dRAB'
  size_t count = 0;
  for (size_t pos =0; (pos=trigger.find("tau", pos)) != std::string::npos; ++pos, ++count);
  if(trigger.find("03dRAB") != std::string::npos && count == 2){
      isDiTau = true;
  } else if (trigger.find("03dRAB") != std::string::npos && count == 1){
      isTAndP = true;
      if(trigger.find("ivarloose")!=std::string::npos) hasElectron = true;
      else if(trigger.find("ivarmedium")!=std::string::npos) hasMuon = true;
  }  

  TrigInfo info{trigger,l1item,type,isL1,hlthr,l1thr,false,isDiTau,isTAndP,hasElectron,hasMuon,electhr,muthr};

  m_trigInfo[trigger] = info;
}

StatusCode TrigTauMonitorAlgorithm::examineTruthTau(const xAOD::TruthParticle& xTruthTau) const
{

  if(!xTruthTau.hasDecayVtx()) return StatusCode::FAILURE;

  xTruthTau.auxdecor<char>("IsLeptonicTau") = false;
    
  TLorentzVector VisSumTLV;
  xTruthTau.auxdecor<double>("pt_vis") = 0;
  xTruthTau.auxdecor<double>("eta_vis") = 0;
  xTruthTau.auxdecor<double>("phi_vis") = 0;
  xTruthTau.auxdecor<double>("mvis") = 0;
  xTruthTau.auxdecor<int>("childChargeSum") = 0;
  xTruthTau.auxdecor<int>("nTracks") = 0;
  
    
  const xAOD::TruthVertex* decayvtx = xTruthTau.decayVtx();
  if(decayvtx)
    {
      const std::size_t nChildren = decayvtx->nOutgoingParticles();
      for ( std::size_t iChild = 0; iChild != nChildren; ++iChild )
        {
          const xAOD::TruthParticle * child = decayvtx->outgoingParticle(iChild);
          if(child){
             if( ( child->absPdgId() == 12 || 
                   child->absPdgId() == 14 || 
                   child->absPdgId() == 16 ) ) continue;
             if(child->status()==3) continue;
             ATH_MSG_DEBUG("child "<< child->pdgId() << ", status "<< child->status() << ", charge "<< child->charge());
             if( ( child->absPdgId() == 11 || 
                   child->absPdgId() == 13 || 
                   child->absPdgId() == 15 ) ) xTruthTau.auxdecor<char>("IsLeptonicTau") = true;
             VisSumTLV += child->p4();
             xTruthTau.auxdecor<int>("childChargeSum") += child->charge();
             xTruthTau.auxdecor<int>("nTracks") += std::abs(child->charge());
           }
        }
    }

  xTruthTau.auxdecor<double>("pt_vis")  = VisSumTLV.Pt();
  xTruthTau.auxdecor<double>("eta_vis") = VisSumTLV.Eta();
  xTruthTau.auxdecor<double>("phi_vis") = VisSumTLV.Phi();
  xTruthTau.auxdecor<double>("mvis")   = VisSumTLV.M();

  if(xTruthTau.auxdecor<int>("childChargeSum")!=xTruthTau.charge() || xTruthTau.auxdecor<int>("nTracks")%2==0)
  { 
      ATH_MSG_WARNING("Strange tau: charge " << xTruthTau.auxdecor<int>("childChargeSum") << " and " 
                      << xTruthTau.auxdecor<int>("nTracks")  << " tracks");
      const std::size_t nChildren = decayvtx->nOutgoingParticles();
      for ( std::size_t iChild = 0; iChild != nChildren; ++iChild )
      {
         const xAOD::TruthParticle * child = decayvtx->outgoingParticle(iChild);
         if(child){
            ATH_MSG_WARNING("child "<< child->pdgId() << ", status "<< child->status() << ", charge "<< child->charge());
         }
      }
  }

  return StatusCode::SUCCESS;
}

void TrigTauMonitorAlgorithm::fillEFTauVsTruth(const std::vector<const xAOD::TauJet*>& ef_taus,const std::vector<const xAOD::TruthParticle*>& true_taus, const std::string& trigger, const std::string& nProng) const
{
  ATH_MSG_DEBUG ("TrigTauMonitorAlgorithm::fillEFTauVsTruth");

  std::string monGroupName = trigger+"_EFVsTruth_"+nProng;
  
  auto monGroup = getGroup(monGroupName);

  std::vector<float> ratio;
  std::vector<float> ptvis;
  std::vector<float> etavis;
  std::vector<float> phivis;
  std::vector<float> mvis;

  auto Etratio = Monitored::Collection("Etratio",ratio);
  auto pt_vis = Monitored::Collection("pt_vis",ptvis);
  auto eta_vis = Monitored::Collection("eta_vis",etavis);
  auto phi_vis = Monitored::Collection("phi_vis",phivis);
  auto mass_vis = Monitored::Collection("mass_vis",mvis);

  float tmpdR(0.2), matchedRatio(-999.);
  float matchedptvis(-999.),matchedetavis(-999.),matchedphivis(-999.),matchedmvis(-999.);

  //Visible-Truth Tau matching to EF(HLT Tau)
  for(auto &aEFTau : ef_taus){
     
     for(auto &truthTau : true_taus){
       TLorentzVector truthTau4V;
       truthTau4V.SetPtEtaPhiM(truthTau->auxdata<double>("pt_vis"),truthTau->auxdata<double>("eta_vis"),truthTau->auxdata<double>("phi_vis"),truthTau->auxdata<double>("mvis"));
       float dR = truthTau4V.DeltaR(aEFTau->p4());
       if(dR < tmpdR){
       matchedptvis = (truthTau->auxdata<double>("pt_vis")/1e3);
       matchedetavis = truthTau->auxdata<double>("eta_vis");
       matchedphivis = truthTau->auxdata<double>("phi_vis");
       matchedmvis = truthTau->auxdata<double>("mvis");
       matchedRatio = (aEFTau->p4().Pt()-truthTau->auxdata<double>("pt_vis"))/truthTau->auxdata<double>("pt_vis");
       }
     }
      if(matchedptvis>0.){
      ptvis.push_back(matchedptvis);
      etavis.push_back(matchedetavis);
      phivis.push_back(matchedphivis);
      mvis.push_back(matchedmvis);
      ratio.push_back(matchedRatio);      
     }
  }

  fill(monGroup,pt_vis,eta_vis,phi_vis,mass_vis,Etratio);

  ATH_MSG_DEBUG("After fillEFTauVsTruth ");

}

void TrigTauMonitorAlgorithm::fillTruthEfficiency(const std::vector<const xAOD::TauJet*>& online_tau_vec,const std::vector<const xAOD::TruthParticle*>& true_taus, const std::string& trigger, const std::string& nProng) const
{

  ATH_MSG_DEBUG("Truth Tau Matching to Offline and Online Taus for trigger");

  std::string monGroupName = trigger+"_Truth_Efficiency_"+nProng;

  auto monGroup = getGroup(monGroupName);

  // Truth Tau + HLT Tau / Truth Tau
  
  auto pt_vis = Monitored::Scalar<float>(monGroupName+"_pt_vis",0.0);
  auto eta_vis = Monitored::Scalar<float>(monGroupName+"_eta_vis",0.0);
  auto phi_vis = Monitored::Scalar<float>(monGroupName+"_phi_vis",0.0);
  auto HLT_truth_match = Monitored::Scalar<bool>(monGroupName+"_HLTpass",false);  

  bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics | TrigDefs::allowResurrectedDecision);
  for(const auto &true_tau : true_taus){

       pt_vis  = true_tau->auxdata<double>("pt_vis")/1e3;
       eta_vis = true_tau->auxdata<double>("eta_vis");
       phi_vis = true_tau->auxdata<double>("phi_vis");

       HLT_truth_match = HLTTruthMatching(true_tau, online_tau_vec, 0.2) && hlt_fires;

       fill(monGroup, pt_vis, eta_vis, phi_vis, HLT_truth_match);
        
  } 

}
