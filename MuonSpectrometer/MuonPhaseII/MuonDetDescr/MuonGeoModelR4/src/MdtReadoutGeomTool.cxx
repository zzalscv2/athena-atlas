/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/MdtReadoutGeomTool.h>

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
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <RDBAccessSvc/IRDBRecord.h>

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
    ATH_CHECK(m_geoUtilTool.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::loadDimensions(const FactoryCache& facCache, MdtReadoutElement::defineArgs& define) const {    
    
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    /// The trapezoid defines the length of the chamber including the extra material
    /// stemming from the faraday cache etc.
    if (shape->typeID() == GeoTrd::getClassTypeID()) {
        ATH_MSG_VERBOSE("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
        const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
        define.longHalfX = std::max(trd->getYHalfLength1(), trd->getYHalfLength2()) * Gaudi::Units::mm;
        define.shortHalfX = std::min(trd->getYHalfLength1(), trd->getYHalfLength2())* Gaudi::Units::mm;
        define.halfY = trd->getZHalfLength()* Gaudi::Units::mm;
        define.halfHeight = std::max(trd->getXHalfLength1(), trd->getXHalfLength2()) * Gaudi::Units::mm;
    } else {
        ATH_MSG_FATAL("Unknown shape type "<<shape->type());
        return StatusCode::FAILURE;
    }
    /// The particular tubes and their lengths can be directly extracted from GeoModel
    /// Loop over the child nodes of the multi layer to find the nodes containing 
    /// all the tubes per layer  
    for (unsigned int child = 0; child < define.physVol->getNChildVols(); ++ child) {
        const PVConstLink layerVol = define.physVol->getChildVol(child);
        /// That's nothing that usually has tubes inside
        if (layerVol->getNChildVols() <= 1) continue;
        ATH_MSG_VERBOSE("Add new tube layer "<<m_idHelperSvc->toStringDetEl(define.detElId)<<
                       std::endl<<std::endl<<m_geoUtilTool->dumpVolume(layerVol));
        define.tubeLayers.emplace_back(layerVol);
    }
    /// As a very wise person once said.. We simply take Bikini booottom
    /// and push it somewhere elseee
    define.toVolCenter = m_geoUtilTool->extractShifts(define.physVol);
    define.readoutSide = facCache.readoutOnLeftSide.count(m_idHelperSvc->chamberId(define.detElId)) ? -1. : 1.;
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    FactoryCache facCache{};
    ATH_CHECK(readParameterBook(facCache));
    fillFlippedReadouts(facCache);

    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector
    // elements
    
    using alignNodeMap = IMuonGeoUtilityTool::alignNodeMap;    
    using physNodeMap = IMuonGeoUtilityTool::physNodeMap;
    using alignedPhysNodes = IMuonGeoUtilityTool::alignedPhysNodes;
    physNodeMap mapFPV = sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");
    alignNodeMap mapAlign = sqliteReader->getPublishedNodes<std::string, GeoAlignableTransform*>("Muon");
    alignedPhysNodes alignedNodes = m_geoUtilTool->selectAlignableVolumes(mapFPV, mapAlign);

    SurfaceBoundSetPtr<Acts::LineBounds> tubeBounds = std::make_shared<SurfaceBoundSet<Acts::LineBounds>>();
    SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds = std::make_shared<SurfaceBoundSet<Acts::TrapezoidBounds>>();
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
        define.tubeBounds = tubeBounds;
        define.layerBounds = layerBounds;
        bool isValid{false};
        define.detElId = idHelper.channelID(key_tokens[0].substr(0, 3), 
                                            atoi(key_tokens[2]),
                                            atoi(key_tokens[3]) + 1, 
                                            atoi(key_tokens[4]), 1, 1, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            return StatusCode::FAILURE;
        }       
        ATH_MSG_DEBUG("Key "<<key<<" brought us "<<m_idHelperSvc->toStringDetEl(define.detElId));
        /// Skip the endcap chambers
        define.physVol = pv;
        define.chambDesign = key_tokens[1];
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol, alignedNodes);   
       
        /// Load first tube etc. from the parameter book table
        ParamBookTable::const_iterator book_itr = facCache.parBook.find(define.chambDesign);
        if (book_itr == facCache.parBook.end()) {
            ATH_MSG_FATAL("There is no chamber called "<<define.chambDesign);
            return StatusCode::FAILURE;
        }
        static_cast<parameterBook&>(define) = book_itr->second;
        
        /// Chamber dimensions are given from the GeoShape
        ATH_CHECK(loadDimensions(facCache, define));
        std::unique_ptr<MdtReadoutElement> mdtDetectorElement = std::make_unique<MdtReadoutElement>(std::move(define));      
        ATH_CHECK(mgr.addMdtReadoutElement(std::move(mdtDetectorElement)));       
    }
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::readParameterBook(FactoryCache& cache) const {
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
        unsigned int nLay = record->getInt("LAYMDT");
        const std::string key {record->getString("WMDT_TYPE")};
        ATH_MSG_DEBUG("Extracted parameters " <<pars<<" number of layers: "<<nLay<<" will be safed under key "<<key);
        cache.parBook[key] = std::move(pars);
    }   
    return StatusCode::SUCCESS;
}

void MdtReadoutGeomTool::fillFlippedReadouts(FactoryCache& facCache) const {
    /// Potentially we want to publish this in a table
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    const int stIdxBEE = idHelper.stationNameIndex("BEE");
    const int stIdxBIM = idHelper.stationNameIndex("BIM");
    const int stIdxBIR = idHelper.stationNameIndex("BIR");
    const int stIdxBIS = idHelper.stationNameIndex("BIS");
    const int stIdxBOF = idHelper.stationNameIndex("BOF");
    const int stIdxEIL = idHelper.stationNameIndex("EIL");
    const int stIdxBOS = idHelper.stationNameIndex("BOS");
    const int stIdxBOG = idHelper.stationNameIndex("BOG");
    
    
    for (MdtIdHelper::const_id_iterator itr = idHelper.module_begin();
         itr != idHelper.module_end(); ++itr) {

      const Identifier& chId{*itr};
      const int stIdx = idHelper.stationName(chId);
      const int stEta = idHelper.stationEta(chId);      
      const int stPhi = idHelper.stationPhi(chId);

      const bool isBarrel{idHelper.isBarrel(chId)};
      const bool sideA{stEta >= 0};
      const bool isLarge{!idHelper.isSmall(chId)};
      int sign = (isBarrel ? 1 : -1) * (sideA ? 1 : -1) * (isLarge ? -1 : 1);
      /// Of course there're exceptions to this rule, otherwise it weren't a rule
      if (stIdxBEE == stIdx ||  
          (stIdxEIL == stIdx && !( stEta ==3  && stPhi ==6) )|| 
          (stIdxBOS == stIdx && !sideA) ||
          (stIdxBOG == stIdx && ( (stPhi == 7) || (stEta ==3) )) ||
          (stIdxBOF == stIdx && stPhi != 6) ||
          (stIdxBIM == stIdx && stPhi == 6) || 
          (stIdxBIR == stIdx && stPhi == 6) || 
          (stIdxBIS == stIdx && stPhi == 6)) sign = -sign;
      if (sign == -1) {
        ATH_MSG_VERBOSE("Readout of "<<m_idHelperSvc->toStringChamber(chId)<<" is at negative Z");
        facCache.readoutOnLeftSide.insert(chId);
      }
    }
}
}  // namespace MuonGMR4