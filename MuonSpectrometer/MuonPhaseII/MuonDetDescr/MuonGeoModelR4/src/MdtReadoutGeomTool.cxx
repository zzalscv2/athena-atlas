/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModelR4/MdtReadoutGeomTool.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoBox.h"

#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/StringUtils.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "MuonGeoModelR4/Utils.h"
namespace MuonGMR4 {
MdtReadoutGeomTool::MdtReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);
}
StatusCode MdtReadoutGeomTool::initialize() {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());

    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::loadDimensions(MdtReadoutElement::defineArgs& define ) {
    const GeoShape* shape = extractShape(define.physVol, msgStream());
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    if (shape->typeID() == GeoTrd::getClassTypeID()){
        const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
        ATH_MSG_VERBOSE("Trapezoidal geometry shape: "<<trd->getXHalfLength1()
                           <<", height: "<<trd->getXHalfLength2()<<" -- short/long tube length: "
                          <<trd->getYHalfLength1()<<"/" <<trd->getYHalfLength2()<<", chamber width: "<<trd->getZHalfLength());
        define.longHalfX = std::max(trd->getYHalfLength1(), trd->getYHalfLength2()) * Gaudi::Units::mm;
        define.shortHalfX = std::min(trd->getYHalfLength1(), trd->getYHalfLength2())* Gaudi::Units::mm;
        define.halfY = trd->getZHalfLength()* Gaudi::Units::mm;
        define.halfHeight = std::max(trd->getXHalfLength1(), trd->getXHalfLength2()) * Gaudi::Units::mm;
    } else {
        ATH_MSG_FATAL("Unknown shape type "<<shape->type());
        return StatusCode::FAILURE;
    }
    /// As a very wise person once said.. We simply take Bikini booottom
    /// and push it somewhere elseee
    /// Also Rotate the system such that the wire is pointing the Y axis
    static const Amg::Transform3D systemRotation = Amg::getRotateY3D(M_PI_2)*Amg::getRotateZ3D(M_PI_2);
    define.toVolCenter = extractShifts(define.physVol, msgStream()) * systemRotation;
    /// Calculate the number of tubes from the width
    /// width  = tube pitch x (N + 0.5) <- On one site a half tube of the second layer 
    ///                                    is visible from the tube  
    define.numTubesPerLay = ( 2. * define.halfY / define.tubePitch - 0.5);
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {

    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(readParameterBook());

    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector
    // elements
    std::map<std::string, GeoFullPhysVol*> mapFPV =
        sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");

    for (auto& [key, pv] : mapFPV) {
        /// The keys should be formatted like
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_(<STATIONPHI>-1)_ML
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() != 5 ||
            key_tokens[1].find("MDT") == std::string::npos)
            continue;

        MdtReadoutElement::defineArgs define{};
        bool isValid{false};
        define.detElId = idHelper.channelID(
            key_tokens[0].substr(0, 3), atoi(key_tokens[2]),
            atoi(key_tokens[3]) + 1, atoi(key_tokens[4]), 1, 1, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            continue;
            /// We should return a status code failure in the future
            /// return StatusCode::FAILURE;
        }
        ATH_MSG_VERBOSE("Key "<<key<<" brought us "<<m_idHelperSvc->toString(define.detElId));
        /// Skip the endcap chambers
        /// if (m_idHelperSvc->isEndcap(define.detElId)) continue;
        define.physVol = pv;
        define.chambDesign = key_tokens[1];
        
        /// Load first tube etc. from the parameter book table
        ParamBookTable::const_iterator book_itr = m_parBook.find(define.chambDesign);
        if (book_itr == m_parBook.end()) {
            ATH_MSG_FATAL("There is no chamber called "<<define.chambDesign);
            return StatusCode::FAILURE;
        }
        static_cast<parameterBook&>(define) = book_itr->second;
        /// Chamber dimensions are given from the GeoShape
        ATH_CHECK(loadDimensions(define));
        
        std::unique_ptr<MdtReadoutElement> mdtDetectorElement =
            std::make_unique<MdtReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addMdtReadoutElement(std::move(mdtDetectorElement)));       
    }
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::readParameterBook() {
    if (m_parBook.size())
        return StatusCode::SUCCESS;
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(),
                                           name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WMDT", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    for (IRDBRecord* record : *paramTable) {
        parameterBook pars{};
        pars.tubeWall = record->getDouble("TUBWAL") * Gaudi::Units::cm;
        pars.tubePitch = record->getDouble("TUBPIT") * Gaudi::Units::cm;
        pars.tubeInnerRad = record->getDouble("TUBRAD") * Gaudi::Units::cm;
        pars.endPlugLength = record->getDouble("TUBDEA") * Gaudi::Units::cm;
        pars.radLengthX0 = record->getDouble("X0");
        /// The following access pattern hopefully changes in the near future
        /// such that we can use the array mechanism of the XML file
        unsigned int nLay = record->getInt("LAYMDT");
        static const Amg::Transform3D tubeRot =Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
        for (unsigned int lay = 0; lay < nLay; ++lay) {
            /// TUBEXCO -> first tube position in the precision plane            
            /// TUBEYCO -> layer height 
            Amg::Vector3D tubePos{record->getDouble("TUBXCO_" + std::to_string(lay)),
                                  record->getDouble("TUBYCO_" + std::to_string(lay)),
                                  0.};
            pars.firstTubePos.emplace_back(tubeRot*tubePos * Gaudi::Units::cm);
        }
        std::stringstream key_sstr{};
        key_sstr<< record->getString("TYP")<<std::setfill('0') << std::setw(2)<<record->getInt("IW"); 
        ATH_MSG_DEBUG("Extracted parameters " << pars<<" will be safed under key "<<key_sstr.str());
        m_parBook[key_sstr.str()] = std::move(pars);
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4