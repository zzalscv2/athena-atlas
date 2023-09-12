/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkMuons/DiMuonTaggingAlg.h"

#include "AthenaKernel/errorcheck.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackingPrimitives.h"

namespace {
    using MuonPassDecor = SG::WriteDecorHandle<xAOD::MuonContainer, bool>;

}
namespace DerivationFramework {
    DiMuonTaggingAlg::DiMuonTaggingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {    }

    // Destructor
    DiMuonTaggingAlg::~DiMuonTaggingAlg() = default;

    // Athena initialize and finalize
    StatusCode DiMuonTaggingAlg::initialize() {
        ATH_MSG_VERBOSE("initialize() ...");

        // trigger decision tool, needed when there is trigger requirement
        const bool has_trigs = m_orTrigs.size() || m_andTrigs.size();
        ATH_CHECK(m_trigDecisionTool.retrieve(DisableTool{!has_trigs}));
        ATH_CHECK(m_matchTool.retrieve(DisableTool{!has_trigs}));
        
        ATH_CHECK(m_evtKey.initialize());
        ATH_CHECK(m_muonSGKey.initialize());
        ATH_CHECK(m_trackSGKey.initialize(m_useTrackProbe));
        ATH_CHECK(m_truthSGKey.initialize(m_isMC && !m_truthSGKey.empty()));

        m_muonKeepKey = m_muonSGKey.key() + ".pass" + m_br_prefix.value();
        m_trkKeepKey = m_trackSGKey.key() + ".pass" + m_br_prefix.value();
        m_skimmingKey = "DIMU_pass" + m_br_prefix;
        ATH_CHECK(m_muonKeepKey.initialize());
        ATH_CHECK(m_trkKeepKey.initialize(m_useTrackProbe));
        ATH_CHECK(m_skimmingKey.initialize());

        m_invariantMassLow2 = m_invariantMassLow * std::abs(m_invariantMassLow);
        m_invariantMassHigh2 = m_invariantMassHigh * std::abs(m_invariantMassHigh);
        m_thinningConeSize2 = m_thinningConeSize * std::abs(m_thinningConeSize);

        return StatusCode::SUCCESS;
    }

    bool DiMuonTaggingAlg::checkTrigMatch(const xAOD::Muon* mu, const std::vector<std::string>& Trigs) const {
        for (const std::string& t : Trigs) {
            if (m_matchTool->match(mu, t)) return true;
        }
        return Trigs.empty();
    }

    StatusCode DiMuonTaggingAlg::execute(const EventContext& ctx) const { 
        SG::ReadHandle<xAOD::EventInfo> eventInfo{m_evtKey, ctx};
        if (!eventInfo.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_evtKey.fullKey());
            return StatusCode::FAILURE;
        }

        SG::WriteHandle<int> keepEventHandle{m_skimmingKey, ctx};
        ATH_CHECK(keepEventHandle.record(std::make_unique<int>(0)));
        int& keepEvent = *keepEventHandle;

        //// check Or triggers
        for (const std::string& or_trig : m_orTrigs) {
            if (m_trigDecisionTool->isPassed(or_trig)) {
                keepEvent = 100;
                break;
            }
        }

        //// check "and" triggers if didn't pass "Or" triggers
        if (!keepEvent && !m_andTrigs.empty()) {
            bool passAll = true;
            for (const std::string& and_trig : m_andTrigs) {
                if (!m_trigDecisionTool->isPassed(and_trig)) {
                    passAll = false;
                    break;
                }
            }
            if (passAll) keepEvent = 100;
            
            if (!keepEvent) {
              MuonPassDecor muo_decor{m_muonKeepKey, ctx};
              /// Ensure that the first object is written anyway
              if (!muo_decor->empty()) muo_decor(*muo_decor->at(0)) = false;
              if (m_trkKeepKey.empty()) return StatusCode::SUCCESS;
              TrackPassDecor trk_decor{m_trkKeepKey, ctx};
              if (!trk_decor->empty()) trk_decor(*trk_decor->at(0)) = false;
              return StatusCode::SUCCESS;
            }
        }

        /// muon selection
        SG::ReadHandle<xAOD::MuonContainer> muons{m_muonSGKey, ctx};
        if (!muons.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_muonSGKey.fullKey());
            return StatusCode::FAILURE;
        }
        MuonPassDecor muo_decor{m_muonKeepKey, ctx};
        /// Ensure that the first object is written anyway
        if (!muons->empty()) muo_decor(*muons->at(0)) = false;

        const xAOD::TruthParticleContainer* truth{nullptr};
        if (!m_truthSGKey.empty()) {
            SG::ReadHandle<xAOD::TruthParticleContainer> handle{m_truthSGKey, ctx};
            if (!handle.isValid()) {
                ATH_MSG_FATAL("Failed to retrieve truth container " << m_truthSGKey.fullKey());
                return StatusCode::FAILURE;
            }
            truth = handle.cptr();
        }      

        for (const xAOD::Muon* mu_itr1 : *muons) {
            if (truth) {
                for (const xAOD::TruthParticle* truth_itr : * truth) {
                    if (truth_itr->isMuon() && xAOD::P4Helpers::deltaR2(truth_itr, mu_itr1) < m_thinningConeSize2) {
                        muo_decor(*mu_itr1) = true;
                        break;
                   }
                }
            }
            if (!passMuonCuts(mu_itr1, m_mu1PtMin, m_mu1AbsEtaMax, m_mu1Types, m_mu1Trigs, m_mu1IsoCuts)) continue;
            for (const xAOD::Muon* mu_itr2 : *muons) {
                if (mu_itr2 == mu_itr1) continue;
                if (!passMuonCuts(mu_itr2, m_mu2PtMin, m_mu2AbsEtaMax, m_mu2Types, m_mu2Trigs, m_mu2IsoCuts)) continue;
                if (!muonPairCheck(mu_itr1, mu_itr2)) continue;
                muo_decor(*mu_itr1) = true;
                muo_decor(*mu_itr2) = true;
                ++keepEvent;
            }           
        }
        
        /// Track probe is needed to establish the T&P selection for the
        /// reco efficiency Muon + ID track -> Z or Jpsi decay
        if (m_useTrackProbe) {
            SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trackSGKey, ctx};
            if (!tracks.isValid()) {
                ATH_MSG_FATAL("Failed to retrieve " << m_trackSGKey.fullKey());
                return StatusCode::FAILURE;
            }
            TrackPassDecor trk_decor{m_trkKeepKey, ctx};
            if (!trk_decor->empty()) trk_decor(*trk_decor->at(0)) = false;
            for (const xAOD::Muon* mu_itr1 : *muons) {
                if (!passMuonCuts(mu_itr1, m_mu1PtMin, m_mu1AbsEtaMax, m_mu1Types, m_mu1Trigs, m_mu1IsoCuts)) continue;
                for (const xAOD::TrackParticle* probe_trk : *tracks) {  ///
                    if (probe_trk == mu_itr1->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle)) continue;
                    if (!passKinematicCuts(probe_trk, m_mu2PtMin, m_mu2AbsEtaMax)) continue;
                    if (!muonPairCheck(mu_itr1, probe_trk)) continue;
                    trk_decor(*probe_trk) = true;
                    ++keepEvent;
                    /// Close by tracks are written as well to ensure that we can
                    /// redo the isolation variables for our probes
                    maskNearbyIDtracks(probe_trk, trk_decor);
                }
            }
            /// also mask tracks around truth muons
            if (truth) {
                for (const xAOD::TruthParticle* mu_itr2 : *truth) {
                    if (mu_itr2->isMuon()) maskNearbyIDtracks(mu_itr2, trk_decor);
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    void DiMuonTaggingAlg::maskNearbyIDtracks(const xAOD::IParticle* ref_part, TrackPassDecor& decor) const {
        for (const xAOD::TrackParticle* trk : *decor) {
            if (xAOD::P4Helpers::deltaR2(ref_part, trk) < m_thinningConeSize2) decor(*trk) = true;
        }
    }

    bool DiMuonTaggingAlg::passKinematicCuts(const xAOD::IParticle* mu, float ptMin, float absEtaMax) const {
        return !(!mu || mu->pt() < ptMin || std::abs(mu->eta()) > absEtaMax);
    }

    template <class probe_type> bool DiMuonTaggingAlg::muonPairCheck(const xAOD::Muon* mu1, const probe_type* mu2) const {
        if (m_requireOS != (mu1->charge() * mu2->charge() < 0)) return false;
        if (m_dPhiMin > 0 && std::abs(xAOD::P4Helpers::deltaPhi(mu1, mu2)) < m_dPhiMin) return false;
        const float mass2 = (mu1->p4() + mu2->p4()).M2();
        return !(mass2 < m_invariantMassLow2 || (m_invariantMassHigh > 0. && mass2 > m_invariantMassHigh2));
    }

    bool DiMuonTaggingAlg::passMuonCuts(const xAOD::Muon* mu, const float ptMin, const float absEtaMax, const std::vector<int>& types,
                                         const std::vector<std::string>& trigs, const std::map<int, double>& muIsoCuts) const {
        if (!passMuonCuts(mu, ptMin, absEtaMax, types, trigs)) return false;
        /// isolation cuts. Mutiple cuts allowed and return the logical AND results.
        for (std::map<int, double>::const_iterator it = muIsoCuts.begin(); it != muIsoCuts.end(); ++it) {
            float isoValue = 0;
            const xAOD::Iso::IsolationType isoType = static_cast<xAOD::Iso::IsolationType>(it->first);
            if (!(mu->isolation(isoValue, isoType)) || isoValue > it->second) return false;
        }
        return true;
    }

    bool DiMuonTaggingAlg::passMuonCuts(const xAOD::Muon* mu, const float ptMin, const float absEtaMax, const std::vector<int>& types,
                                         const std::vector<std::string>& trigs) const {
        if (!passKinematicCuts(mu, ptMin, absEtaMax)) return false;
        if (!types.empty() &&
            std::find_if(types.begin(), types.end(), [mu](const int type) { return type == mu->muonType(); }) == types.end())
            return false;

        return checkTrigMatch(mu, trigs);
    }
}  // namespace DerivationFramework
