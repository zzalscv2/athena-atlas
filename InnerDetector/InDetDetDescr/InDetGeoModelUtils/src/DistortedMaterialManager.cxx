/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */


#include "InDetGeoModelUtils/DistortedMaterialManager.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"

namespace InDetDD {
  DistortedMaterialManager::DistortedMaterialManager() {
    ISvcLocator* svcLocator = Gaudi::svcLocator(); // from Bootstrap

    MsgStream log(Athena::getMessageSvc(), "ExtraMaterialManager");
    log << MSG::DEBUG << "Initialized InDet Distorted Material Manager" << endmsg;

    StoreGateSvc* detStore{nullptr};
    StatusCode sc = svcLocator->service("DetectorStore", detStore);
    if (sc.isFailure()) log << MSG::FATAL << "Could not locate DetectorStore" << endmsg;

    IGeoDbTagSvc* geoDbTag{nullptr};
    sc = svcLocator->service("GeoDbTagSvc",geoDbTag);
    if (sc.isFailure()) log << MSG::FATAL << "Could not locate GeoDbTagSvc" << endmsg;

    IRDBAccessSvc* rdbSvc{nullptr};
    sc = svcLocator->service(geoDbTag->getParamSvcName(), rdbSvc);
    if (sc.isFailure()) log << MSG::FATAL << "Could not locate " << geoDbTag->getParamSvcName() << endmsg;

    // Get version tag and node for InDet.
    DecodeVersionKey versionKey("InnerDetector");
    const std::string& detectorKey = versionKey.tag();
    const std::string& detectorNode = versionKey.node();

    log << MSG::DEBUG << "Retrieving Record Sets from database ..." << endmsg;
    log << MSG::DEBUG << "Key = " << detectorKey << " Node = " << detectorNode << endmsg;

    m_xMatTable = rdbSvc->getRecordsetPtr("InDetExtraMaterial", detectorKey, detectorNode);
    m_materialManager = detStore->tryRetrieve<StoredMaterialManager>("MATERIALS");
  }
} // end namespace
