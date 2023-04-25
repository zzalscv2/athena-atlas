/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsDetAlignCondAlg.h"

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"

using namespace ActsTrk;
ActsDetAlignCondAlg::ActsDetAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

ActsDetAlignCondAlg::~ActsDetAlignCondAlg() = default;

StatusCode ActsDetAlignCondAlg::initialize() {
    ATH_CHECK(m_inputKey.initialize());
    ATH_CHECK(m_outputKey.initialize());
    ATH_CHECK(m_trackingGeoSvc.retrieve());
    try {
        m_Type = static_cast<DetectorType>(m_detType.value());
    } catch (const std::exception& what) {
        ATH_MSG_FATAL("Invalid detType is configured " << m_detType);
        return StatusCode::FAILURE;
    }
    if (m_Type == DetectorType::UnDefined) {
        ATH_MSG_FATAL("Please configure the deType " << m_detType << " to be something not undefined");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode ActsDetAlignCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<RawGeomAlignStore> writeHandle{m_outputKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("Nothing needs to be done for " << ctx.eventID().event_number());
        return StatusCode::SUCCESS;
    }
    SG::ReadCondHandle<GeoAlignmentStore> readHandle{m_inputKey, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve " << m_inputKey.fullKey());
        return StatusCode::FAILURE;
    }
    writeHandle.addDependency(readHandle);
    /// Create the new alignment
    std::unique_ptr<RawGeomAlignStore> newAlignment = std::make_unique<RawGeomAlignStore>();
    /// We need to copy over the cache from the GeoModel alignment store
    newAlignment->geoModelAlignment->append(**readHandle);
    newAlignment->detType = m_Type;
    /// Process using the tracking geometry
    if (!m_trackingGeoSvc->populateAlignmentStore(*newAlignment)) {
        ATH_MSG_WARNING("No detector elements of " << to_string(m_Type) << " are part of the tracking geometry");
    }
    if (m_whipeGeoStore) newAlignment->geoModelAlignment.reset();
    ATH_CHECK(writeHandle.record(std::move(newAlignment)));
    return StatusCode::SUCCESS;
}
