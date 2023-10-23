/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <AthContainers/ConstDataVector.h>

#include <DerivationFrameworkMuons/TrackIsolationDecorAlg.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODBase/IParticleHelpers.h>
#include <xAODTracking/TrackParticleContainer.h>

//**********************************************************************

#include <StoreGate/WriteDecorHandle.h>
#include <MuonDetDescrUtils/MuonSectorMapping.h>

namespace DerivationFramework {
TrackIsolationDecorAlg::TrackIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

TrackIsolationDecorAlg::~TrackIsolationDecorAlg() = default;

StatusCode TrackIsolationDecorAlg::initialize() {
    m_trk_corr.trackbitset.set(static_cast<unsigned int>(xAOD::Iso::coreTrackPtr));
    ATH_CHECK(m_idTrkKey.initialize());
    ATH_CHECK(m_toDeorTrkKey.initialize());
    ATH_CHECK(m_vtx_key.initialize());
    m_ptcone20_key = std::string("ptcone20") + (m_customName.empty() ? "" : "_") + m_customName;
    m_ptcone30_key = std::string("ptcone30") + (m_customName.empty() ? "" : "_") + m_customName;
    m_ptcone40_key = std::string("ptcone40") + (m_customName.empty() ? "" : "_") + m_customName;

    m_ptvarcone20_key = std::string("ptvarcone20") + (m_customName.empty() ? "" : "_") + m_customName;
    m_ptvarcone30_key = std::string("ptvarcone30") + (m_customName.empty() ? "" : "_") + m_customName;
    m_ptvarcone40_key = std::string("ptvarcone40") + (m_customName.empty() ? "" : "_") + m_customName;

    ATH_CHECK(m_ptcone20_key.initialize());
    ATH_CHECK(m_ptcone30_key.initialize());
    ATH_CHECK(m_ptcone40_key.initialize());

    ATH_CHECK(m_ptvarcone20_key.initialize());
    ATH_CHECK(m_ptvarcone30_key.initialize());
    ATH_CHECK(m_ptvarcone40_key.initialize());
    ATH_CHECK(m_track_iso_tool.retrieve());
    ATH_CHECK(m_trkSelKeys.initialize());

    return StatusCode::SUCCESS;
}
bool TrackIsolationDecorAlg::isSame(const xAOD::IParticle* P, const xAOD::IParticle* P1) {
    if (P == P1) { return true; }
    const xAOD::IParticle* OrigP1 = xAOD::getOriginalObject(*P1);
    const xAOD::IParticle* OrigP = xAOD::getOriginalObject(*P);
    if (OrigP == OrigP1) { return OrigP != nullptr; }
    return (OrigP == P1 || OrigP1 == P);

}
StatusCode TrackIsolationDecorAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::TrackParticleContainer> idTracks{m_idTrkKey, ctx};
    if (!idTracks.isPresent()) {
        ATH_MSG_FATAL("Failed to load "<<m_idTrkKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_toDeorTrkKey, ctx};
    if (!tracks.isPresent()) {
        ATH_MSG_FATAL("Failed to retrieve track collection " << m_toDeorTrkKey.fullKey());
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::VertexContainer> vertices{m_vtx_key, ctx};
    if (!vertices.isPresent()) {
        ATH_MSG_FATAL("Failed to retrive vertex collection " << m_vtx_key.fullKey());
        return StatusCode::FAILURE;
    }
    if (vertices->empty()) return StatusCode::SUCCESS;
    Muon::MuonSectorMapping sector_mapping{};
    
    using IsoDecorator = SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>;
    using SelDecorator = SG::ReadDecorHandle<xAOD::TrackParticleContainer, bool>;
    using TrkViewContainer = ConstDataVector<xAOD::TrackParticleContainer>;
    using view_map = std::map<int, std::vector<const xAOD::TrackParticle*> >;
    view_map track_sectors;

    for (const xAOD::TrackParticle* trk : *idTracks) {
        if (!trk) continue;
        const int sec = sector_mapping.getSector(trk->phi());
        std::vector<const xAOD::TrackParticle*>& container = track_sectors[sec];
        if (container.empty()) container.reserve(idTracks->size());
        container.push_back(trk);
    }

    IsoDecorator decor_ptcone20{m_ptcone20_key, ctx};
    IsoDecorator decor_ptcone30{m_ptcone30_key, ctx};
    IsoDecorator decor_ptcone40{m_ptcone40_key, ctx};

    IsoDecorator decor_ptvarcone20{m_ptvarcone20_key, ctx};
    IsoDecorator decor_ptvarcone30{m_ptvarcone30_key, ctx};
    IsoDecorator decor_ptvarcone40{m_ptvarcone40_key, ctx};

    std::vector<SelDecorator> selDecors;
    for (const SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>& key : m_trkSelKeys){
        selDecors.emplace_back(key, ctx);
    }
    for (const xAOD::TrackParticle* trk : *tracks) {
        if (!trk) continue;
        if (!selDecors.empty() && std::find_if(selDecors.begin(), selDecors.end(), [trk](const SelDecorator& dec){
                return dec(*trk);
        }) == selDecors.end()) continue;
        std::vector<int> sectors;
        sector_mapping.getSectors(trk->phi(), sectors);
        TrkViewContainer iso_tracks{SG::VIEW_ELEMENTS};
        iso_tracks.reserve(tracks->size());
        for (const int sector : sectors) {
            view_map::iterator itr = track_sectors.find(sector);
            if (itr == track_sectors.end()) continue;
            for (const xAOD::TrackParticle* to_copy : itr->second) { 
                if(!isSame(trk, to_copy)) iso_tracks.push_back(to_copy); 
            }
        }
        xAOD::TrackIsolation result;
        if (trk->pt() < m_pt_min) continue;
        if (!m_track_iso_tool->trackIsolation(result, *trk, m_trk_iso_types, m_trk_corr, nullptr, nullptr, iso_tracks.asDataVector())) {
            ATH_MSG_WARNING("Unable to decorate track isolation!!");
        }
        decor_ptcone40(*trk) = result.ptcones[0];
        decor_ptcone30(*trk) = result.ptcones[1];
        decor_ptcone20(*trk) = result.ptcones[2];

        decor_ptvarcone40(*trk) = result.ptvarcones_10GeVDivPt[0];
        decor_ptvarcone30(*trk) = result.ptvarcones_10GeVDivPt[1];
        decor_ptvarcone20(*trk) = result.ptvarcones_10GeVDivPt[2];
    }
    return StatusCode::SUCCESS;
}
}