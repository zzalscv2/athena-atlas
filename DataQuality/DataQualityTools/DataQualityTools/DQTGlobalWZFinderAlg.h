/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     DQTGlobalWZFinderAlg.h
// PACKAGE:  DataQualityTools
//
// AUTHORS:   Jahred Adelman (jahred.adelman@cern.ch)
//            Simon Viel (svielcern.ch)
//            Koos van Nieuwkoop (jvannieu@cern.ch)
//	          Samuel Alibocus (salibocu@cern.ch)
//
// ********************************************************************
#ifndef DQTGlobalWZFinderAlg_H
#define DQTGlobalWZFinderAlg_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "TriggerMatchingTool/R3MatchingTool.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthParticle.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

#include "StoreGate/ReadDecorHandleKeyArray.h"

class DQTGlobalWZFinderAlg: public AthMonitorAlgorithm
{

 public:
  
  DQTGlobalWZFinderAlg(const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~DQTGlobalWZFinderAlg() = default;

  virtual StatusCode initialize() override;
    
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;

private:

  Gaudi::Property<float_t> m_electronEtCut {this, "electronEtCut", 27};
  Gaudi::Property<float_t> m_muonPtCut {this, "muonPtCut", 27};
  Gaudi::Property<float_t> m_zCutLow {this, "zCutLow", 66.0};
  Gaudi::Property<float_t> m_zCutHigh {this, "zCutHigh", 116.0};
  Gaudi::Property<float_t> m_muonMaxEta {this, "muonMaxEta", 2.4};

  Gaudi::Property<std::vector<std::string>> m_Z_ee_trigger {this, "Z_ee_trigger", {"HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI"}};
  Gaudi::Property<std::vector<std::string>> m_Z_mm_trigger {this, "Z_mm_trigger", {"HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH"}};

  BooleanProperty m_doRunBeam{this, "doRunBeam", true};
  BooleanProperty m_doTrigger{this, "doTrigger", false};
  BooleanProperty m_do_BCID{this, "do_BCID", false};

  bool kinematicCuts(const xAOD::Egamma* particle) const;
  bool goodElectrons(const xAOD::Electron* electron_itr, const xAOD::Vertex* pVtx, const EventContext& ctx) const;
  bool antiGoodElectrons(const xAOD::Electron* electron_itr, const xAOD::Vertex* pVtx, const EventContext& ctx) const;
  void fillEleEffHistos(bool tag_good, bool probe_good, bool probe_anti_good, bool os, double el_mass) const;

  void doEleTriggerTP(const xAOD::Electron* el1, const xAOD::Electron* el2, const EventContext& ctx, bool writeTTrees, const float evtWeight, bool osel, bool ssel) const;
  void doEleTP(const xAOD::Electron* leadingAllEle, const xAOD::Electron* subleadingAllEle, const xAOD::Vertex* pVtx, const EventContext& ctx, bool writeTTrees, bool isSimulation, const float evtWeight) const; 
  void doEleContainerTP(std::vector<const xAOD::Electron*>& allElectrons, std::vector<const xAOD::Electron*>& goodelectrons, const EventContext& ctx) const;
 
  void doMuonTriggerTP(const xAOD::Muon* mu1, const xAOD::Muon* mu2, const EventContext& ctx, bool isSimulation, bool writeTTrees, const float evtWeight) const;
  void doMuonTruthEff(std::vector<const xAOD::Muon*>& goodmuonsZ, const EventContext& ctx) const;
  void doMuonLooseTP(std::vector<const xAOD::Muon*>& goodmuonsZ, const xAOD::Vertex* pVtx, const EventContext& ctx, bool isSimulation, bool writeTTrees, const float evtWeight) const;
  void doMuonInDetTP(std::vector<const xAOD::Muon*>& goodmuonsZ, const xAOD::Vertex* pVtx, const EventContext& ctx, bool isSimulation, bool writeTTrees, const float evtWeight) const;

  bool checkTruthElectron(const xAOD::Electron* electron) const;
  bool checkTruthMuon(const xAOD::Muon* muon) const;
  bool checkTruthTrack(const xAOD::TrackParticle* trk) const;

  ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool{this,"MuonSelectionTool","CP::MuonSelectionTool/MuonSelectionTool","MuonSelectionTool"};
  ToolHandle<Trig::R3MatchingTool> m_r3MatchingTool{this, "R3MatchingTool", "Trig::R3MatchingTool", "R3MatchingTool"};

  ToolHandle<IMCTruthClassifier> m_truthClassifier{this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier", "MCTruthClassifier"}; 

  SG::ReadHandleKey<xAOD::ElectronContainer> m_ElectronContainerKey
    { this, "ElectronContainerName", "Electrons", "" };
  SG::ReadHandleKey<xAOD::MuonContainer> m_MuonContainerKey
   { this, "MuonContainerName", "Muons", "" };
  SG::ReadHandleKey<xAOD::PhotonContainer> m_PhotonContainerKey
   { this, "PhotonContainerName", "Photons", ""};
  SG::ReadHandleKey<xAOD::VertexContainer> m_VertexContainerKey
    { this, "PrimaryVertexContainerName", "PrimaryVertices" };
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_TruthParticleContainerKey
   {this, "TruthParticleContainerName", "TruthParticles", "" }; 
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_idTrackParticleContainerKey
   {this, "MuonInDetTrackParticleContainerName", "InDetTrackParticles", ""};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_msTrackParticleContainerKey
   {this, "MuonExtrapolatedTrackParticleContainerName", "ExtrapolatedMuonTrackParticles", ""};

  SG::ReadDecorHandleKeyArray<xAOD::MuonContainer> m_isoMuonContainerKey
   {this, "IsoMuonVariableNames", {"Muons.ptcone20"}, "Isolation decoration for muon container"};
  SG::ReadDecorHandleKeyArray<xAOD::ElectronContainer> m_isoElectronContainerKey
   {this, "IsoElectronVariableNames", {"Electrons.ptcone20"}, "Isolation decoration for electron container"};
};
#endif
