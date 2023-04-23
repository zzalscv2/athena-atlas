/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : RpcLv1RawDataSectorLogic
// Authors:  A. Vest (TU Dresden), C. Rudolph (TU Dresden)
// February 2010
//
// DESCRIPTION:
// Subject: RPCLV1-->Sector Logic Offline Muon Data Quality
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RpcLv1RawDataSectorLogic_H
#define RpcLv1RawDataSectorLogic_H

#include <map>
#include <sstream>
#include <vector>

#include "AthenaMonitoring/AthenaMonManager.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/NTuple.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonDQAUtils/MuonDQAHistMap.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/RpcCoinMatrix.h"
#include "MuonRDO/RpcFiredChannel.h"
#include "MuonRDO/RpcPad.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonRDO/RpcSectorLogicContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutSet.h"
#include "xAODEventInfo/EventInfo.h"

class TFile;
class RpcIdHelper;
/////////////////////////////////////////////////////////////////////////////

class RpcLv1RawDataSectorLogic : public ManagedMonitorToolBase {
   public:
    RpcLv1RawDataSectorLogic(const std::string& type, const std::string& name,
                             const IInterface* parent);
    virtual ~RpcLv1RawDataSectorLogic() = default;
    StatusCode initialize();

    virtual StatusCode bookHistogramsRecurrent();
    virtual StatusCode fillHistograms();

   private:
    // Private functions and members and histograms

    StatusCode StoreTriggerType();
    int GetTriggerType() const { return m_trigtype; }
    int m_trigtype{0};

    SG::ReadHandleKey<RpcSectorLogicContainer> m_sectorLogicContainerKey{
        this, "RPCSec", "RPC_SECTORLOGIC", "RPC sector logic"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo",
                                                   "EventInfo", "event info"};

    int m_rpc_eventstotal{0};
    int m_rpc_event_inarea{0};

    bool m_lumiblockhist{false};
    bool m_isMC{false};

    // Declare Histograms
    TH1* m_rpclv1_Hits_per_TriggerSector{nullptr};
    TH1* m_rpclv1_TriggerHitsperEvent{nullptr};
    TH2* m_rpclv1_TriggerHitsperEventperTriggerSector{nullptr};
    TH1* m_rpclv1_Hits_per_TriggerSector_LB{nullptr};
    TH2* m_rpclv1_TriggerHitsperEventperTriggerSector_LB{nullptr};
    TH1* m_rpclv1_triggerBCid_inout_LB{nullptr};

    TH2* m_rpclv1_TriggerSector_vs_Pad_triggerBCid_inout{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad_Pt1{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad_Pt2{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad_Pt3{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad_Pt4{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad_Pt5{nullptr};
    TH2* m_rpclv1_TriggerSector_vs_Pad_Pt6{nullptr};
    TH2* m_rpclv1_rowinBCid_vs_TriggerSector{nullptr};
    TH1* m_rpclv1_triggerBCid_inout{nullptr};

    TH2* m_rpclv1_ptid_vs_Tower{nullptr};
    TH2* m_rpclv1_triggerBCid_inout_vs_TriggerSector{nullptr};

    TH2* m_rpclv1_triggerBCid_inout_vs_Tower{nullptr};

    int m_nTriggerHits{0};
    int m_nTriggerHits_out{0};
    int m_nTriggerHitsperSector{0};
    int m_Diff_triggerBCid{0};
    int m_in_triggerBCid{0};
    int m_out_triggerBCid{0};
    int m_in_rowinBCid{0};
    int m_out_rowinBCid{0};
    int m_Tower_out{0};
    int m_Tower_in{0};
    int m_in_sectorid{0};
};

#endif
