/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CaloTrackingGeometryBuilderCond.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Calo
#include "CaloTrackingGeometry/CaloTrackingGeometryBuilderCond.h"

#include <memory>

#include "StoreGate/ReadCondHandle.h"

// constructor
Calo::CaloTrackingGeometryBuilderCond::CaloTrackingGeometryBuilderCond(
    const std::string& t, const std::string& n, const IInterface* p)
    : Calo::CaloTrackingGeometryBuilderImpl(t, n, p) {
  declareInterface<Trk::IGeometryBuilderCond>(this);
}

// initialize
StatusCode Calo::CaloTrackingGeometryBuilderCond::initialize() {

  ATH_CHECK(m_caloMgrKey.initialize());
  return Calo::CaloTrackingGeometryBuilderImpl::initialize();
}

std::unique_ptr<Trk::TrackingGeometry>
Calo::CaloTrackingGeometryBuilderCond::trackingGeometry(
    const EventContext& ctx, Trk::TrackingVolume* innerVol,
    SG::WriteCondHandle<Trk::TrackingGeometry>& /*whandle*/) const {

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
  const CaloDetDescrManager* caloDDM = *caloMgrHandle;
  return Calo::CaloTrackingGeometryBuilderImpl::createTrackingGeometry(innerVol,
                                                                       caloDDM);
}

