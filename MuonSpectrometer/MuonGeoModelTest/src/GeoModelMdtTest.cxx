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
    ATH_CHECK(m_deadChanKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    if (m_dumpTree) ATH_CHECK(m_tree.init(this));

    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};

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
        /// Add the second multilayer if possible
        const Identifier secMl = id_helper.multilayerID(eleId, 2, is_valid);
        if (is_valid)
            m_testStations.insert(secMl);
    }
    /// Add all stations for testing
    if (m_testStations.empty()){
        for(auto itr = id_helper.detectorElement_begin();
                 itr!= id_helper.detectorElement_end();++itr){
           if (!id_helper.isBMG(*itr)) m_testStations.insert(*itr);
        }
    }
    return StatusCode::SUCCESS;
}
const MdtCondDbData* GeoModelMdtTest::retrieveDeadChannels(const EventContext& ctx ) const {
    if (m_deadChanKey.empty()) return nullptr;
    SG::ReadCondHandle<MdtCondDbData> deadChanHandle{m_deadChanKey,ctx};
    if (!deadChanHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve Mdt conditions "<<m_deadChanKey.fullKey());
        throw std::runtime_error("No dead channels found");
    }
    return deadChanHandle.cptr();    
}
StatusCode GeoModelMdtTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    const MdtCondDbData* deadChan{retrieveDeadChannels(ctx)};
    
    std::optional<std::fstream> outStream{};
    if (!m_outputTxt.empty()) {
        outStream = std::make_optional<std::fstream>(m_outputTxt, std::ios_base::out);
        if (!outStream->good()) {
            ATH_MSG_FATAL("Failed to create output file " << m_outputTxt);
            return StatusCode::FAILURE;
        }
    }
    for (const Identifier& test_me : m_testStations) {
        const std::string detStr = m_idHelperSvc->toStringDetEl(test_me);
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " << detStr);
        if (deadChan && !deadChan->isGoodStation(test_me)) {
            ATH_MSG_VERBOSE("Dead station found " << detStr);
            continue;
        }
        const MdtReadoutElement* reElement = detMgr->getMdtReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << detStr << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
        ATH_CHECK(dumpToTree(ctx, reElement));
        if (outStream) dumpToFile(ctx, reElement, *outStream);
        
    }

   
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::finalize() {
    if (m_dumpTree) ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::dumpToTree(const EventContext& ctx, const MdtReadoutElement* readoutEle) {
    if (!m_dumpTree) return StatusCode::SUCCESS;
    m_stIndex = readoutEle->getStationIndex();
    m_stEta = readoutEle->getStationEta();
    m_stPhi = readoutEle->getStationPhi();
    m_stML = readoutEle->getMultilayer();

    m_numTubes = readoutEle->getNtubesperlayer();
    m_numLayers = readoutEle->getNLayers();

    m_tubeRad = readoutEle->innerTubeRadius();
    m_tubePitch = readoutEle->tubePitch();

    const Amg::Transform3D& trans{readoutEle->transform()};
    m_readoutTransform.push_back(trans.translation());
    m_readoutTransform.push_back(trans.linear()*Amg::Vector3D::UnitX());
    m_readoutTransform.push_back(trans.linear()*Amg::Vector3D::UnitY());
    m_readoutTransform.push_back(trans.linear()*Amg::Vector3D::UnitX());
    
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};

    const MdtCondDbData* deadChan{retrieveDeadChannels(ctx)};
    
    for (int lay = 1; lay <= readoutEle->getNLayers(); ++lay) {
        for (int tube = 1; tube <= readoutEle->getNtubesperlayer(); ++tube) {
            bool is_valid{false};
            const Identifier tube_id =
                id_helper.channelID(readoutEle->identify(), 
                                    readoutEle->getMultilayer(), lay, tube, is_valid);
            if (!is_valid) continue;
            if (deadChan && !deadChan->isGood(tube_id)) {
                ATH_MSG_ALWAYS("Dead dube detected "<<m_idHelperSvc->toString(tube_id));
                continue;
            }
            const Trk::SaggedLineSurface& surf{readoutEle->surface(tube_id)};
            if (tube == 1) {
                const Amg::Transform3D layTransf{readoutEle->transform(tube_id)};
                m_layTransNumber.push_back(lay);
                m_layCenter.push_back(layTransf.translation());
                m_layTransColX.push_back(layTransf.linear()* Amg::Vector3D::UnitX());
                m_layTransColY.push_back(layTransf.linear()* Amg::Vector3D::UnitY());
                m_layTransColZ.push_back(layTransf.linear()* Amg::Vector3D::UnitZ());
            }
            const Amg::Vector3D roPos = readoutEle->ROPos(tube_id);
            const Amg::Vector3D tubePos = readoutEle->tubePos(tube_id);
           
            m_tubePos.push_back(tubePos);
            m_roPos.push_back(roPos);            
            m_tubeLay.push_back(lay);
            m_tubeNum.push_back(tube);
            m_activeTubeLength.push_back(readoutEle->getActiveTubeLength(lay,tube));
            m_tubeLength.push_back(readoutEle->tubeLength(tube_id));
            m_wireLength.push_back(readoutEle->getWireLength(lay, tube));
            
            if (!m_dumpSurfaces) continue;
            const Amg::Vector3D globalDir {(tubePos - roPos).unit()};
               
            for (double l = readoutEle->tubeLength(tube_id) /2; l > 0; l = l  -100. ) {
                Amg::Vector2D lPos{Amg::Vector2D::Zero()};
                surf.globalToLocal(roPos + l * globalDir,Amg::Vector3D::Zero(),lPos);
                std::unique_ptr<Trk::StraightLineSurface> sagged{surf.correctedSurface(lPos)};                
                m_layDistTubeLay.push_back(lay);
                m_layDistTubeNum.push_back(tube);
                m_layDistPosAlongWire.push_back(l);
                const Amg::Transform3D layTransf{sagged->transform()};
                m_layDistCenter.push_back(layTransf.translation());
                m_layDistColX.push_back(layTransf.linear()* Amg::Vector3D::UnitX());
                m_layDistColY.push_back(layTransf.linear()* Amg::Vector3D::UnitY());
                m_layDistColZ.push_back(layTransf.linear()* Amg::Vector3D::UnitZ());               
            }
        }
    }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
void GeoModelMdtTest::dumpToFile(const EventContext& ctx, const MdtReadoutElement* reElement, std::ostream& sstr) {
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};    
    const MdtCondDbData* deadChan{retrieveDeadChannels(ctx)};
    
    sstr << "##############################################################################"<< std::endl;
    sstr << "Found Readout element " << m_idHelperSvc->toStringDetEl(reElement->identify()) << std::endl;
    sstr << "##############################################################################"<< std::endl;
    const Amg::Transform3D& localToGlob{reElement->transform()};
    sstr << "Displacement:       "
         << Amg::toString(localToGlob.translation()) << std::endl;
    sstr << "x-Axis orientation: "
         << Amg::toString(localToGlob.linear() * Amg::Vector3D::UnitX()) << std::endl;
    sstr << "y-Axis orientation: "
         << Amg::toString(localToGlob.linear() * Amg::Vector3D::UnitY()) << std::endl;
    sstr << "z-Axis orientation: "
         << Amg::toString(localToGlob.linear() * Amg::Vector3D::UnitZ()) << std::endl;
    sstr << "Number of layers: " << reElement->getNLayers()
         << ", number of tubes: " << reElement->getNtubesperlayer()
         << std::endl;
    for (int lay = 1; lay <= reElement->getNLayers(); ++lay) {
        for (int tube = 1; tube <= reElement->getNtubesperlayer(); ++tube) {
            bool is_valid{false};
            const Identifier tube_id = id_helper.channelID(reElement->identify(), 
                                            reElement->getMultilayer(), lay, tube, is_valid);
            if (!is_valid) continue;
            if (deadChan && !deadChan->isGood(tube_id)) {
                ATH_MSG_ALWAYS("Dead dube detected "<<m_idHelperSvc->toString(tube_id));
                continue;
            }
            if (tube == 1) {
                const Amg::Transform3D layTransf{reElement->transform(tube_id)};
                sstr << "Displacement layer:       "
                    << Amg::toString(layTransf.translation()) << std::endl;
                sstr << "Layer x-axis orientation: "
                     << Amg::toString(layTransf.linear() * Amg::Vector3D::UnitX()) << std::endl;
                sstr << "Layer y-axis orientation: "
                     << Amg::toString(layTransf.linear() * Amg::Vector3D::UnitY()) << std::endl;
                sstr << "Layer z-axis orientation: "
                     << Amg::toString(layTransf.linear() * Amg::Vector3D::UnitZ()) << std::endl;                    
            }
            const Amg::Vector3D roPos{reElement->ROPos(tube_id)},
                                LocRoPos{reElement->localROPos(tube_id)},
                                tubePos{reElement->tubePos(tube_id)},
                                LocTubePos{reElement->localTubePos(tube_id)};
                
            const Amg::Vector3D globalDir {(tubePos - roPos).unit()};
            sstr << " *** (" << std::setfill('0') << std::setw(2) << lay
                 << ", " << std::setfill('0') << std::setw(3) << tube << ")    "; 
            sstr << Amg::toString(roPos,3)<<" / "
                 << Amg::toString(LocRoPos,3)<<"  --> "
                 << Amg::toString(tubePos,3) << " / "
                 << Amg::toString(LocTubePos,3);
                
                
            sstr<<", activeTube: "<<reElement->getActiveTubeLength(lay,tube);
            sstr<<", tubeLength: "<<reElement->tubeLength(tube_id);
            sstr<<", wireLength: "<<reElement->getWireLength(lay, tube);
            sstr<< std::endl;
            if(m_dumpSurfaces) {
                const Trk::SaggedLineSurface& surf{reElement->surface(lay,tube)};
                sstr <<reElement->bounds(lay,tube)<<std::endl;
                sstr <<surf<<std::endl;
                for (double l = reElement->tubeLength(tube_id) /2; l > 0; l = l  -100. ) {
                    Amg::Vector2D lPos{Amg::Vector2D::Zero()}; 
                    surf.globalToLocal(roPos + l * globalDir,Amg::Vector3D::Zero(),lPos);
                    sstr<<"Local position along tube: "<<Amg::toString(lPos)<<std::endl;
                    std::unique_ptr<Trk::StraightLineSurface> sagged{surf.correctedSurface(lPos)};
                    sstr<<"   "<<(*sagged)<<std::endl;
                }
            }   
        }
    }
}
}  // namespace MuonGM
