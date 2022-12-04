/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCombinedInDetExtensionAlg.h"

#include "MuonCombinedEvent/MuonCandidateCollection.h"

MuonCombinedInDetExtensionAlg::MuonCombinedInDetExtensionAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonCombinedInDetExtensionAlg::initialize() {
    ATH_MSG_VERBOSE(" usePRDs = " << m_usePRDs);
    ATH_CHECK(m_muonCombinedInDetExtensionTool.retrieve());
    ATH_CHECK(m_indetCandidateCollectionName.initialize());
    ATH_CHECK(m_MDT_ContainerName.initialize(m_usePRDs));
    ATH_CHECK(m_RPC_ContainerName.initialize(m_usePRDs));
    ATH_CHECK(m_TGC_ContainerName.initialize(m_usePRDs));
    ATH_CHECK(m_CSC_ContainerName.initialize(m_usePRDs && m_hasCSC));
    ATH_CHECK(m_sTGC_ContainerName.initialize(m_usePRDs && m_hasSTGC));
    ATH_CHECK(m_MM_ContainerName.initialize(m_usePRDs && m_hasMM));
    ATH_CHECK(m_tagMap.initialize());
    ATH_CHECK(m_combTracks.initialize(!m_combTracks.key().empty()));
    ATH_CHECK(m_METracks.initialize(!m_METracks.key().empty()));
    ATH_CHECK(m_segments.initialize(!m_segments.key().empty()));

    return StatusCode::SUCCESS;
}

StatusCode MuonCombinedInDetExtensionAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<InDetCandidateCollection> indetCandidateCollection(m_indetCandidateCollectionName, ctx);
    if (!indetCandidateCollection.isValid()) {
        ATH_MSG_ERROR("Could not read " << m_indetCandidateCollectionName);
        return StatusCode::FAILURE;
    }    
    ATH_MSG_VERBOSE("Loaded InDetCandidateCollection " << m_indetCandidateCollectionName << " with  " << indetCandidateCollection->size()
                                                       << " elements.");
    if (msgLvl(MSG::VERBOSE)) {
        for (const MuonCombined::InDetCandidate* candidate : *indetCandidateCollection){
            ATH_MSG_VERBOSE(candidate->toString());          
        }
    }


    MuonCombined::InDetCandidateToTagMap* tagMap{nullptr};
    TrackCollection* combTracks{nullptr}, *meTracks{nullptr};
    Trk::SegmentCollection* segments{nullptr};
    ATH_CHECK(record(ctx, m_tagMap, tagMap));
    ATH_CHECK(record(ctx,m_combTracks, combTracks ));
    ATH_CHECK(record(ctx,m_METracks, meTracks ));
    ATH_CHECK(record(ctx,m_segments, segments ));

    if (m_usePRDs) {
        MuonCombined::IMuonCombinedInDetExtensionTool::MuonPrdData prdData{};
        ATH_CHECK(loadPrdContainer(ctx, m_MDT_ContainerName, prdData.mdtPrds));
        ATH_CHECK(loadPrdContainer(ctx, m_CSC_ContainerName, prdData.cscPrds));
        ATH_CHECK(loadPrdContainer(ctx, m_sTGC_ContainerName, prdData.stgcPrds));
        ATH_CHECK(loadPrdContainer(ctx, m_MM_ContainerName, prdData.mmPrds));
        ATH_CHECK(loadPrdContainer(ctx, m_RPC_ContainerName, prdData.rpcPrds));
        ATH_CHECK(loadPrdContainer(ctx, m_TGC_ContainerName, prdData.tgcPrds));
        m_muonCombinedInDetExtensionTool->extendWithPRDs(*indetCandidateCollection, tagMap, prdData, combTracks, meTracks, segments, ctx);
        
    } else {
        m_muonCombinedInDetExtensionTool->extend(*indetCandidateCollection, tagMap, combTracks, meTracks, segments, ctx);
    }
    return StatusCode::SUCCESS;
}
template <class ContType> StatusCode MuonCombinedInDetExtensionAlg::loadPrdContainer(const EventContext& ctx , const SG::ReadHandleKey<ContType>& key, const ContType* & target_ptr) const{
    if (key.empty()) {
        ATH_MSG_DEBUG("loadPrdContainer() -- No key given assume it's intended");
        target_ptr = nullptr;
        return StatusCode::SUCCESS;    
    }
    SG::ReadHandle<ContType> readHandle{key,ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to load "<<key.fullKey());
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Loaded successfully "<<key.fullKey());
    target_ptr = readHandle.cptr();
    return StatusCode::SUCCESS;
}
template <class ContType> StatusCode MuonCombinedInDetExtensionAlg::record(const EventContext& ctx, const SG::WriteHandleKey<ContType>& key, ContType* & target_ptr) const {
    if (key.empty()) {
        ATH_MSG_VERBOSE("record() -- No key was given... Assume it's intended ");
        target_ptr = nullptr;
        return StatusCode::SUCCESS;
    }
    SG::WriteHandle<ContType> writeHandle{key, ctx};
    ATH_CHECK(writeHandle.record(std::make_unique<ContType>()));
    target_ptr = writeHandle.ptr();
    ATH_MSG_VERBOSE("record() -- Successfully written "<<key.fullKey());
    return StatusCode::SUCCESS;
}
    
