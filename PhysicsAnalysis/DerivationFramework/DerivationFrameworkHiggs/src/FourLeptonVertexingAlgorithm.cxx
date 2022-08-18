/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "DerivationFrameworkHiggs/FourLeptonVertexingAlgorithm.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/VertexAuxContainer.h"

namespace {
    using LeptonQuadruplet = DerivationFramework::FourLeptonVertexingAlgorithm::LeptonQuadruplet;
    using TrkLink = ElementLink<xAOD::TrackParticleContainer>;

    constexpr double MeVtoGeV = 1. / Gaudi::Units::GeV;

    std::ostream& operator<<(std::ostream& sstr, const xAOD::IParticle* part) {
        if (part->type() == xAOD::Type::ObjectType::Muon) {
            const xAOD::Muon* mu = static_cast<const xAOD::Muon*>(part);
            sstr << " Muon pT: " << mu->pt() * MeVtoGeV << " [GeV], eta: " << mu->eta() << ", phi: " << mu->phi() << ", q: " << mu->charge()
                 << ", priamry Author: " << mu->author();
        } else if (part->type() == xAOD::Type::ObjectType::Electron) {
            const xAOD::Electron* elec = static_cast<const xAOD::Electron*>(part);
            sstr << " Electron pT: " << elec->pt() * MeVtoGeV << " [GeV], eta: " << elec->eta() << ", phi: " << elec->phi()
                 << ", q: " << elec->charge();
        } else if (part->type() == xAOD::Type::ObjectType::TrackParticle) {
            const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(part);
            sstr << " Track pT: " << trk->pt() * MeVtoGeV << " [GeV], eta: " << trk->eta() << ", phi: " << trk->phi()
                 << ", q: " << trk->charge() << ", d0: " << trk->d0() << ", z0: " << trk->z0();
        }
        sstr << " index: " << part->index();
        return sstr;
    }

    std::ostream& operator<<(std::ostream& sstr, const LeptonQuadruplet& quad) {
        sstr << std::endl;
        for (const xAOD::IParticle* lep : quad) { sstr << " **** " << lep << std::endl; }
        return sstr;
    }
}  // namespace

namespace DerivationFramework {

    FourLeptonVertexingAlgorithm::FourLeptonVertexingAlgorithm(const std::string& n, ISvcLocator* p) : AthReentrantAlgorithm(n, p) {}

    StatusCode FourLeptonVertexingAlgorithm::initialize() {
        ATH_CHECK(m_muonKey.initialize());
        ATH_CHECK(m_elecKey.initialize());
        ATH_CHECK(m_evtKey.initialize());
        ATH_CHECK(m_vtxKey.initialize());
        ATH_CHECK(m_fitter.retrieve());

        if (!m_elecSelTool.empty()) ATH_CHECK(m_elecSelTool.retrieve());
        if (!m_muonSelTool.empty()) ATH_CHECK(m_muonSelTool.retrieve());

        if (m_muonTrkProp < MuonTrk::Primary || m_muonTrk > MuonTrk::MSOnlyExtrapolatedMuonSpectrometerTrackParticle) {
            ATH_MSG_FATAL("A bogous muon track particle has been picked " << m_muonTrkProp);
            return StatusCode::FAILURE;
        }
        m_muonTrk = static_cast<MuonTrk>(m_muonTrkProp.value());
        return StatusCode::SUCCESS;
    }
    StatusCode FourLeptonVertexingAlgorithm::execute(const EventContext& ctx) const {
        /// Setup the output container
        SG::WriteHandle<xAOD::VertexContainer> vtxContainer{m_vtxKey, ctx};
        ATH_CHECK(vtxContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
        xAOD::VertexContainer* out_container = vtxContainer.ptr();

        std::vector<LeptonQuadruplet> all_quads = buildAllQuadruplets(ctx);
        for (const LeptonQuadruplet& quad : all_quads) {
            std::unique_ptr<xAOD::Vertex> candidate = fitQuadruplet(ctx, quad);
            if (candidate) out_container->push_back(std::move(candidate));
        }
        return StatusCode::SUCCESS;
    }

    bool FourLeptonVertexingAlgorithm::passSelection(const xAOD::Electron* elec) const {
        return elec->pt() >= m_minElecPt && (m_elecSelTool.empty() || m_elecSelTool->accept(elec)) &&
               !xAOD::EgammaHelpers::getTrackParticlesVec(elec, !m_elecUseGSF).empty();
    }
    bool FourLeptonVertexingAlgorithm::passSelection(const xAOD::Muon* muon) const {
        return muon->pt() >= m_minMuonPt && (m_muonSelTool.empty() || m_muonSelTool->accept(*muon)) && muon->trackParticle(m_muonTrk);
    }
    std::vector<LeptonQuadruplet> FourLeptonVertexingAlgorithm::buildAllQuadruplets(const EventContext& ctx) const {
        std::vector<LeptonQuadruplet> to_ret{};

        /// Retrieve the container from the store gate
        SG::ReadHandle<xAOD::MuonContainer> muonContainer{m_muonKey, ctx};
        if (!muonContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the muon container for input " << m_muonKey.fullKey());
            throw std::runtime_error("Invalid key access");
        }
        SG::ReadHandle<xAOD::ElectronContainer> elecContainer{m_elecKey, ctx};
        if (!elecContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the electron container for input " << m_elecKey.fullKey());
            throw std::runtime_error("Invalid key access");
        }
        /// Dump the leptons in a common vector to build all possible quadruplets
        using PartVec = std::vector<const xAOD::IParticle*>;
        PartVec selected_lep{};
        const size_t num_lep = elecContainer->size() + muonContainer->size();
        if (num_lep < 4) {
            ATH_MSG_DEBUG("Less than four objects reconstructed");
            return to_ret;
        }
        selected_lep.reserve(num_lep);

        for (const xAOD::Muon* muon : *muonContainer) {
            if (passSelection(muon)) {
                ATH_MSG_DEBUG("Add " << muon);
                selected_lep.push_back(muon);
            }
        }
        for (const xAOD::Electron* elec : *elecContainer) {
            if (passSelection(elec)) {
                ATH_MSG_DEBUG("Add " << elec);
                selected_lep.push_back(elec);
            }
        }

        if (selected_lep.size() < 4) {
            ATH_MSG_DEBUG("Less than four leptons survived.");
            return to_ret;
        }
        for (PartVec::const_iterator itr1 = selected_lep.begin() + 3; itr1 != selected_lep.end(); ++itr1) {
            for (PartVec::const_iterator itr2 = selected_lep.begin() + 2; itr2 != itr1; ++itr2) {
                for (PartVec::const_iterator itr3 = selected_lep.begin() + 1; itr3 != itr2; ++itr3) {
                    for (PartVec::const_iterator itr4 = selected_lep.begin(); itr4 != itr3; ++itr4) {
                        LeptonQuadruplet quad{*itr1, *itr2, *itr3, *itr4};
                        std::sort(quad.begin(), quad.end(), [](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                            if (a->type() != b->type()) return a->type() == xAOD::Type::ObjectType::Muon;
                            return a->pt() > b->pt();
                        });
                        ATH_MSG_DEBUG("Create new quaduplet " << quad);
                        to_ret.push_back(std::move(quad));
                    }
                }
            }
        }
        return to_ret;
    }
    const xAOD::TrackParticle* FourLeptonVertexingAlgorithm::trackParticle(const xAOD::IParticle* part) const {
        if (part->type() == xAOD::Type::ObjectType::Muon)
            return static_cast<const xAOD::Muon*>(part)->trackParticle(m_muonTrk);
        else if (part->type() == xAOD::Type::Electron)
            return xAOD::EgammaHelpers::getTrackParticlesVec(static_cast<const xAOD::Electron*>(part), !m_elecUseGSF)[0];
        return dynamic_cast<const xAOD::TrackParticle*>(part);
    }

    std::unique_ptr<xAOD::Vertex> FourLeptonVertexingAlgorithm::fitQuadruplet(const EventContext& ctx, const LeptonQuadruplet& quad) const {
        if (!passSelection(quad)) return nullptr;
        std::vector<const xAOD::TrackParticle*> trks{};
        trks.reserve(4);
        for (const xAOD::IParticle* lep : quad) { trks.push_back(trackParticle(lep)); }
        /// Use the beam spot as an initial constraint
        SG::ReadHandle<xAOD::EventInfo> evtInfo{m_evtKey, ctx};
        if (!evtInfo.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the event info " << m_evtKey.fullKey());
            throw std::runtime_error("Invalid key access");
        }

        const Amg::Vector3D beam_spot{evtInfo->beamPosX(), evtInfo->beamPosY(), evtInfo->beamPosZ()};
        std::unique_ptr<xAOD::Vertex> common_vtx = m_fitter->fit(ctx, trks, beam_spot);
        if (!common_vtx) {
            ATH_MSG_DEBUG("Fit from  " << quad << " failed.");
            return nullptr;
        }
        ATH_MSG_DEBUG("Fit from " << quad << " gave a vertex with position at " << common_vtx->position() << " with chi2 "
                                  << (common_vtx->chiSquared() / common_vtx->numberDoF()) << " nDoF: " << common_vtx->numberDoF());
        return common_vtx;
    }
    bool FourLeptonVertexingAlgorithm::passSelection(const LeptonQuadruplet& quad) const {
        for (unsigned int i = 1; i < quad.size(); ++i) {
            for (unsigned int j = 0; j < i; ++j) {
                if (std::abs(trackParticle(quad[i])->z0() - trackParticle(quad[j])->z0()) > m_z0Cut) return false;
            }
        }
        return true;
    }
}  // namespace DerivationFramework
