/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkMuons/IDTrackCaloDepositsDecoratorAlg.h"

#include "CaloEvent/CaloCellContainer.h"
#include "muonEvent/DepositInCalo.h"
#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticle.h"

namespace {   
    static const SG::AuxElement::Decorator<std::vector<float>> dec_deposit("CaloDeposits");
    static const SG::AuxElement::Decorator<std::vector<float>> dec_eloss("CaloElosses");
    static const SG::AuxElement::Decorator<std::vector<uint16_t>> dec_type("CaloDepType");

}  // namespace

namespace DerivationFramework {
IDTrackCaloDepositsDecoratorAlg::IDTrackCaloDepositsDecoratorAlg(const std::string& n, ISvcLocator* p):
    AthReentrantAlgorithm(n, p) {
 }

StatusCode IDTrackCaloDepositsDecoratorAlg::initialize() {
    ATH_CHECK(m_trkDepositInCalo.retrieve());

    ATH_CHECK(m_muon_key.initialize());
    ATH_CHECK(m_id_trk_key.initialize());
    SG::AuxTypeRegistry& registry{SG::AuxTypeRegistry::instance()};
    m_deposit_key = (m_decor_muons ? m_muon_key.key() : m_id_trk_key.key()) + "."+registry.getName(dec_deposit.auxid());
    m_eloss_key = (m_decor_muons ? m_muon_key.key() : m_id_trk_key.key()) + "."+registry.getName(dec_eloss.auxid());
    m_type_key = (m_decor_muons ? m_muon_key.key() : m_id_trk_key.key()) + "."+registry.getName(dec_type.auxid());

    ATH_CHECK(m_deposit_key.initialize());
    ATH_CHECK(m_eloss_key.initialize());
    ATH_CHECK(m_type_key.initialize());

    return StatusCode::SUCCESS;
}

StatusCode IDTrackCaloDepositsDecoratorAlg::recompute_and_decorate(const xAOD::IParticle* particle) const {
    ATH_MSG_DEBUG("Recomputing calo deposition by hand");

    const xAOD::TrackParticle* track_part = nullptr;
    if (particle->type() == xAOD::Type::ObjectType::TrackParticle) {
        track_part = static_cast<const xAOD::TrackParticle*>(particle);
    } else if (particle->type() == xAOD::Type::ObjectType::Muon) {
        const xAOD::Muon* muon = static_cast<const xAOD::Muon*>(particle);
        track_part = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        if (!track_part) {
            ATH_MSG_VERBOSE("The muon does not have an associated ID track");
            track_part = muon->trackParticle(xAOD::Muon::Primary);
        }
    }
    if (!track_part) {
        ATH_MSG_ERROR("Unable to retrieve xAOD::TrackParticle from probe object");
        return StatusCode::FAILURE;
    }

    const CaloCellContainer* caloCellCont = nullptr;
    std::vector<DepositInCalo> deposits = m_trkDepositInCalo->getDeposits(&(track_part->perigeeParameters()), caloCellCont);
    std::vector<float>& dep_val{dec_deposit(*particle)};
    std::vector<float>& eloss_val{dec_eloss(*particle)};
    std::vector<uint16_t>& types{dec_type(*particle)};
    dep_val.reserve(deposits.size());
    eloss_val.reserve(deposits.size());
    types.reserve(deposits.size());
    for (const DepositInCalo& it : deposits) {
        dep_val.push_back(it.energyDeposited());
        eloss_val.push_back(it.muonEnergyLoss());
        types.push_back(it.subCaloId());
    }
    return StatusCode::SUCCESS;
}

StatusCode IDTrackCaloDepositsDecoratorAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::MuonContainer> muons{m_muon_key, ctx};
    if (!muons.isValid()) {
        ATH_MSG_FATAL("Failed to load muon container " << m_muon_key.fullKey());
        return StatusCode::FAILURE;
    }
    bool added_one_muon{false};
    for (const xAOD::Muon* muon : *muons) {
        if (m_decor_muons) {
            ATH_CHECK(recompute_and_decorate(muon));
            continue;
        }
        if ((muon->muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) != m_use_SAF) {
            ATH_MSG_DEBUG("The muon is a SAF muon. Discard them, while the tool is not setup to decorate the SAF particles");
            continue;
        }
        const xAOD::TrackParticle* track_part = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        if (!track_part) {
            ATH_MSG_VERBOSE("The muon does not have an ID track ");
            continue;
        }
        added_one_muon = true;
        ATH_CHECK(recompute_and_decorate(track_part));
    }
    /// We do not need to do anything here
    if (m_decor_muons || added_one_muon) return StatusCode::SUCCESS;

    SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_id_trk_key, ctx};
    if (!tracks.isValid()) {
        ATH_MSG_FATAL("Failed to load ID track container " << m_id_trk_key.fullKey());
        return StatusCode::FAILURE;
    }
    if (tracks->empty()) return StatusCode::SUCCESS;

    ATH_CHECK(recompute_and_decorate(tracks->front()));
    return StatusCode::SUCCESS;
}
}