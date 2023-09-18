/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelsTgcTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"

namespace MuonGM {

GeoModelsTgcTest::GeoModelsTgcTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelsTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string STL3A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
        bool is_valid{false};
        const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        std::copy_if(id_helper.detectorElement_begin(), 
                     id_helper.detectorElement_end(), 
                     std::inserter(m_testStations, m_testStations.end()), 
                        [&](const Identifier& id) {
                            return id_helper.elementID(id) == eleId;
                        });
    }
    /// Add all stations for testing if nothing has been specified
    if (m_testStations.empty()){
        std::copy(id_helper.detectorElement_begin(), 
                  id_helper.detectorElement_end(), 
                  std::inserter(m_testStations, m_testStations.end()));
    } else {
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations){
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of sTgc detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const sTgcReadoutElement* reElement = detMgr->getsTgcReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (m_idHelperSvc->toStringDetEl(reElement->identify()) != m_idHelperSvc->toStringDetEl(test_me)) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << m_idHelperSvc->toStringDetEl(test_me) << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
        ATH_CHECK(dumpToTree(ctx, reElement));
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::dumpToTree(const EventContext& ctx, const sTgcReadoutElement* readoutEle) {
    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};

//// Identifier of the readout element
    int stIndex    = readoutEle->getStationIndex();
    int stEta      = readoutEle->getStationEta();
    int stPhi      = readoutEle->getStationPhi();
    int stML      = id_helper.multilayer(readoutEle->identify());

    m_stIndex = stIndex;
    m_stEta = stEta;
    m_stPhi = stPhi;
    m_stML = stML;

    const Identifier genWireID =id_helper.channelID(stIndex, stEta, stPhi, 
                    stML, 1, sTgcIdHelper::sTgcChannelTypes::Wire, 1);
    const Identifier genStripID =id_helper.channelID(stIndex, stEta, stPhi, 
                    stML, 1, sTgcIdHelper::sTgcChannelTypes::Strip, 1);
    const Identifier genPadID =id_helper.channelID(stIndex, stEta, stPhi, 
                    stML, 1, sTgcIdHelper::sTgcChannelTypes::Pad, 1);

//// Chamber Details from sTGCDetectorDescription 
    int numLayers = readoutEle->numberOfLayers(true); 
    double yCutout = readoutEle->getDesign(genStripID)->yCutout();
    double activeHeight = readoutEle->getDesign(genStripID)->xSize();
    double gasTck = readoutEle->getDesign(genStripID)->thickness;

    m_numLayers = numLayers;
    m_yCutout = yCutout;
    m_activeHeight = activeHeight; 
    m_gasTck = gasTck;

/// Transformation of the readout element (Translation, ColX, ColY, ColZ) 
    const Amg::Transform3D& trans{readoutEle->transform()};
    m_readoutTransform.push_back(Amg::Vector3D(trans.translation()));
    m_readoutTransform.push_back(Amg::Vector3D(trans.linear()*Amg::Vector3D::UnitX()));
    m_readoutTransform.push_back(Amg::Vector3D(trans.linear()*Amg::Vector3D::UnitY()));
    m_readoutTransform.push_back(Amg::Vector3D(trans.linear()*Amg::Vector3D::UnitZ()));

//// All the Vectors
    for (int lay = 1; lay <= numLayers; ++lay) {
        const Identifier layWireID =id_helper.channelID(stIndex, stEta, stPhi, 
                        stML, lay, sTgcIdHelper::sTgcChannelTypes::Wire, 1);
        const Identifier layStripID =id_helper.channelID(stIndex, stEta, stPhi, 
                        stML, lay, sTgcIdHelper::sTgcChannelTypes::Strip, 1);
        const Identifier layPadID =id_helper.channelID(stIndex, stEta, stPhi, 
                        stML, lay, sTgcIdHelper::sTgcChannelTypes::Pad, 1);

//// Wire Dimensions
        int numWires = readoutEle->numberOfWires(layWireID); 
        int firstWireGroupWidth = readoutEle->getDesign(layWireID)->firstPitch;
        int numWireGroups = readoutEle->getDesign(layWireID)->nGroups;
        int wireCutout = readoutEle->getDesign(layWireID)->wireCutout;
        double wirePitch = readoutEle->wirePitch(); 
        double wireWidth = readoutEle->getDesign(layWireID)->inputWidth;
        double wireGroupWidth = readoutEle->getDesign(layWireID)->groupWidth;

        m_numWires.push_back(numWires);
        m_firstWireGroupWidth.push_back(firstWireGroupWidth);
        m_numWireGroups.push_back(numWireGroups);
        m_wireCutout.push_back(wireCutout);
        m_wirePitch = wirePitch;
        m_wireWidth = wireWidth;
        m_wireGroupWidth = wireGroupWidth;

//// Global transformations for wires
        for (int wireGroupIndex = 1; wireGroupIndex <= numWireGroups; ++wireGroupIndex) {
            bool isValid = false;
            const Identifier wireGroupID =id_helper.channelID(stIndex, stEta, stPhi, stML, lay, 
                            sTgcIdHelper::sTgcChannelTypes::Wire, wireGroupIndex, isValid);
            if(!isValid) {
                ATH_MSG_WARNING("The following wire group ID is not valid: " << wireGroupID);
            }
            Amg::Vector3D wireGroupPosition(Amg::Vector3D::Zero());
            
            readoutEle->stripGlobalPosition(wireGroupID, wireGroupPosition);
            m_globalWireGroupPositions.push_back(wireGroupPosition);
            m_wireGroupNum.push_back(wireGroupIndex);
            m_wireGroupGasGap.push_back(lay);
        }

////Strip Dimensions
        int numStrips = readoutEle->getDesign(layStripID)->nch;
        double stripPitch = readoutEle->channelPitch(layStripID);
        double stripWidth = readoutEle->getDesign(layStripID)->inputWidth;
        int firstStripPitch = readoutEle->getDesign(layStripID)->firstPitch;
        
        m_numStrips = numStrips;
        m_stripPitch = stripPitch;
        m_stripWidth = stripWidth;
        m_firstStripPitch.push_back(firstStripPitch);

//// Global transformations for strips
        for (int stripIndex = 1; stripIndex <= numStrips; ++stripIndex) {
            bool isValid = false;
            const Identifier stripID =id_helper.channelID(stIndex, stEta, stPhi, stML, lay, 
                            sTgcIdHelper::sTgcChannelTypes::Strip, stripIndex, isValid);
            if(!isValid) {
                ATH_MSG_WARNING("The following strip ID is not valid: " << stripID);
            }
            double stripLength = readoutEle->getDesign(stripID)->channelLength(stripIndex);
            Amg::Vector3D stripPosition(Amg::Vector3D::Zero());

            readoutEle->stripGlobalPosition(stripID, stripPosition);
            m_globalStripPositions.push_back(stripPosition);
            m_stripNum.push_back(stripIndex);
            m_stripGasGap.push_back(lay);
            m_stripLengths.push_back(stripLength);
        }

////Pad Dimensions
        int numPads = readoutEle->numberOfPads(layPadID);
        int numPadEta = readoutEle->getPadDesign(layPadID)->nPadH;
        int numPadPhi = readoutEle->getPadDesign(layPadID)->nPadColumns;
                
        m_numPads.push_back(numPads);
        m_numPadEta.push_back(numPadEta);
        m_numPadPhi.push_back(numPadPhi);
        
//// Global transformations for pads
        for (int phiIndex = 1; phiIndex <= numPadPhi; ++phiIndex) {
            for(int etaIndex = 1; etaIndex <= numPadEta; ++etaIndex) {
                bool isValid = false;
                const Identifier padID =id_helper.padID(stIndex, stEta, stPhi, stML, lay, 
                    sTgcIdHelper::sTgcChannelTypes::Pad, etaIndex, phiIndex, isValid);
                if(!isValid) {
                    ATH_MSG_WARNING("The following pad ID is not valid: " << padID);
                }
                Amg::Vector3D padPosition(Amg::Vector3D::Zero());
                std::array<Amg::Vector3D,4> padCorners{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};

                readoutEle->padGlobalPosition(padID, padPosition);
                readoutEle->padGlobalCorners(padID, padCorners);
               
                m_globalPadPositions.push_back(padPosition);
                m_globalPadCornerBL.push_back(padCorners[0]);
                m_globalPadCornerBR.push_back(padCorners[1]);
                m_globalPadCornerTL.push_back(padCorners[2]);
                m_globalPadCornerTR.push_back(padCorners[3]);

                m_padEta.push_back(etaIndex);
                m_padPhi.push_back(phiIndex);
                m_padGasGap.push_back(lay);
            }

        }

    }

    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}
