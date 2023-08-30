/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_DIMUONTAGGINGTOOL_H
#define DERIVATIONFRAMEWORK_DIMUONTAGGINGTOOL_H

#include <map>
#include <string>
#include <vector>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigMuonMatching/ITrigMuonMatching.h"

// xAOD header files
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
namespace DerivationFramework {

    class DiMuonTaggingAlg : public AthReentrantAlgorithm {
    public:
        /** Constructor with parameters */
        DiMuonTaggingAlg(const std::string& name, ISvcLocator* pSvcLocator);

        /** Destructor */
        virtual ~DiMuonTaggingAlg();
        // Athena algtool's Hooks
        StatusCode initialize() override;

        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        using TrackPassDecor = SG::WriteDecorHandle<xAOD::TrackParticleContainer, bool>;
        /// Returns true of the pointer is valid and also whether the pt and absEta are above and below the thresholds, respectively
        bool passKinematicCuts(const xAOD::IParticle* mu, float ptMin, float absEtaMax) const;

        /// The muon has to satisfy the kinematic requirements to be of a certain type (I.e. combined) and matched to at least on trigger
        bool passMuonCuts(const xAOD::Muon* mu, const float ptMin, const float absEtaMax, const std::vector<int>& types,
                          const std::vector<std::string>& trigs) const;

        /// The muon has to satisfy the pevious muon cuts + cuts on each isolation variable.
        bool passMuonCuts(const xAOD::Muon* mu, const float ptMin, const float absEtaMax, const std::vector<int>& types,
                          const std::vector<std::string>& trigs, const std::map<int, double>& muIsoCuts) const;

        bool checkTrigMatch(const xAOD::Muon* mu, const std::vector<std::string>& Trigs) const;

        template <class probe_type> bool muonPairCheck(const xAOD::Muon* mu1, const probe_type* mu2) const;

        void maskNearbyIDtracks(const xAOD::IParticle* mu, TrackPassDecor& decor) const;

        PublicToolHandle<Trig::ITrigDecisionTool> m_trigDecisionTool{this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool",
                                                               "Tool to access the trigger decision"};

        Gaudi::Property<std::vector<std::string>> m_orTrigs{this, "OrTrigs", {}};
        Gaudi::Property<std::vector<std::string>> m_andTrigs{this, "AndTrigs", {}};

        /// Do we need to change this to the ordinary matching tool?
        ToolHandle<Trig::ITrigMuonMatching> m_matchTool{this, "TrigMatchTool", "Trig::TrigMuonMatching/TrigMuonMatching"};
        Gaudi::Property<float> m_triggerMatchDeltaR{this, "TriggerMatchDeltaR", 0.1};

        SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EventInfoKey", "EventInfo", ""};

        SG::ReadHandleKey<xAOD::MuonContainer> m_muonSGKey{this, "MuonContainerKey", "Muons"};

        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackSGKey{this, "TrackContainerKey", "InDetTrackParticles"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthSGKey{this, "TruthKey", ""};

        /// Keys to whitelist the muons & tracks needed for MCP studies to output
        SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonKeepKey{
            this, "MuonKeepKey", "Key to whitelist the muon for writeout. Will be overwritten by BranchPrefix property"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trkKeepKey{
            this, "TrackKeepKey", "Key to whitelist the tracks for writeout. Will be overwritten by BranchPreFix property"};
        /// Event Decision Key
        SG::WriteHandleKey<int> m_skimmingKey{this, "SkimmingKey", "", "Set via BranchPreFixProperty + DIMU_pass"};

        Gaudi::Property<float> m_mu1PtMin{this, "Mu1PtMin", -1};
        Gaudi::Property<float> m_mu1AbsEtaMax{this, "Mu1AbsEtaMax", 10};
        Gaudi::Property<std::vector<std::string>> m_mu1Trigs{this, "Mu1Trigs", {}};
        Gaudi::Property<std::vector<int>> m_mu1Types{this, "Mu1Types", {}};
        Gaudi::Property<std::map<int, double>> m_mu1IsoCuts{this, "Mu1IsoCuts", {}};

        Gaudi::Property<float> m_mu2PtMin{this, "Mu2PtMin", -1};
        Gaudi::Property<float> m_mu2AbsEtaMax{this, "Mu2AbsEtaMax", 10};

        Gaudi::Property<std::vector<std::string>> m_mu2Trigs{this, "Mu2Trigs", {}};
        Gaudi::Property<std::vector<int>> m_mu2Types{this, "Mu2Types", {}};
        Gaudi::Property<std::map<int, double>> m_mu2IsoCuts{this, "Mu2IsoCuts", {}};

        Gaudi::Property<bool> m_isMC{this, "isMC", false};
        Gaudi::Property<bool> m_useTrackProbe{this, "UseTrackProbe", true};

        Gaudi::Property<std::string> m_br_prefix{this, "BranchPrefix", ""};

        Gaudi::Property<bool> m_requireOS{this, "OppositeCharge", true};
        Gaudi::Property<float> m_dPhiMin{this, "PairDPhiMin", -1};

        Gaudi::Property<float> m_invariantMassLow{this, "InvariantMassLow", 2.0 * Gaudi::Units::GeV};
        Gaudi::Property<float> m_invariantMassHigh{this, "InvariantMassHigh", 4.3 * Gaudi::Units::GeV};

        Gaudi::Property<float> m_thinningConeSize{this, "IDTrackThinningConeSize", 0.4};

        float m_invariantMassLow2{0};
        float m_invariantMassHigh2{0};
        float m_thinningConeSize2{0};
    };
}  // namespace DerivationFramework
#endif  // DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H
