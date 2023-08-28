/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/RpcReadoutGeomTool.h>

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoBox.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <RDBAccessSvc/IRDBRecord.h>

namespace {
    constexpr double tolerance = 0.001 * Gaudi::Units::mm;
    

}

namespace MuonGMR4 {

using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;
using defineArgs = RpcReadoutElement::defineArgs;

/// Helper struct to attribute the Identifier fields with the
/// gas gap volumes
struct gapVolume: public physVolWithTrans {
    gapVolume(physVolWithTrans&& physVol,
                unsigned int gap,
                unsigned int phi):
        physVolWithTrans{std::move(physVol)},
        gasGap{gap},
        doubPhi{phi} {}
    unsigned int gasGap{0};
    unsigned int doubPhi{0};
    
};


RpcReadoutGeomTool::RpcReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);

}
StatusCode RpcReadoutGeomTool::initialize() {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoUtilTool.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode RpcReadoutGeomTool::loadDimensions(RpcReadoutElement::defineArgs& define,
                                              FactoryCache& factoryCache) {    
    
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
    /// The half sizes of the 
    if (shape->typeID() != GeoBox::getClassTypeID()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" expect shape to be a box but it's "<<m_geoUtilTool->dumpShape(shape));
        return StatusCode::FAILURE;
    }

    const GeoBox* box = static_cast<const GeoBox*>(shape);
    define.halfZ = box->getYHalfLength() * Gaudi::Units::mm;
    define.halfY = box->getZHalfLength() * Gaudi::Units::mm;
    define.halfX = box->getXHalfLength() * Gaudi::Units::mm;

    /// Navigate through the GeoModel tree to find all gas volume leaves
    std::vector<physVolWithTrans> allGasGaps = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "RpcLayer");
    /// For one reason or another the x-axis points along the gasgap 
    /// and y along doublet phi
    std::sort(allGasGaps.begin(), allGasGaps.end(), [](const physVolWithTrans&a, const physVolWithTrans & b){
         const Amg::Vector3D cA = a.transform.translation();
         const Amg::Vector3D cB = b.transform.translation();
         if (std::abs(cA.x() - cB.x()) > tolerance) return cA.x() < cB.x();
         return cA.y() < cB.y();
    });
    /// Now we need to associate the gasGap volumes with the gas gap number &
    /// the doublet Phi
    Amg::Vector3D prevGap{allGasGaps[0].transform.translation()};
    unsigned int gasGap{1}, doubletPhi{0};
    
    std::vector<gapVolume> allGapsWithIdx{};
    for (physVolWithTrans& gapVol : allGasGaps) {
        Amg::Vector3D gCen = gapVol.transform.translation();
        /// The volume points to a new gasgap
        if (std::abs(gCen.x() - prevGap.x()) > tolerance) {
            ++gasGap;
            doubletPhi = 1;
        } else ++doubletPhi;
        prevGap = std::move(gCen);
        allGapsWithIdx.emplace_back(std::move(gapVol), gasGap, doubletPhi);
    }
    /// We know now whether we had 2 or 3 gasgaps and also whether there 2 or 1 panels in phi
    define.nGasGaps = gasGap;
    define.nGapsInPhi = doubletPhi;
    
    FactoryCache::ParamBookTable::const_iterator parBookItr = factoryCache.parameterBook.find(define.chambDesign);
    if (parBookItr == factoryCache.parameterBook.end()) {
        ATH_MSG_FATAL("The chamber "<<define.chambDesign<<" is not part of the WRPC table");
        return StatusCode::FAILURE;
    }
    const wRPCTable& paramBook{parBookItr->second};

    for (gapVolume& gapVol : allGapsWithIdx) {
        const GeoShape* gapShape = m_geoUtilTool->extractShape(gapVol.physVol);
        if (gapShape->typeID() != GeoBox::getClassTypeID()) {
            ATH_MSG_FATAL("Failed to extract a geo shape");
            return StatusCode::FAILURE;
        }
        const GeoBox* gapBox = static_cast<const GeoBox*>(gapShape);
        ATH_MSG_VERBOSE("Gas gap dimensions "<<m_geoUtilTool->dumpShape(gapBox));

        StripDesignPtr etaDesign = std::make_unique<StripDesign>();
        /// Define the strip layout
        etaDesign->defineStripLayout(Amg::Vector2D{-gapBox->getZHalfLength() + 26.55, 0.},
                                     paramBook.stripPitchEta,
                                     paramBook.stripWidthEta,
                                     paramBook.numEtaStrips);
        /// Define the box layout
        etaDesign->defineTrapezoid(gapBox->getXHalfLength(), gapBox->getXHalfLength(), gapBox->getZHalfLength());
        gapVol.transform = gapVol.transform * Amg::Translation3D{0,-10, 0} * Amg::getRotateY3D(-90. * Gaudi::Units::degree);
        
        etaDesign = (*factoryCache.stripDesigns.emplace(etaDesign).first);
        StripLayer etaLayer(gapVol.transform, etaDesign, 
                            layerHash(define, gapVol.gasGap, gapVol.doubPhi, false));
        ATH_MSG_VERBOSE("Added new eta gap at "<<etaLayer);
        define.layers.push_back(std::move(etaLayer));
        if (!define.etaDesign) define.etaDesign = etaDesign;
        StripDesignPtr phiDesign = std::make_unique<StripDesign>();
        phiDesign->defineStripLayout(Amg::Vector2D{-gapBox->getXHalfLength() +22.1, 0.},
                                     paramBook.stripPitchPhi,
                                     paramBook.stripWidthPhi,
                                     paramBook.numPhiStrips);
        phiDesign->defineTrapezoid(gapBox->getZHalfLength(), gapBox->getZHalfLength(), gapBox->getXHalfLength());
        /// Next build the phi layer
        phiDesign = (*factoryCache.stripDesigns.emplace(phiDesign).first);
        StripLayer phiLayer(gapVol.transform  * Amg::getRotateY3D(-90. * Gaudi::Units::deg),
                            phiDesign,
                            layerHash(define, gapVol.gasGap, gapVol.doubPhi, true));
        ATH_MSG_VERBOSE("Added new phi gap at "<<phiLayer);
        define.layers.push_back(std::move(phiLayer));
        if (!define.phiDesign) define.phiDesign = phiDesign;
    }
    std::sort(define.layers.begin(), define.layers.end(), 
             [](const StripLayer&a ,const StripLayer& b) {
                 return a.hash() < b.hash();
             });
    return StatusCode::SUCCESS;
}
IdentifierHash RpcReadoutGeomTool::layerHash(const RpcReadoutElement::defineArgs& args, const int gasGap, const int doubPhi, const bool measPhi) const {
    const unsigned int hashShiftDbl{args.hasPhiStrips ? 1u :0u};
    const int readOutDoubPhi = m_idHelperSvc->rpcIdHelper().doubletPhi(args.detElId);
    const unsigned int hashShiftGap{hashShiftDbl + (args.nGapsInPhi <= readOutDoubPhi ? 0u : 1u)};
    return IdentifierHash{ (gasGap -1) << hashShiftGap | 
                          1u * std::max(doubPhi - readOutDoubPhi,0) << hashShiftDbl | measPhi};
}

StatusCode RpcReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    
    FactoryCache facCache{};
    ATH_CHECK(readParameterBook(facCache));

    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    for (auto itr = idHelper.detectorElement_begin(); itr != idHelper.detectorElement_end(); ++itr){
        ATH_MSG_INFO("Expect detector Identifier "<<m_idHelperSvc->toString(*itr));
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
    
   

    for (auto& [key, pv] : mapFPV) {
        /// The keys should be formatted like
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_(<STATIONPHI>-1)_<DOUBLETR>_<DOUBLETPHI>_<DOUBLETZ>
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() < 7 ||
            key_tokens[1].find("RPC") == std::string::npos)
            continue;
       
        bool isValid{false};
        /// Retrieve first the station Identifier
        const Identifier elementID = idHelper.padID(idHelper.stationNameIndex(key_tokens[0].substr(0, 3)),
                                                    atoi(key_tokens[2]),
                                                    atoi(key_tokens[3]) + 1, 
                                                    atoi(key_tokens[4]),
                                                    atoi(key_tokens[6]),
                                                    atoi(key_tokens[5]), isValid);
        if (!isValid){
            ATH_MSG_FATAL("Failed to construct the station Identifier from "<<key);
            continue;
            /// Keep it for this iteration
            // return StatusCode::FAILURE;
        }
        /// Use for the debugging only the BML phi 3 station, eta = 1 station as it's mounted on the top
        if (m_idHelperSvc->stationNameString(elementID) != "BML" ||
            m_idHelperSvc->stationEta(elementID) != 1 ||
            m_idHelperSvc->stationPhi(elementID) != 3) continue;

        
        defineArgs define{};
        define.physVol = pv;
        define.chambDesign = key_tokens[1];
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol, alignedNodes);
        define.detElId = elementID;
        ATH_MSG_VERBOSE("Key  "<<key<<" lead to Identifier "<<m_idHelperSvc->toStringDetEl(elementID));
        ATH_CHECK(loadDimensions(define, facCache));
        std::unique_ptr<RpcReadoutElement> readoutEle = std::make_unique<RpcReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addRpcReadoutElement(std::move(readoutEle)));
    }    
    return StatusCode::SUCCESS;
}
StatusCode RpcReadoutGeomTool::readParameterBook(FactoryCache& cache) {
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(), name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WRPC", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    
    for (const IRDBRecord* record : *paramTable) {
        const std::string chambType = record->getString("WRPC_TYPE");
        wRPCTable& parBook = cache.parameterBook[record->getString("WRPC_TYPE")];
        parBook.stripPitchEta = record->getDouble("pitch_z") * Gaudi::Units::cm;
        parBook.stripPitchPhi = record->getDouble("pitch_s") * Gaudi::Units::cm;
        const double stripDeadWidth = record->getDouble("stripdeadsep") * Gaudi::Units::cm;
        parBook.stripWidthEta = parBook.stripPitchEta - stripDeadWidth;
        parBook.stripWidthPhi = parBook.stripPitchPhi - stripDeadWidth;
        parBook.numEtaStrips = record->getInt("n_strips_z");
        parBook.numPhiStrips = record->getInt("n_strips_s");

    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4