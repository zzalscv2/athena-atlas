/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMdtTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
namespace MuonGM {
GeoModelMdtTest::GeoModelMdtTest(const std::string& name,
                                 ISvcLocator* pSvcLocator)
    : AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelMdtTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());

    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};

    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format give " << testCham);
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
            ATH_MSG_FATAL("Failed to deduce a station name for " << statName);
            return StatusCode::FAILURE;
        }
        m_testStations.insert(eleId);
        /// Add the second multilayer if possible
        const Identifier secMl = id_helper.multilayerID(eleId, 2, is_valid);
        if (is_valid)
            m_testStations.insert(secMl);
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }

    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    std::stringstream sstr{};
    for (const Identifier& test_me : m_testStations) {
        const int ml = id_helper.multilayer(test_me);
        const std::string detStr = m_idHelperSvc->toStringDetEl(test_me);
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " << detStr);
        const MdtReadoutElement* reElement =
            detMgr->getMdtReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL(
                "Expected to retrieve "
                << detStr << ". But got instead "
                << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
        sstr << "##############################################################"
                "########################"
             << std::endl;
        sstr << "Found Readout element " << detStr << std::endl;
        sstr << "##############################################################"
                "########################"
             << std::endl;
        /// location
        const Amg::Transform3D& localToGlob{reElement->transform()};
        sstr << "Displacement:       "
             << Amg::toString(localToGlob.translation(), 3) << std::endl;
        sstr << "x-Axis orientation: "
             << Amg::toString(localToGlob.linear() * Amg::Vector3D::UnitX(), 3)
             << std::endl;
        sstr << "y-Axis orientation: "
             << Amg::toString(localToGlob.linear() * Amg::Vector3D::UnitY(), 3)
             << std::endl;
        sstr << "z-Axis orientation: "
             << Amg::toString(localToGlob.linear() * Amg::Vector3D::UnitZ(), 3)
             << std::endl;
        sstr << "Number of layers: " << reElement->getNLayers()
             << ", number of tubes: " << reElement->getNtubesperlayer()
             << std::endl;
        for (int lay = 1; lay <= reElement->getNLayers(); ++lay) {
            for (int tube = 1; tube <= reElement->getNtubesperlayer(); ++tube) {
                const Identifier tube_id =
                    id_helper.channelID(test_me, ml, lay, tube);
                sstr << " *** (" << std::setfill('0') << std::setw(2) << lay
                     << ", " << std::setfill('0') << std::setw(3) << tube
                     << ")    " 
                     << Amg::toString(reElement->ROPos(tube_id), 3)<<" / "
                     << Amg::toString(reElement->localROPos(tube_id), 3)<<"  --> "
                     << Amg::toString(reElement->tubePos(tube_id), 3) << " / "
                     << Amg::toString(reElement->localTubePos(tube_id), 3)
                     << std::endl;
            }
        }
    }

    if (!m_outputTxt.empty()) {
        std::fstream outStream{m_outputTxt, std::ios_base::out};
        if (!outStream.good()) {
            ATH_MSG_FATAL("Failed to create output file " << m_outputTxt);
            return StatusCode::FAILURE;
        }
        outStream << sstr.str();
    }
    ATH_MSG_ALWAYS(std::endl << sstr.str());
    return StatusCode::SUCCESS;
}

}  // namespace MuonGM