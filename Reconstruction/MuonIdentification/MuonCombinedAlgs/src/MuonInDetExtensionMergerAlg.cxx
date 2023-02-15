/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonInDetExtensionMergerAlg.h"
#include "AthContainers/ConstDataVector.h"
MuonInDetExtensionMergerAlg::MuonInDetExtensionMergerAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonInDetExtensionMergerAlg::initialize() {    
    ATH_CHECK(m_inputCandidates.initialize());
    ATH_CHECK(m_writeKey.initialize());       
    return StatusCode::SUCCESS;
}

StatusCode MuonInDetExtensionMergerAlg::execute(const EventContext& ctx) const {
    ConstDataVector<InDetCandidateCollection> merged{SG::VIEW_ELEMENTS};
    for (const SG::ReadHandleKey<InDetCandidateCollection>& key : m_inputCandidates) {
        SG::ReadHandle<InDetCandidateCollection> readHandle{key, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve InDetCandidates from storegate "<<key.fullKey());
            return StatusCode::FAILURE;
        }
        merged.insert(merged.end(), readHandle->begin(), readHandle->end());
    }
    std::unique_ptr<InDetCandidateCollection> output_coll = std::make_unique<InDetCandidateCollection>(*merged.asDataVector());
    SG::WriteHandle<InDetCandidateCollection> writeHandle{m_writeKey, ctx};
    ATH_CHECK(writeHandle.record(std::move(output_coll)));
    return StatusCode::SUCCESS;
}