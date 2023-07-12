/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MDTSensitiveDetectorTool.h"
#include "MDTSensitiveDetector.h"


namespace MuonG4R4 {

MDTSensitiveDetectorTool::MDTSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ) {}

StatusCode MDTSensitiveDetectorTool::initialize() {
    ATH_CHECK(SensitiveDetectorBase::initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
G4VSensitiveDetector* MDTSensitiveDetectorTool::makeSD() const {
  return new MDTSensitiveDetector(name(), m_outputCollectionNames[0], m_detMgr);
}
}
