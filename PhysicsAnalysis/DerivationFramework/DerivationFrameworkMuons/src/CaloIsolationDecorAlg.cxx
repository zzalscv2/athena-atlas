/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <DerivationFrameworkMuons/CaloIsolationDecorAlg.h>

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteDecorHandle.h>

//**********************************************************************
namespace DerivationFramework {
CaloIsolationDecorAlg::CaloIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}
//**********************************************************************

StatusCode CaloIsolationDecorAlg::initialize() {
    ATH_CHECK(m_calo_iso_tool.retrieve());

    m_calo_corr.calobitset.set(static_cast<unsigned int>(xAOD::Iso::coreCone));
    m_calo_corr.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));
    // isolation types to run. The ptcones each also imply the respective ptvarcone.

    ATH_CHECK(m_trk_key.initialize());
    m_topocone20_key = std::string{"topoetcone20"} + (m_customName.empty() ? "" : "_") + m_customName;
    m_topocone30_key = std::string{"topoetcone30"} + (m_customName.empty() ? "" : "_") + m_customName;
    m_topocone40_key = std::string{"topoetcone40"} + (m_customName.empty() ? "" : "_") + m_customName;
    m_corr_key = std::string{"etcore_correction"} + (m_customName.empty() ? "" : "_") + m_customName;
    ATH_CHECK(m_topocone20_key.initialize());
    ATH_CHECK(m_topocone30_key.initialize());
    ATH_CHECK(m_topocone40_key.initialize());
    ATH_CHECK(m_corr_key.initialize());
    ATH_CHECK(m_trkSelKeys.initialize());

    ATH_MSG_DEBUG("Decorate " << m_trk_key.fullKey() << " using '" << m_customName << "' as suffix.");
    return StatusCode::SUCCESS;
}

//**********************************************************************

StatusCode CaloIsolationDecorAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trk_key, ctx};
    if (!tracks.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve track collection " << m_trk_key.fullKey());
        return StatusCode::FAILURE;
    }
    

    using FloatDecor = SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>;
    using SelDecorator = SG::ReadDecorHandle<xAOD::TrackParticleContainer, bool>;
    
    std::vector<SelDecorator> selDecors;
    for (const SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>& key : m_trkSelKeys){
        selDecors.emplace_back(key, ctx);
    }

    FloatDecor topocone40_dec{m_topocone40_key, ctx};
    FloatDecor topocone30_dec{m_topocone30_key, ctx};
    FloatDecor topocone20_dec{m_topocone20_key, ctx};
    FloatDecor corr_dec{m_corr_key, ctx};

    for (const xAOD::TrackParticle* trk : *tracks) {
        if (trk->pt() < m_pt_min) continue;
        if (!selDecors.empty() && std::find_if(selDecors.begin(), selDecors.end(), [trk](const SelDecorator& dec){
                return dec(*trk);
        }) == selDecors.end()) continue;
        ATH_MSG_DEBUG("Recomputing isolation by hand");
        xAOD::CaloIsolation resultCalo;
        if (!m_calo_iso_tool->caloTopoClusterIsolation(resultCalo, *trk, m_calo_isos, m_calo_corr)) {
            ATH_MSG_ERROR("Failed to compute calorimeter isolation");
            return StatusCode::FAILURE;
        }
        topocone40_dec(*trk) = resultCalo.etcones[0];
        topocone30_dec(*trk) = resultCalo.etcones[1];
        topocone20_dec(*trk) = resultCalo.etcones[2];
        corr_dec(*trk) = resultCalo.coreCorrections[xAOD::Iso::coreCone][xAOD::Iso::coreEnergy];
    }
    return StatusCode::SUCCESS;
}
}
//**********************************************************************
