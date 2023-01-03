/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonByteStream/sTgcPadTriggerRawDataProvider.h"

Muon::sTgcPadTriggerRawDataProvider::sTgcPadTriggerRawDataProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode Muon::sTgcPadTriggerRawDataProvider::initialize() {
    ATH_MSG_INFO("sTgcPadTriggerRawDataProvider::initialize");
    ATH_CHECK(m_rawDataTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode Muon::sTgcPadTriggerRawDataProvider::execute(const EventContext& ctx) const {
    ATH_MSG_INFO("sTgcPadTriggerRawDataProvider::execute");
    if (!m_rawDataTool->convert(ctx).isSuccess()) {
      ATH_MSG_ERROR("STGC Pad Trigger BS conversion into RDOs failed");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
