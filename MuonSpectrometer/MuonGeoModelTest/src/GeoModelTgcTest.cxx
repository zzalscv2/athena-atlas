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


namespace MuonGM {

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
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string BIL1A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
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
    m_readoutTransform.push_back(Amg::Vector3D(trans.translation()));
    m_readoutTransform.push_back(Amg::Vector3D(trans.linear()*Amg::Vector3D::UnitX()));
    m_readoutTransform.push_back(Amg::Vector3D(trans.linear()*Amg::Vector3D::UnitY()));
    m_readoutTransform.push_back(Amg::Vector3D(trans.linear()*Amg::Vector3D::UnitZ()));

   const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
   if (station->hasALines()){ 
        m_ALineTransS = station->getALine_tras();
        m_ALineTransT = station->getALine_traz();
        m_ALineTransZ = station->getALine_trat();
        m_ALineRotS   = station->getALine_rots();
        m_ALineRotT   = station->getALine_rotz();
        m_ALineRotZ   = station->getALine_rott();
    }

    for (bool measPhi : {false, true}) {        
        for (int layer = 1 ; layer <= readoutEle->numberOfLayers(measPhi); ++layer){

            const Identifier layerId = idHelper.channelID(readoutEle->identify(),layer, measPhi,1);
            const Amg::Transform3D layerTransform = readoutEle->localToGlobalTransf(layerId);
            m_layCenter.push_back(Amg::Vector3D(layerTransform.translation()));
            m_layTransColX.push_back(Amg::Vector3D(layerTransform.linear()*Amg::Vector3D::UnitX()));
            m_layTransColY.push_back(Amg::Vector3D(layerTransform.linear()*Amg::Vector3D::UnitY()));
            m_layTransColZ.push_back(Amg::Vector3D(layerTransform.linear()*Amg::Vector3D::UnitZ()));
            m_layMeasPhi.push_back(measPhi);
            m_layNumber.push_back(layer);            
            
            if (measPhi) {
                for (int strip = 1; strip<= readoutEle->getNStrips(layer); ++strip) {
                    bool is_valid{false};
                    const Identifier stripId = idHelper.channelID(readoutEle->identify(), layer, measPhi, strip, is_valid);
                    if (!is_valid) continue;
                    Amg::Vector3D globStripPos = readoutEle->channelPos(stripId);
                    m_stripCenter.push_back(globStripPos);
                    m_stripGasGap.push_back(layer);
                    m_stripNum.push_back(strip);
                }
            } else {                
                for (int gang = 1; gang <= readoutEle->nGangs(layer); ++gang) {
                       m_gangCenter.push_back(readoutEle->gangPos(layer, gang));
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


}
