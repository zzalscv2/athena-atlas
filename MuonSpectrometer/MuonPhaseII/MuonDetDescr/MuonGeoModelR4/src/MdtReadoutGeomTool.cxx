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
StatusCode MdtReadoutGeomTool::loadDimensions(MdtReadoutElement::defineArgs& define) {
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
    ///                                    is visible from the top  
    define.numTubesPerLay = ( 2. * define.halfY / define.tubePitch - 0.5);
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::loadCutouts(MdtReadoutElement::defineArgs& define) {
    const std::vector<CutOutArea>& definedCuts = m_amdbCutOuts[define.detElId];
    if (definedCuts.empty()) {
        ATH_MSG_VERBOSE("No cutouts were defiend for "<<m_idHelperSvc->toStringDetEl(define.detElId));
        return StatusCode::SUCCESS;
    }
    if (m_idHelperSvc->isEndcap(define.detElId)){
        ATH_MSG_FATAL("The cutouts are not yet implemented for endcap chambers");
        return StatusCode::FAILURE;
    }
    /// Wuhu
    ATH_MSG_VERBOSE("Load "<<definedCuts.size()<<" cutout elements for chamber "
                    <<m_idHelperSvc->toStringDetEl(define.detElId)  
                    <<" tube lengths: "<<define.longHalfX<<", chamber length: "<<define.halfY
                    <<", n Tubes: "<<define.numTubesPerLay);
    
    MdtCutOuts cutTubes{};
    for (const CutOutArea& cut : definedCuts) {
        ATH_MSG_VERBOSE("Process cut "<<cut);
        /// Calculate the boundaries along the tube. If the sign of  x is positive, 
        /// then apply the cut on the right, otherwise on the left
        const double minX = cut.origin.x() - cut.halfLengthX;
        const double maxX = cut.origin.x() + cut.halfLengthX;
        /// Calculate the tube number
        for (uint16_t lay = 1; lay <= define.firstTubePos.size(); ++lay){
            const Amg::Vector3D& tubeLay{define.firstTubePos[lay-1]};
            ATH_MSG_VERBOSE("Process cut for layer "<<static_cast<int>(lay)<<". First tube: "<<Amg::toString(tubeLay,1));
            /// Intersect the 
            std::optional<double> isect = MuonGM::intersect<3>(Amg::Vector3D::Zero(), cut.inclanation, 
                                                               tubeLay.z() * Amg::Vector3D::UnitZ(), Amg::Vector3D::UnitY());
            
            const double minY = (isect ? *isect : 0.) + cut.origin.y();
            const double maxY = (isect ? *isect : 0.) + cut.origin.y() + cut.lengthY;
            const uint16_t minTube = (minY - tubeLay.y()) / define.tubePitch;
            const uint16_t maxTube = (maxY - tubeLay.y()) / define.tubePitch;
            for (uint16_t tube = minTube; tube <= maxTube ; ++tube) {
                const IdentifierHash hash = MdtReadoutElement::measurementHash(lay, tube);
                MdtCutOut tubeCut{};
                tubeCut.layer = MdtReadoutElement::layerNumber(hash);
                tubeCut.firstTube = MdtReadoutElement::tubeNumber(hash);
                tubeCut.lastTube = MdtReadoutElement::tubeNumber(hash);
                /// Apply the cut on the right-hand side
                if (minX > 0 && maxX > 0) {
                    tubeCut.rightX = (define.longHalfX - minX);
                }
                /// Apply the cut on the left hand side
                else if (minX < 0 && maxX < 0) {
                    tubeCut.leftX = (define.longHalfX - std::abs(maxX));
                } else if (minX < 0.) {
                    tubeCut.leftX = (maxX - minX); 
                }
                if (tubeCut.rightX < 0. || tubeCut.leftX <0.) {
                    ATH_MSG_FATAL("A cut needs to be positive "<<tubeCut<<", "<<m_idHelperSvc->toStringDetEl(define.detElId));
                    return StatusCode::FAILURE;
                }
                MdtCutOuts::const_iterator exist = cutTubes.find(tubeCut);
                /// AMDB has the possibility to define cuts that are overlapping...
                if (exist != cutTubes.end()) {
                    ATH_MSG_VERBOSE("Overwrite the exisiting cut values of "<<(*exist));
                    tubeCut.rightX = std::max(tubeCut.rightX, (*exist).rightX);
                    tubeCut.leftX = std::max(tubeCut.leftX, (*exist).leftX);
                    cutTubes.erase(exist);
                }
                ATH_MSG_VERBOSE("Add new cut "<<tubeCut);
                cutTubes.insert(std::move(tubeCut));
            }
        }       
    }
    /// Now we need to summarize the cutouts
    std::vector<MdtCutOut> cutVec{};
    cutVec.insert(cutVec.end(), cutTubes.begin(), cutTubes.end());
    std::sort(cutVec.begin(),cutVec.end());

    for (size_t tC = 0; tC < cutVec.size(); ++tC) {
        MdtCutOut& cut = cutVec[tC];
        for(size_t seqTube = tC +1; seqTube < cutVec.size(); ++seqTube) {
            MdtCutOut& nextCut = cutVec[seqTube];
            /// Cut applies on a different tube
            if (nextCut.layer != cut.layer) break;
            // The cuts change
            if (std::abs(nextCut.leftX - cut.leftX) > std::numeric_limits<double>::epsilon() ||
                std::abs(nextCut.rightX - cut.rightX)> std::numeric_limits<double>::epsilon()) break;
            /// There is a hole between the cuts?
            if (nextCut.firstTube - cut.lastTube > 1 ) break;
            cut.lastTube = nextCut.lastTube;
            ++tC;
        }
        define.cutouts.insert(std::move(cut));
    }
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
            // return StatusCode::FAILURE;
            continue;
        }
        ATH_MSG_VERBOSE("Key "<<key<<" brought us "<<m_idHelperSvc->toStringDetEl(define.detElId));
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
        /// Cut outs
        ATH_CHECK(loadCutouts(define));
        
        std::unique_ptr<MdtReadoutElement> mdtDetectorElement = std::make_unique<MdtReadoutElement>(std::move(define));
        mgr.addMdtReadoutElement(std::move(mdtDetectorElement)).ignore();

        //ATH_CHECK(mgr.addMdtReadoutElement(std::move(mdtDetectorElement)));       
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
        const std::vector<double> tubeX{tokenizeDouble(record->getString("TUBXCO"),";")},
                                  tubeY{tokenizeDouble(record->getString("TUBYCO"),";")};
        for (unsigned int lay = 0; lay < nLay; ++lay) {
            /// TUBEXCO -> first tube position in the precision plane            
            /// TUBEYCO -> layer height 
            Amg::Vector3D tubePos{tubeX[lay], tubeY[lay], 0.};
            pars.firstTubePos.emplace_back(tubeRot*tubePos * Gaudi::Units::cm);
        }
        const std::string key {record->getString("WMDT_TYPE")};
        ATH_MSG_DEBUG("Extracted parameters " << pars<<" will be safed under key "<<key);
        m_parBook[key] = std::move(pars);
    }   
    /// Next step... Download the AMDB cut out tables
    paramTable = accessSvc->getRecordsetPtr("mdtCutouts", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty cutout table found");
        return StatusCode::FAILURE;
    }
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    for (const IRDBRecord* record : *paramTable) {
        const std::string stName{record->getString("MDTCUTOUTS_DATA_ID").substr(0,3)};
        const int stationEta{record->getInt("etaIndex")};
        const int stationPhi{record->getInt("phiIndex")+1};
        /// Minor comment: May be rename it to multiLayer?
        const int multiLayer{record->getInt("Multilayer")};
        bool is_valid{false};
        Identifier detElId = idHelper.channelID(stName, stationEta, stationPhi, multiLayer, 1, 1, is_valid);
        if (!is_valid){
            ATH_MSG_FATAL("Failed to construct Identifier from "<<stName
                          <<" eta: "<<stationEta<<", phi: "<<stationPhi<<", ml:" <<multiLayer);
            return StatusCode::FAILURE;
        }
        /// Define a new cutout
        CutOutArea newCutOut{};
        newCutOut.origin = Amg::Vector3D{record->getDouble("dx"),
                                         record->getDouble("dy"), 0};
        newCutOut.inclanation = Amg::getRotateX3D(record->getDouble("D1") * Gaudi::Units::degree) * Amg::Vector3D::UnitZ();
        newCutOut.halfLengthX = std::max(record->getDouble("W_xS"),record->getDouble("W_xL") ) / 2.;
        newCutOut.lengthY = record->getDouble("L_y");
        m_amdbCutOuts[detElId].emplace_back(std::move(newCutOut));
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4