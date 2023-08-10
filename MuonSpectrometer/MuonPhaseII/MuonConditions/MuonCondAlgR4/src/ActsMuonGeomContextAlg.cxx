/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsMuonGeomContextAlg.h"

#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>


ActsMuonGeomContextAlg::ActsMuonGeomContextAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator} {}


StatusCode ActsMuonGeomContextAlg::initialize(){
    ATH_CHECK(m_readKeys.initialize());
    ATH_CHECK(m_writeKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode ActsMuonGeomContextAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<ActsGeometryContext> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()){
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(IOVInfiniteRange::infiniteTime());
    std::unique_ptr<ActsGeometryContext> writeCdo = std::make_unique<ActsGeometryContext>();
    for (const  SG::ReadCondHandleKey<ActsTrk::RawGeomAlignStore>& key : m_readKeys) {
        SG::ReadCondHandle<ActsTrk::RawGeomAlignStore> readHandle{key, ctx};
        if(!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve alignment key "<<readHandle.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        writeCdo->alignmentStores[readHandle->detType] = readHandle->trackingAlignment;
    }
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}