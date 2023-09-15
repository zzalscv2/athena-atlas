/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMmTest.h"

#include <fstream>
#include <iostream>


#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"

namespace MuonGM {

GeoModelMmTest::GeoModelMmTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelMmTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));

    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
    if (m_testStations.empty()) {
        m_testStations.insert(id_helper.detectorElement_begin(),
                              id_helper.detectorElement_end());
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::execute() {

    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    //Looping through the MicroMegas identifiers and pushing back the respective roe.
    for (const Identifier& detId : m_testStations) {
        const MuonGM::MMReadoutElement *roe = detMgr->getMMReadoutElement(detId);
        if (roe) {
            ATH_CHECK(dumpToTree(ctx,roe));
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode GeoModelMmTest::dumpToTree(const EventContext& ctx, const MuonGM::MMReadoutElement* roEl) {

    const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
    const Identifier id = roEl->identify();
    const int multlayer = id_helper.multilayer(id);
    m_stationEta = id_helper.stationEta(id);
    m_stationPhi = id_helper.stationPhi(id);
    m_stationName = id_helper.stationName(id);
    m_multilayer = multlayer;
    for (int gasgap = 1; gasgap <= 4; ++gasgap) {

        Identifier layerId = id_helper.channelID(id, multlayer, gasgap, 1024);
        
        for (int channel = id_helper.channelMin(layerId);
                 channel < id_helper.channelMax(layerId); channel++) {

            bool is_valid{false};
            const Identifier strip_id = id_helper.channelID(id, multlayer, 
                                                            gasgap, channel, is_valid);
            if (!is_valid) continue;

               
            //If the strip number is outside the range of valid strips, the function will return false
            //this method also assignes strip center global coordinates to the gp vector.

            //Strip global points
            auto global_points = [&roEl,&id_helper](const Identifier& id, 
                                                    Amg::Vector3D& left,
                                                    Amg::Vector3D& center, 
                                                    Amg::Vector3D& right) {
                Amg::Vector2D l_cen{Amg::Vector2D::Zero()}, 
                              l_left{Amg::Vector2D::Zero()},
                              l_right{Amg::Vector2D::Zero()};

                const MuonGM::MuonChannelDesign* design = roEl->getDesign(id);

                const int chan = id_helper.channel(id);
                design->leftEdge(chan, l_left);
                design->center(chan, l_cen);
                design->rightEdge(chan, l_right);

                roEl->surface(id).localToGlobal(l_left, Amg::Vector3D::Zero(), left);
                roEl->surface(id).localToGlobal(l_cen, Amg::Vector3D::Zero(), center);
                roEl->surface(id).localToGlobal(l_right, Amg::Vector3D::Zero(), right);

            };

            Amg::Vector3D strip_center{Amg::Vector3D::Zero()},
                          strip_leftEdge{Amg::Vector3D::Zero()},
                          strip_rightEdge{Amg::Vector3D::Zero()};

            global_points(strip_id, strip_leftEdge, strip_center, strip_rightEdge);                    
            
            m_gasGap.push_back(id_helper.gasGap(strip_id));
            m_channel.push_back(id_helper.channel(strip_id));
            m_stripCenter.push_back(strip_center);
            m_stripLeftEdge.push_back(strip_leftEdge);
            m_stripRightEdge.push_back(strip_rightEdge);
            m_stripLength.push_back(roEl->stripLength(strip_id));
            m_stripActiveLength.push_back(roEl->stripActiveLength(strip_id));
            m_stripActiveLengthLeft.push_back(roEl->stripActiveLengthLeft(strip_id));
            m_stripActiveLengthRight.push_back(roEl->stripActiveLengthRight(strip_id));

        }
    }
    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}



}