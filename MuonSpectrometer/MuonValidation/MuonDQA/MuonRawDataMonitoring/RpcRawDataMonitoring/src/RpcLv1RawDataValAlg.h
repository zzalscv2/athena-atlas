/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//////////////////////////////////////////////////////////////////////////////////////////////
// Package : RpcRawDataMonitoring
// Author:   N. Benekos(Illinois) - M. Bianco(INFN-Lecce) - G.
// Chiodini(INFN-Lecce) Sept. 2007
//
// DESCRIPTION:
// Subject: RPCLV1-->Offline Muon Data Quality
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RpcLv1RawDataValAlg_H
#define RpcLv1RawDataValAlg_H

#include <map>
#include <sstream>
#include <vector>

#include "AthenaMonitoring/AthenaMonManager.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/NTuple.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonDQAUtils/MuonDQAHistMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/RpcCoinMatrix.h"
#include "MuonRDO/RpcFiredChannel.h"
#include "MuonRDO/RpcPad.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonRDO/RpcSectorLogicContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutSet.h"
#include "RPC_CondCabling/RpcCablingCondData.h"
#include "RpcGlobalUtilities.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

class TFile;
/////////////////////////////////////////////////////////////////////////////

class RpcLv1RawDataValAlg : public ManagedMonitorToolBase {
   public:
    RpcLv1RawDataValAlg(const std::string& type, const std::string& name,
                        const IInterface* parent);
    virtual ~RpcLv1RawDataValAlg() = default;
    StatusCode initialize();

    virtual StatusCode bookHistogramsRecurrent();
    virtual StatusCode fillHistograms();
    virtual StatusCode procHistograms();

   private:
    // Function for histogram booking and parameterd for fitting
    StatusCode bookRPCLV1cmatimevschHistograms(
        const std::string& m_sectorlogic_name, const std::string& m_tower_name,
        const std::string& m_cma_name);
    StatusCode bookRPCLV1TriggerRoadHistograms(
        const std::string& m_sectorlogic_name, const std::string& m_tower_name,
        const std::string& m_cma_name, const std::string& m_thr_name);
    StatusCode bookRPCLV1ProfilesHistograms(
        int m_i_sector, const std::string& m_sectorlogic_name,
        const std::string& m_cma_name, int m_i_ijk,
        const std::string& m_ijk_name);

    MuonDQAHistMap m_stationHists;

    StatusCode StoreTriggerType();
    int GetTriggerType() const { return m_trigtype; }
    int m_trigtype{0};

    int m_sector{0};
    int m_side{0};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo",
                                                   "EventInfo", "event info"};
    SG::ReadHandleKey<RpcPadContainer> m_rpcRdoKey{this, "RpcRdo", "RPCPAD",
                                                   "RPC RDO"};

    std::vector<std::string> m_sectorlogicTowerCma_name_list{};
    std::vector<std::string> m_sectorlogicTowerCma_name_list2{};
    std::vector<std::string> m_profile_list{};

    bool m_doClusters{false};
    bool m_checkCabling{false};
    bool m_rpclv1file{false};
    bool m_rpclv1hist{false};
    bool m_rpclv1prof{false};
    int m_rpclv1reducenbins{false};

    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadCondHandleKey<RpcCablingCondData> m_readKey{
        this, "ReadKey", "RpcCablingCondData", "Key of RpcCablingCondData"};

    void bookRPCCoolHistograms(std::vector<std::string>::const_iterator& m_iter,
                               int, int, const std::string& m_layer);
    bool m_doCoolDB{false};

    std::string m_chamberName{};
    std::string m_StationSize{};
    int m_StationEta{0};
    int m_StationPhi{0};
    int m_lastEvent{0};
    int m_cosmicStation{0};
};

#endif
