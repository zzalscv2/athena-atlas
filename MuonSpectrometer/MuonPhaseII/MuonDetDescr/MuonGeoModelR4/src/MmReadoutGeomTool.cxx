/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/MmReadoutGeomTool.h>
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
#include <RDBAccessSvc/IRDBRecord.h>

using namespace ActsTrk;

namespace {
    constexpr double tolerance = 0.001 * Gaudi::Units::mm;

}

using namespace CxxUtils;

namespace MuonGMR4 {


using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;

MmReadoutGeomTool::MmReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);

}
StatusCode MmReadoutGeomTool::initialize() {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoUtilTool.retrieve());
    return StatusCode::SUCCESS;
}



StatusCode MmReadoutGeomTool::loadDimensions(MmReadoutElement::defineArgs& define,
                                              FactoryCache& factoryCache, int stationEta) {    
    
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
     const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
    /// The half sizes of the MicroMegas trapezoid
    if (shape->typeID() != GeoTrd::getClassTypeID()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" expect shape to be a trapezoid but it's "<<m_geoUtilTool->dumpShape(shape));
        return StatusCode::FAILURE;
    }

    const GeoTrd* trapezoid = static_cast<const GeoTrd*>(shape);   
    define.halfThickness = trapezoid->getXHalfLength1() * Gaudi::Units::mm;
    define.halfShortWidth = trapezoid->getYHalfLength1() * Gaudi::Units::mm;
    define.halfLongWidth = trapezoid->getYHalfLength2() * Gaudi::Units::mm;
    define.halfLength = trapezoid->getZHalfLength() * Gaudi::Units::mm;
   
        ATH_MSG_DEBUG("Extracted parameters "
                       <<", halfThickness: "<<define.halfThickness<<"/"
                       <<", halfShortWidth : "<<define.halfShortWidth<<"/"
                       <<", halfLongWidth : "<<define.halfLongWidth<<"/"
                       <<", halfLength : "<<define.halfLength<<"/"
                    );


    std::vector<physVolWithTrans> allGasGaps = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "MicroMegasGas");
    if (allGasGaps.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any children MicroMegasGas");
        return StatusCode::FAILURE;
    }

    FactoryCache::ParamBookTable::const_iterator parBookItr = factoryCache.parameterBook.find(define.chambDesign);
    if (parBookItr == factoryCache.parameterBook.end()) {
        ATH_MSG_FATAL("The chamber "<<define.chambDesign<<" is not part of the WMM table");
        return StatusCode::FAILURE;
    }     

    const wMMTable& paramBook{parBookItr->second};

    /*Sort Gas Gaps in a module quadruplet based on their Z position
      On side A the gas gap have local x axis translations : -24.975 , -8.175 , 8.175, 24.975
      This order refers to the gap closest to the origin  to the outermost gap of the experiement.
      On side C the order is reversed : 24.975 , 8.175 , -8.175, -24.975.
      I'm sorting the gas gap with the first one being closest to the experiment's origin.
      I doing this in case the gaps aren't sorted in that way within the Full Phys Vol Quadruplet.
      Also, now the stereoAngles and totalActiveStrips vectors will match the sequence of the allGasGaps vector.*/
    std::sort(allGasGaps.begin(), allGasGaps.end(),
              [&define,&stationEta](const physVolWithTrans&gapI, const physVolWithTrans & gapJ) {
                const Amg::Vector3D posGapI = gapI.transform.translation();
                const Amg::Vector3D posGapJ = gapJ.transform.translation();
                if (stationEta>0) // stationEta 1 or 2.
                    return posGapI.x() < posGapJ.x();
                else // stationEta -1 or -2.
                    return posGapI.x() > posGapJ.x();             
              });

    for (std::size_t gap = 0; gap < allGasGaps.size(); ++gap) {

        const auto& gapVol = allGasGaps[gap];
        const GeoShape* gapShape = m_geoUtilTool->extractShape(gapVol.physVol);
        if (gapShape->typeID() != GeoTrd::getClassTypeID()) {
            ATH_MSG_FATAL("Failed to extract a geo shape");
            return StatusCode::FAILURE;
        }
        const GeoTrd* gapTrd = static_cast<const GeoTrd*>(gapShape);
        ATH_MSG_DEBUG("MicroMegas Gas gap dimensions "<<m_geoUtilTool->dumpShape(gapTrd));

        /*The origin of the chamber/gasGap axes system is located at the center of the chamber.
        We subtract the HalfLength across the Z axis to transform from the center to the origin of the trapezoid
        The we add the strip pitch to reach the position of the first strip.*/

        StripDesignPtr stripDesign = std::make_unique<StripDesign>();

        stripDesign->defineStripLayout(Amg::Vector2D{-gapTrd->getZHalfLength() + paramBook.stripPitch, 0.},
                                        paramBook.stripPitch,
                                        paramBook.stripWidth,
                                        paramBook.totalActiveStrips.at(gap));       

        stripDesign->defineTrapezoid(define.halfShortWidth, define.halfLongWidth, define.halfLength, paramBook.stereoAngle.at(gap));
        stripDesign = (*factoryCache.stripDesigns.emplace(stripDesign).first);
        StripLayer stripLayer(gapVol.transform, stripDesign, 
                            layerHash(gap+1));
        define.layers.push_back(std::move(stripLayer));


    } //end of gas gap loop


    return StatusCode::SUCCESS;
}


IdentifierHash MmReadoutGeomTool::layerHash(const int gasGap) const {

    
    IdentifierHash idHash = (gasGap-1);
    ATH_MSG_DEBUG("gasGap: "<<gasGap<<" --> "<<static_cast<unsigned int>(idHash));
    return idHash;
}


StatusCode MmReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }


    FactoryCache facCache{};
    ATH_CHECK(readParameterBook(facCache));


    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    for (auto itr = idHelper.detectorElement_begin(); itr != idHelper.detectorElement_end(); ++itr){
        ATH_MSG_DEBUG("Expect detector Identifier "<<m_idHelperSvc->toString(*itr));
    }
    // Get the list of full phys volumes from SQLite, and create detector
    // elements
    using alignNodeMap = IMuonGeoUtilityTool::alignNodeMap;    
    using physNodeMap = IMuonGeoUtilityTool::physNodeMap;
    using alignedPhysNodes = IMuonGeoUtilityTool::alignedPhysNodes;
    /// Retrieve the list of full physical volumes & alignable nodes and connect them together afterwards
    physNodeMap mapFPV = sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");
    alignNodeMap mapAlign = sqliteReader->getPublishedNodes<std::string, GeoAlignableTransform*>("Muon");
    alignedPhysNodes alignedNodes = m_geoUtilTool->selectAlignableVolumes(mapFPV, mapAlign);
#ifndef SIMULATIONBASE
    SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds= std::make_shared<SurfaceBoundSet<Acts::TrapezoidBounds>>();
#endif    

    for (auto& [key, pv] : mapFPV) {
        /// For MicroMegas the Keys are formatted in the following way.
        ///   <MM>_<LARGE/SMALL SECTOR + MODULE TYPE>_<QUADRUPLET NUMBER>_<ETA INDEX>_<PHI INDEX -1>_<COPY NUMBER>
        // e.g. MM_SM1Q2_1_6_1 . It's the "type" Attribute in the new GeoModel XML files.
        std::vector<std::string> key_tokens = tokenize(key, "_");

        if (key_tokens[0].find("MM") == std::string::npos )
            continue;   

        ATH_MSG_DEBUG("Retrieving MicroMegas Quadruplet : " << key );

        MmReadoutElement::defineArgs define{};
        bool isValid{false};
        define.detElId = idHelper.channelID(key_tokens[1][0] == 'S' ? "MMS" : "MML", // Replace <MM> string part with <MMS> or <MML> to match the Identifier.
                                            atoi(key_tokens[2].c_str()), // Eta index
                                            atoi(key_tokens[3].c_str()) + 1, // Phi index (from 0 to 7 in GeoModel). needs a +1
                                            atoi(key_tokens[4].c_str()), 1, 1, isValid); //Copy Number which reflects the number of the multilayer.
                                            // THen the two 1s are reflecting gasGap and channel Number. They can be set to 1s as this is all we need
                                            // to get the Identifier for the multilayer.

    
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            return StatusCode::FAILURE;
        }      

        ATH_MSG_DEBUG("Key "<<key<<" brought us "<<m_idHelperSvc->toStringDetEl(define.detElId));
        define.physVol = pv;
        define.chambDesign = key_tokens[0]+"_"+key_tokens[1]; // Recover the string denoted in WMM tables. e.g. chambDesign = "MM_SM1Q2"
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol, alignedNodes);  
        ATH_CHECK(loadDimensions(define, facCache,atoi(key_tokens[2].c_str())));
#ifndef SIMULATIONBASE
        define.layerBounds = layerBounds;
#endif
        std::unique_ptr<MmReadoutElement> readoutEle = std::make_unique<MmReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addMmReadoutElement(std::move(readoutEle)));
    }    
    return StatusCode::SUCCESS;
}



StatusCode MmReadoutGeomTool::readParameterBook(FactoryCache& cache) {

    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(), name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WMM", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    

    
    for (const IRDBRecord* record : *paramTable) {
        const std::string chambType = record->getString("WMM_TYPE");
        wMMTable& parBook = cache.parameterBook[chambType];
        parBook.stripPitch = record->getDouble("stripPitch") ;
        parBook.stripWidth = record->getDouble("stripWidth") ; 
        parBook.stereoAngle = tokenizeDouble(record->getString("stereoAngle"), ";");
        parBook.totalActiveStrips = tokenizeInt(record->getString("totalActiveStrips"), ";");

        
        ATH_MSG_VERBOSE("Extracted parameters for chamber "<<chambType
                       <<", stripPitch (eta/phi): "<<parBook.stripPitch<<"/"
                       <<", stripWidth (eta/phi): "<<parBook.stripWidth<<"/"
                    );
    }
    
    return StatusCode::SUCCESS;
}





}  // namespace MuonGMR4