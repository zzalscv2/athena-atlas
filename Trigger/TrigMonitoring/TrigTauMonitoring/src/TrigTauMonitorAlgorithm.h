/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORALGORITHM_H

#include "xAODTau/TauJet.h"
#include "xAODTau/TauxAODHelpers.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "TrigTauInfo.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "StoreGate/ReadDecorHandleKey.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "CxxUtils/phihelper.h"

class TrigTauMonitorAlgorithm : public AthMonitorAlgorithm {
public:
  TrigTauMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~TrigTauMonitorAlgorithm();
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:
  // List of triggers to monitor
  std::vector<std::string> m_trigList;

  // List of triggers from menu (before duplicate filtering)
  Gaudi::Property<std::vector<std::string>> m_trigInputList{this, "TriggerList", {}};
  Gaudi::Property<bool> m_isMC{this, "isMC", false};

  // Enable total efficiency histograms
  // Note: Should only be used when reprocessing EB or MC data. Comparisons of total efficiencies between chains on normal data-taking 
  // conditions would be meaningless, since different L1/HLT items can have different prescales, and are not within a Coherent-Prescale-Set
  Gaudi::Property<bool> m_doTotalEfficiency{this, "doTotalEfficiency", false, "Do total efficiency histograms"};

  // Phase1 L1 item name -> threshold mapping
  Gaudi::Property<std::map<int, int>> m_L1Phase1ThrMap_eTAU {this, "eTAUL1Phase1ThrMap", {}, "eTAU Phase1 name to ET threshold mapping"};
  Gaudi::Property<std::map<int, int>> m_L1Phase1ThrMap_jTAU {this, "jTAUL1Phase1ThrMap", {}, "jTAU Phase1 name to ET threshold mapping"};

  // TrigTauInfo objects, containing all information from each trigger
  std::map<std::string, TrigTauInfo> m_trigInfo;
  inline std::map<std::string, TrigTauInfo> getTrigInfoMap() { return m_trigInfo; }
  inline const TrigTauInfo& getTrigInfo(const std::string& trigger) const { return m_trigInfo.at(trigger); }

  // Navigation method called by executeNavigation
  StatusCode executeNavigation(const EventContext& ctx, const std::string& trigItem, std::vector<std::pair<const xAOD::TauJet*, const TrigCompositeUtils::Decision*>> &) const;

  // Filling of HLT histograms:
  void fillDistributions(const EventContext& ctx, const std::vector< std::pair< const xAOD::TauJet*, const TrigCompositeUtils::Decision * >>& pairObjs, const std::string& trigger) const;

  void fillHLTEfficiencies(const EventContext& ctx,const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec, const std::string& nProng) const;
  void fillRNNInputVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec,const std::string& nProng, bool online) const;
  void fillRNNTrack(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const;
  void fillRNNCluster(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const;
  void fillbasicVars(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const;

  void fillDiTauHLTEfficiencies(const EventContext& ctx,const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const;
  void fillDiTauVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec) const;

  void fillTAndPHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const std::vector<TLorentzVector>& offline_lep_vec, const std::vector<TLorentzVector>& online_lep_vec, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const;
  void fillTagAndProbeVars(const std::string& trigger, const std::vector<TLorentzVector>& tau_vec, const std::vector<TLorentzVector>& lep_vec) const;

  void fillTruthEfficiency(const std::vector<const xAOD::TauJet*>& online_tau_vec_all, const std::vector<const xAOD::TruthParticle*>& true_taus, const std::string& trigger, const std::string& nProng) const;
  void fillTruthVars(const std::vector<const xAOD::TauJet*>& tau_vec, const std::vector<const xAOD::TruthParticle*>& true_taus, const std::string& trigger, const std::string& nProng) const; 

  // Filling of L1 histograms
  void fillL1Distributions(const EventContext& ctx, const std::vector< std::pair< const xAOD::TauJet*, const TrigCompositeUtils::Decision * >>& pairObjs, const std::string& trigger) const;
  void fillL1Vars(const std::string& trigL1Item, const std::vector<const xAOD::EmTauRoI*>& legacyL1rois, const std::vector<const xAOD::eFexTauRoI*>& eFexphase1L1rois, const std::vector<const xAOD::jFexTauRoI*>& jFexphase1L1rois)  const;
  void fillL1Efficiencies(const EventContext& ctx, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::string& nProng, const std::string& trigL1Item, const std::vector<const xAOD::EmTauRoI*>& legacyL1rois, const std::vector<const xAOD::eFexTauRoI*>& eFexphase1L1rois, const std::vector<const xAOD::jFexTauRoI*>& jFexphase1L1rois) const;

  // Get matched L1 RoIs:
  std::tuple<std::vector<const xAOD::eFexTauRoI*>, std::vector<const xAOD::jFexTauRoI*>, std::vector<const xAOD::EmTauRoI*>> getL1RoIs(const EventContext& ctx, const std::string& trigger) const;
  std::vector<const xAOD::eFexTauRoI*> getL1items_efex(const EventContext& ctx, float L1thr) const;
  std::pair<std::vector<const xAOD::eFexTauRoI*>, std::vector<const xAOD::jFexTauRoI*>> getL1items_cfex(const EventContext& ctx, float L1thr) const;
  std::vector<const xAOD::jFexTauRoI*> getL1items_jfex(const EventContext& ctx, float L1thr) const;
  std::vector<const xAOD::EmTauRoI*> getL1items_legacy(const EventContext& ctx, const std::string& trigL1Item) const;

  // Get matched offline objects
  std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> getOfflineTaus(const std::vector< std::pair< const xAOD::TauJet*, const TrigCompositeUtils::Decision * >>& pairObjs, const float threshold) const;
  
  // Process truth tau object
  StatusCode examineTruthTau(const xAOD::TruthParticle* xTruthParticle) const;

  // Helper functions
  inline double dR(const double eta1, const double phi1, const double eta2, const double phi2) const
  {
    double deta = std::fabs(eta1 - eta2);
    double dphi = std::fabs(CxxUtils::wrapToPi(phi1-phi2));
    return sqrt(deta*deta + dphi*dphi);
  }

  template <typename T1 = xAOD::TauJet, typename T2 = xAOD::TauJet>
  inline bool matchObjects(const T1* tau, const std::vector<const T2*>& tau_vec, float threshold) const
  {
    for(auto tau_2 : tau_vec) {
      if(tau->p4().DeltaR(tau_2->p4()) < threshold) return true;
    }
    return false;
  }

  inline bool matchObjects(const TLorentzVector& tau, const std::vector<TLorentzVector>& tau_vec, float threshold) const
  {
    for(auto& tau_2 : tau_vec) {
      if(tau.DeltaR(tau_2) < threshold) return true;
    }
    return false;
  }

  template <typename T1 = xAOD::TauJet, typename T2 = xAOD::eFexTauRoI>
  inline bool matchObjects(const T1* tau_1, const T2* tau_2, float threshold) const
  {
    return dR(tau_1->eta(), tau_1->phi(), tau_2->eta(), tau_2->phi()) < threshold;
  }

  const std::string getOnlineContainerKey(const std::string& trigger) const;

  // StorageGate keys
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{ this, "LArStatusFlag", "EventInfo.larFlags", "Key for EventInfo object" }; //To get data-dependencies right

  SG::ReadHandleKey< xAOD::TauJetContainer> m_offlineTauJetKey { this, "offlineTauJetKey", "TauJets", "Offline taujet container key" };
  SG::ReadHandleKey< xAOD::TruthParticleContainer> m_truthParticleKey { this, "truthParticleKey", "TruthParticles", "TruthParticleContainer key" };

  SG::ReadHandleKey<xAOD::ElectronContainer> m_offlineElectronKey{ this, "offlineElectronKey", "Electrons", "Offline Electron key for tau-e chains"};
  SG::ReadHandleKey<xAOD::MuonContainer> m_offlineMuonKey{ this, "offlineMuonKey", "Muons", "Offline Muon key for tau-mu chains"};

  SG::ReadHandleKey< xAOD::EmTauRoIContainer> m_legacyl1TauRoIKey { this, "legacyl1TauRoIKey","LVL1EmTauRoIs","Tau Legacy L1 RoI key"};
  SG::ReadHandleKey< xAOD::eFexTauRoIContainer>  m_phase1l1eTauRoIKey {this, "phase1l1eTauRoIKey", "L1_eTauRoI","eTau Phase1 L1 RoI key"};
  SG::ReadHandleKey< xAOD::jFexTauRoIContainer>  m_phase1l1jTauRoIKey {this, "phase1l1jTauRoIKey", "L1_jFexTauRoI","jTau Phase1 L1 RoI key"};
  SG::ReadHandleKey< xAOD::eFexTauRoIContainer>  m_phase1l1cTauRoIKey {this, "phase1l1cTauRoIKey", "L1_cTauRoI","cTau Phase1 L1 RoI key"};
  SG::ReadDecorHandleKey<xAOD::eFexTauRoIContainer> m_phase1l1cTauRoIDecorKey {this, "phase1l1cTauRoIjTauRoILinkKey", "L1_cTauRoI.jTauLink", "Decoration for the link from eTau to the matching jTau"};

  SG::ReadHandleKey< xAOD::TauJetContainer> m_hltTauJetKey { this, "hltTauJetKey", "HLT_TrigTauRecMerged_MVA", "HLT tracktwoMVA taujet container key" };
  SG::ReadHandleKey< xAOD::TauJetContainer> m_hltTauJetLLPKey { this, "hltTauJetLLPKey", "HLT_TrigTauRecMerged_LLP", "HLT tracktwoLLP taujet container key" };
  SG::ReadHandleKey< xAOD::TauJetContainer> m_hltTauJetLRTKey { this, "hltTauJetLRTKey", "HLT_TrigTauRecMerged_LRT", "HLT trackLRT taujet container key" };
  SG::ReadHandleKey< xAOD::TauJetContainer> m_hltTauJetCaloMVAOnlyKey { this, "hltTauJetCaloMVAOnlyKey", "HLT_TrigTauRecMerged_CaloMVAOnly", "HLT ptonly taujet container key" };
};
#endif
