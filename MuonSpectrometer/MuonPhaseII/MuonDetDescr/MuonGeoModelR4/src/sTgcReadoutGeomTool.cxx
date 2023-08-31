/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/sTgcReadoutGeomTool.h>

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>

#include <MuonReadoutGeometryR4/StringUtils.h>
#include <RDBAccessSvc/IRDBRecord.h>

namespace MuonGMR4 {

sTgcReadoutGeomTool::sTgcReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);
}
StatusCode sTgcReadoutGeomTool::initialize() {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoUtilTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode sTgcReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(readParameterBook());

    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector
    // elements
    std::map<std::string, GeoFullPhysVol*> mapFPV =
        sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");

    for (auto& [key, pv] : mapFPV) {
        /// The keys should be formatted like UPDATE!!
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_(<STATIONPHI>-1)_ML
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() != 5 ||
            key_tokens[0].find("sTGC") == std::string::npos)
            continue;
        //ATH_MSG_ALWAYS("Ishan has written tool. Key is "<<key);
        sTgcReadoutElement::defineArgs define{};
        bool isValid{false};
        const std::string stName = key_tokens[1][1] == 'L' ? "STL" : "STS";
        const int stEta = atoi(key_tokens[2]);
        const int stPhi = atoi(key_tokens[3]) + 1;
        const int ml = atoi(key_tokens[4]);

        define.detElId = idHelper.channelID(stName, stEta, stPhi, ml, 1, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            return StatusCode::FAILURE;
        }
        /// Skip the endcap chambers
        define.physVol = pv;
        define.chambDesign = key_tokens[1];       

        ATH_MSG_ALWAYS("Key "<<key<<" brought us "<<m_idHelperSvc->toStringDetEl(define.detElId));  
        std::unique_ptr<sTgcReadoutElement> newEle = std::make_unique<sTgcReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addsTgcReadoutElement(std::move(newEle)));
    }
    return StatusCode::SUCCESS;
}
StatusCode sTgcReadoutGeomTool::readParameterBook() {
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(),
                                           name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WSTGC", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_ALWAYS("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    for (const IRDBRecord* record : *paramTable) {
        // parameterBook pars{};
        const std::string padShift = record-> getString("PadPhiShift_A");
        std::vector<double> padShiftVwc = tokenizeDouble(padShift, ";");
        ATH_MSG_ALWAYS("Blub "<<padShift);
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4