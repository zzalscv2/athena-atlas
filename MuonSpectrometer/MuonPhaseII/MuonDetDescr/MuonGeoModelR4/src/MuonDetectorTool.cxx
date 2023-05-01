/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModelR4/MuonDetectorTool.h"

#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
/**
 ** Constructor(s)
 **/

namespace MuonGMR4 {
MuonDetectorTool::MuonDetectorTool(const std::string &type,
                                   const std::string &name,
                                   const IInterface *parent)
    : GeoModelTool(type, name, parent) {
    declareInterface<IGeoModelTool>(this);
}

StatusCode MuonDetectorTool::initialize() {
    ATH_MSG_INFO("Initializing ...");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_detTechTools.retrieve());
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    return StatusCode::SUCCESS;
}

MuonDetectorTool::~MuonDetectorTool() = default;
/**
 ** Create the Detector Node corresponding to this tool
 **/
StatusCode MuonDetectorTool::create() {

    m_manager = new MuonDetectorManager();

    GeoModelExperiment *theExpt = nullptr;
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
    GeoPhysVol *world = theExpt->getPhysVol();

    for (auto &readOutTool : m_detTechTools)
        ATH_CHECK(readOutTool->buildReadOutElements(*m_manager));

    GeoVolumeCursor cursor(world);
    while (!cursor.atEnd()) {
        std::string volName = cursor.getName();
        if (volName == "MuonBarrel") {
            m_manager->addTreeTop(GeoPVLink(cursor.getVolume().operator->()));
        }
        cursor.next();
    }

    ATH_CHECK(detStore()->record(m_manager, m_manager->getName()));
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
    theExpt->addManager(m_manager);

    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorTool::clear() {
    SG::DataProxy *proxy = detStore()->proxy(
        ClassID_traits<MuonDetectorManager>::ID(), m_manager->getName());
    if (proxy) {
        proxy->reset();
        m_manager = nullptr;
    }
    return StatusCode::SUCCESS;
}

}  // namespace MuonGMR4
