/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_RpcSENSITIVEDETECTORTOOL_H
#define MUONG4R4_RpcSENSITIVEDETECTORTOOL_H

#include <GeoPrimitives/GeoPrimitives.h>

#include <G4AtlasTools/SensitiveDetectorBase.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>

namespace MuonG4R4 {

class RpcSensitiveDetectorTool : public SensitiveDetectorBase {

public:
    RpcSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
    ~RpcSensitiveDetectorTool()=default;

    StatusCode initialize() override final;
protected:
    G4VSensitiveDetector* makeSD() const override final;
private:
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
   
};
}

#endif
