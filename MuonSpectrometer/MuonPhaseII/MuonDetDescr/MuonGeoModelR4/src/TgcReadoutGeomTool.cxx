/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/TgcReadoutGeomTool.h>

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoBox.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <MuonReadoutGeometryR4/WireGroupDesign.h>
#include <MuonReadoutGeometryR4/RadialStripDesign.h>

#include <RDBAccessSvc/IRDBRecord.h>

using namespace ActsTrk;
namespace {
    constexpr double tolerance = 0.001 * Gaudi::Units::mm;
}

namespace MuonGMR4 {

using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;
using defineArgs = TgcReadoutElement::defineArgs;


TgcReadoutGeomTool::TgcReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);

}
StatusCode TgcReadoutGeomTool::initialize() {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoUtilTool.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode TgcReadoutGeomTool::loadDimensions(TgcReadoutElement::defineArgs& define,
                                              FactoryCache& factoryCache) {
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }

    if (shape->typeID() != GeoTrd::getClassTypeID()) {
        ATH_MSG_FATAL("The shape of "<<m_idHelperSvc->toStringDetEl(define.detElId)
                    <<" is expected to be a trapezoid "<<m_geoUtilTool->dumpShape(shape));
        return StatusCode::FAILURE;
    }
    const GeoTrd* chambTrd = static_cast<const GeoTrd*>(shape);
    define.halfWidthShort = std::min(chambTrd->getYHalfLength1(), chambTrd->getYHalfLength2());
    define.halfWidthLong = std::max(chambTrd->getYHalfLength1(), chambTrd->getYHalfLength2());
    define.halfHeight = chambTrd->getZHalfLength();
    define.halfThickness = chambTrd->getXHalfLength1();
    /// Navigate through the GeoModel tree to find all gas volume leaves
    std::vector<physVolWithTrans> allGasGaps = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "tgcGas");
    if (allGasGaps.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any childern StripLayer"
            <<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
        return StatusCode::FAILURE;
    }
    unsigned int gasGap{0};
    for (const physVolWithTrans& pVolTrans : allGasGaps) {
        std::stringstream key{};
        key<<define.chambDesign<<"_"<<(gasGap+1);

        StripLayerPtr& stripLayout{factoryCache.stripDesigns[key.str()]};
        StripLayerPtr& wireLayout{factoryCache.wireDesigns[key.str()]};
        
        if (!stripLayout || !wireLayout) {
            const wTgcTable& table{factoryCache.parameterBook[key.str()]};
            if (!table.gasGap) {
                ATH_MSG_FATAL("No wTGC table could be found for "
                        <<m_idHelperSvc->toStringDetEl(define.detElId)
                        <<" "<<define.chambDesign<<", gasGap "<<(gasGap+1));
                return StatusCode::FAILURE;
            }
            const GeoShape* gapShape = m_geoUtilTool->extractShape(pVolTrans.physVol);
            if (gapShape->typeID() != GeoTrd::getClassTypeID()) {
                ATH_MSG_FATAL("Expected shape "<<m_geoUtilTool->dumpShape(gapShape)
                            <<" to be a trapezoid");
                return StatusCode::FAILURE;
            }
            const GeoTrd* gapTrd = static_cast<const GeoTrd*>(gapShape);

            const double halfMinX = std::min(gapTrd->getYHalfLength1(), gapTrd->getYHalfLength2());
            const double halfMaxX = std::max(gapTrd->getYHalfLength1(), gapTrd->getYHalfLength2());
            const double halfY = gapTrd->getZHalfLength();

            if (!wireLayout && table.wireGangs.size()) {
                std::shared_ptr<WireGroupDesign> wireGrp = std::make_shared<WireGroupDesign>();
                wireGrp->defineTrapezoid(halfMinX, halfMaxX, halfY);
                for (unsigned int gang : table.wireGangs) {
                    wireGrp->declareGroup(gang); 
                }
                const double wireOffSet = -0.5*table.wirePitch * wireGrp->nAllWires();
                wireGrp->defineStripLayout(Amg::Vector2D{wireOffSet, 0.},
                                           table.wirePitch, 0., table.wireGangs.size());
                
                const Amg::Transform3D trans{pVolTrans.transform 
                                             * Amg::getRotateY3D(-90.*Gaudi::Units::deg)
                                             * Amg::getRotateX3D(180.* Gaudi::Units::deg)};
                /// Reserve the first bit for the isStrip property
                const IdentifierHash hash{gasGap << 1};
                StripDesignPtr stripDesign{wireGrp};
                wireLayout = std::make_unique<StripLayer>(trans, stripDesign, hash);
            }
            if (!stripLayout && table.bottomStripPos.size()) {
                std::shared_ptr<RadialStripDesign> radDesign = std::make_shared<RadialStripDesign>();
                radDesign->defineTrapezoid(halfMinX, halfMaxX, halfY);
                radDesign->defineStripLayout(Amg::Vector2D{-halfY,0.},
                                                0.,0.,table.bottomStripPos.size());
                radDesign->flipTrapezoid();
                for (size_t s = 0; s < table.bottomStripPos.size(); ++s) {
                    radDesign->addStrip(table.bottomStripPos.at(s),
                                        table.topStripPos.at(s));
                }
                /// The extra 1 to account for the is Strip true
                const IdentifierHash hash{gasGap<< 1 | 1};
                const Amg::Transform3D trans{pVolTrans.transform 
                                             * Amg::getRotateZ3D(90.* Gaudi::Units::deg)                                            
                                             * Amg::getRotateX3D(90.*Gaudi::Units::deg)};
                StripDesignPtr stripDesign{radDesign};
                stripLayout = std::make_unique<StripLayer>(trans, stripDesign, hash);                
            }
        }
        ++gasGap;
        if (stripLayout) {
            unsigned int stripIdx = static_cast<unsigned>(stripLayout->hash());
            if (stripIdx >= define.sensorLayouts.size()) {
                ATH_MSG_FATAL("The strip index "<<stripIdx<<" is out of range for gasGap "<<gasGap);
                return StatusCode::FAILURE;
            }           
            define.sensorLayouts[stripIdx] = stripLayout;
        }
        if (wireLayout) {
            unsigned int wireIdx = static_cast<unsigned>(wireLayout->hash());
            if (wireIdx >= define.sensorLayouts.size()) {
                ATH_MSG_FATAL("The wire index "<<wireIdx<<" is out of range for gasGap "<<gasGap);
                return StatusCode::FAILURE;
            }
            define.sensorLayouts[wireIdx] = wireLayout;
        }
    }
    define.nGasGaps = gasGap;        
    return StatusCode::SUCCESS;
}
StatusCode TgcReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    
    FactoryCache facCache{};
    ATH_CHECK(readParameterBook(facCache));

    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
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
        ///   <CHAMBERTYPE>_<STATIONNAME>_<STATIONETA>_<STATIONPHI>
        /// where CHAMBERTYPE has to start with TGC
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() < 4 ||
            key_tokens[0].find("TGC") != 0)
            continue;
        
        bool isValid{false};
        const Identifier elementID = idHelper.elementID(key_tokens[1].substr(0,3), atoi(key_tokens[2]), atoi(key_tokens[3]), isValid);
        /// Retrieve first the station Identifier
        if (!isValid){
            ATH_MSG_FATAL("Failed to construct the station Identifier from "<<key);
            return StatusCode::FAILURE;
        }        
        defineArgs define{};
        define.physVol = pv;
        define.detElId = elementID;
        define.chambDesign = key_tokens[0];
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol, alignedNodes);
        ATH_MSG_DEBUG("Key  "<<key<<" lead to Identifier "<<m_idHelperSvc->toStringDetEl(define.detElId));
        ATH_CHECK(loadDimensions(define, facCache));
        std::unique_ptr<TgcReadoutElement> readoutEle = std::make_unique<TgcReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addTgcReadoutElement(std::move(readoutEle)));
    }    
    return StatusCode::SUCCESS;
}
StatusCode TgcReadoutGeomTool::readParameterBook(FactoryCache& cache) {
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(), name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("TgcSensorLayout", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    for (const IRDBRecord* record : *paramTable) {
        const std::string chambType = record->getString("technology");
        const int gasGap = record->getInt("gasGap");
        std::stringstream key{};
        key<<chambType<<"_"<<gasGap;
        wTgcTable& parBook{cache.parameterBook[key.str()]};
        const std::vector<int> wireGangs{tokenizeInt(record->getString("wireGangs"),",")};
        parBook.wireGangs.insert(parBook.wireGangs.end(), wireGangs.begin(), wireGangs.end());

        parBook.bottomStripPos = tokenizeDouble(record->getString("bottomStrips"), ",");
        parBook.topStripPos = tokenizeDouble(record->getString("topStrips"), ",");
        parBook.wirePitch = record->getDouble("wirePitch");
        parBook.gasGap = gasGap;
    }
    ATH_MSG_DEBUG("Read in total "<<cache.parameterBook.size()<<" chamber layouts");
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4