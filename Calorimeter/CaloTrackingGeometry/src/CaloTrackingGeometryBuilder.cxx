/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CaloTrackingGeometryBuilder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Calo
#include "CaloTrackingGeometry/CaloTrackingGeometryBuilder.h"
// Trk
#include <memory>

#include "CaloDetDescrUtils/CaloDetDescrBuilder.h"
// constructor
Calo::CaloTrackingGeometryBuilder::CaloTrackingGeometryBuilder(
    const std::string& t, const std::string& n, const IInterface* p)
    : Calo::CaloTrackingGeometryBuilderImpl(t, n, p) {
  declareInterface<Trk::IGeometryBuilder>(this);
}

// Athena standard methods
// initialize
StatusCode Calo::CaloTrackingGeometryBuilder::initialize() {
  return Calo::CaloTrackingGeometryBuilderImpl::initialize();
}

// finalize
StatusCode Calo::CaloTrackingGeometryBuilder::finalize() {
  return Calo::CaloTrackingGeometryBuilderImpl::finalize();
}

Trk::TrackingGeometry* Calo::CaloTrackingGeometryBuilder::trackingGeometry(
    Trk::TrackingVolume* innerVol) const {

  const CaloDetDescrManager* caloDDM =
      detStore()->tryConstRetrieve<CaloDetDescrManager>(caloMgrStaticKey);
  if (!caloDDM) {
    std::unique_ptr<CaloDetDescrManager> caloMgrPtr =
        buildCaloDetDescrNoAlign(serviceLocator(), Athena::getMessageSvc());
    if (detStore()->record(std::move(caloMgrPtr), caloMgrStaticKey) !=
        StatusCode::SUCCESS) {
      ATH_MSG_WARNING("Failed to record CaloDetDescrManager with the key "
                      << caloMgrStaticKey << " in DetStore");
      return nullptr;
    }
    if (detStore()->retrieve(caloDDM, caloMgrStaticKey) !=
        StatusCode::SUCCESS) {
      ATH_MSG_WARNING("Failed to retrieve CaloDetDescrManager with the key "
                      << caloMgrStaticKey << " from DetStore");
      return nullptr;
    }
  }
  // if caloDD is still null, we;re in trouble because it gets dereferenced
  // after this
  if (!caloDDM) {
    ATH_MSG_WARNING("caloDDM is a null pointer in CaloTrackingGeometryBuilder");
    return nullptr;
  }
  return Calo::CaloTrackingGeometryBuilderImpl::createTrackingGeometry(innerVol,
                                                                       caloDDM)
      .release();
}

