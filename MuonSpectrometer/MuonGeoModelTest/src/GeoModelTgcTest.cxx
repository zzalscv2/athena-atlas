/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelTgcTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"


namespace MuonGM {

struct TgcChamberLayout {
    Identifier gasGap{};
    std::vector<double> botStripPos{};
    std::vector<double> topStripPos{};
    std::vector<unsigned int> wireGangLayout{};
    double wirePitch{0.};
};


GeoModelTgcTest::GeoModelTgcTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    const TgcIdHelper& id_helper{m_idHelperSvc->tgcIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 7) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string BIL1A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 2).c_str());
        bool is_valid{false};
        const Identifier eleId =
            id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        m_testStations.insert(eleId);
    }
    /// Add all stations for testing
    if (m_testStations.empty()){
        for(auto itr = id_helper.detectorElement_begin();
                 itr!= id_helper.detectorElement_end();++itr){
            m_testStations.insert(*itr);
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    dumpReadoutXML(**detMgr);
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const TgcReadoutElement* reElement = detMgr->getTgcReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << m_idHelperSvc->toStringDetEl(test_me) << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
        if (reElement->center().z() > 0. && reElement->center().z() < 7.5 * Gaudi::Units::m ) {
            const Identifier prevId = reElement->getStationPhi() > 1 ? m_idHelperSvc->tgcIdHelper().elementID(m_idHelperSvc->stationNameString(test_me),
                                                                             reElement->getStationEta(),
                                                                             reElement->getStationPhi() - 1) : test_me;
            const TgcReadoutElement* prevRE = detMgr->getTgcReadoutElement(prevId);
            const Amg::Vector3D center = reElement->center();
            ATH_MSG_ALWAYS("Tgc element "<<m_idHelperSvc->toString(reElement->identify())
                         <<" position "<<Amg::toString(center, 2)
                         <<" perp: "<<center.perp()
                         <<" phi: "<<(center.phi() / Gaudi::Units::deg)
                         <<" theta: "<<(center.theta() / Gaudi::Units::deg)
                         <<" rSize: "<<reElement->getRsize()<<"/"<<reElement->getLongRsize()
                         <<" sSize: "<<reElement->getSsize()<<"/"<<reElement->getLongSsize()
                         <<" zSize: "<<reElement->getZsize()<<"/"<<reElement->getLongZsize()
                         <<" dPhi: "<<(prevRE->center().deltaPhi(center) / Gaudi::Units::deg));
        }
        ATH_CHECK(dumpToTree(ctx, reElement));
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::dumpToTree(const EventContext& ctx, const TgcReadoutElement* readoutEle) {
    m_stIndex = readoutEle->getStationIndex();
    m_stEta   = readoutEle->getStationEta();
    m_stPhi   = readoutEle->getStationPhi();
    ATH_MSG_DEBUG("Dump readout element "<<m_idHelperSvc->toString(readoutEle->identify()));

    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};

    const Amg::Transform3D& trans{readoutEle->transform()};
    m_readoutTransform = trans;

   const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
   if (station->hasALines()){ 
        m_ALineTransS = station->getALine_tras();
        m_ALineTransT = station->getALine_traz();
        m_ALineTransZ = station->getALine_trat();
        m_ALineRotS   = station->getALine_rots();
        m_ALineRotT   = station->getALine_rotz();
        m_ALineRotZ   = station->getALine_rott();
    }

    for (bool isStrip : {false, true}) {        
        for (int layer = 1 ; layer <= readoutEle->numberOfLayers(isStrip); ++layer){
            const Identifier layerId = idHelper.channelID(readoutEle->identify(),layer, isStrip,1);
            m_layTans.push_back(readoutEle->localToGlobalTransf(layerId));
            m_layMeasPhi.push_back(isStrip);
            m_layNumber.push_back(layer);
            
            if (isStrip) {
                for (int strip = 1; strip < readoutEle->getNStrips(layer); ++strip) {
                    bool is_valid{false};
                    const Identifier stripId = idHelper.channelID(readoutEle->identify(), layer, isStrip, strip, is_valid);
                    if (!is_valid) continue;
                    
                    /// Strip center
                    const Amg::Vector3D globStripPos = readoutEle->channelPos(stripId);
                    Amg::Vector2D locStripPos{Amg::Vector2D::Zero()};
                    const Trk::Surface& surf{readoutEle->surface(stripId)};
                    if (!surf.globalToLocal(globStripPos, Amg::Vector3D::Zero(), locStripPos)){
                        ATH_MSG_FATAL("Failed to build local strip position "<<m_idHelperSvc->toString(stripId));
                        return StatusCode::FAILURE;
                    }
                    m_locStripCenter.push_back(locStripPos);
                    m_stripCenter.push_back(globStripPos);
                    /// Strip bottom & top edges
                    const double stripHalfLength = readoutEle->stripLength(layer, strip) / 2.;

                    const Amg::Vector2D locStripBot{readoutEle->getStripPositionOnShortBase(strip), -stripHalfLength};                    
                    const Amg::Vector2D locStripTop{readoutEle->getStripPositionOnLargeBase(strip), stripHalfLength};
                    const Amg::Vector3D globStripBot{surf.localToGlobal(locStripBot)};
                    const Amg::Vector3D globStripTop{surf.localToGlobal(locStripTop)};
                    
                    m_stripBottom.push_back(globStripBot);
                    m_stripTop.push_back(globStripTop);
    
                    m_locStripBottom.push_back(locStripBot);
                    m_locStripTop.push_back(locStripTop);
  
                    m_stripGasGap.push_back(layer);
                    m_stripNum.push_back(strip);
                    m_stripLength.push_back(stripHalfLength * 2.);
                    m_stripPitch.push_back(readoutEle->stripPitch(layer,strip));
                    m_stripShortWidth.push_back(readoutEle->stripShortWidth(layer, strip));
                    m_stripLongWidth.push_back(readoutEle->stripLongWidth(layer, strip));
                }
            } else {
                for (int gang = 1; gang < readoutEle->nGangs(layer); ++gang) {
                       const Identifier gangId{idHelper.channelID(readoutEle->identify(), layer, isStrip, gang)};
                       const Trk::Surface& surf{readoutEle->surface(gangId)};
                       const Amg::Vector3D globPos{readoutEle->gangPos(layer, gang)};
                       Amg::Vector2D locPos{Amg::Vector2D::Zero()};
                       if (!surf.globalToLocal(globPos,Amg::Vector3D::Zero(),locPos)) {
                           ATH_MSG_FATAL("Failed to extract local position "<<m_idHelperSvc->toString(gangId));
                           return StatusCode::FAILURE;
                       }
                       m_locGangPos.push_back(locPos);
                       m_gangCenter.push_back(globPos);
                       m_gangGasGap.push_back(layer);
                       m_gangNum.push_back(gang);
                       m_gangNumWires.push_back(readoutEle->getNWires(layer, gang));
                       m_gangLength.push_back(readoutEle->gangLength(layer,gang));
                }
            }
        }    
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

void GeoModelTgcTest::dumpReadoutXML(const MuonGM::MuonDetectorManager& detMgr) {
    if (m_readoutXML.empty()) {
        return;
    }
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    for (TgcIdHelper::const_id_iterator itr = idHelper.module_begin();
                                        itr != idHelper.module_end(); ++itr) {
        const MuonGM::TgcReadoutElement* reEle = detMgr.getTgcReadoutElement(*itr);
        if (!reEle) continue;
        for (bool isStrip : {false, true}) {        
            for (int layer = 1 ; layer <= reEle->numberOfLayers(isStrip); ++layer){
                const Identifier layerId [[maybe_unused]] = idHelper.channelID(reEle->identify(), layer, isStrip, 1);
            }
        }

    }
}


}
