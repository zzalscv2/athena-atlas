/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : RpcLv1RawDataEfficiency
// Author:  U. Schnoor (ulrike.schnoor@cern.ch) - P. Anger (panger@cern.ch)
// September 2012
//
// DESCRIPTION:
// Subject: RPCLV1-->Efficiency Offline Muon Data Quality
// RPCLv1 Sector Hits vs LB
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RpcLv1RawDataEfficiency_H
#define RpcLv1RawDataEfficiency_H

#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/RpcSectorLogicContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

// STL includes

#include <map>
#include <sstream>
#include <vector>

// ROOT includes
#include <TH2I.h>
#include <inttypes.h>

//================================================================================================================================
class OfflineMuon {
   public:
    OfflineMuon() : m_pt(0.0), m_eta(0.0), m_phi(0.0), m_q(0.0) {
        m_pt_default_if_max = 50.1;
        m_pt_max = 50.;
    }
    ~OfflineMuon() = default;

    // Getter
    float Pt() const { return m_pt; }
    float TruncatedPt() const {
        return (m_pt > m_pt_max ? m_pt_default_if_max * m_q : m_pt);
    }
    float Eta() const { return m_eta; }
    float Phi() const { return m_phi; }
    float Q() const { return m_q; }
    int QIndex() const { return (m_q < 0 ? 1 : 0); }
    float PhiSeg() const {
        return (m_phi < -CLHEP::pi / 12. ? m_phi + 2 * CLHEP::pi : m_phi);
    }

    unsigned int ChargeIndex() const { return (m_q >= 0 ? 0 : 1); }

    // Setter
    void SetPtEtaPhiQ(float pt, float eta, float phi, float q) {
        SetPt(pt);
        SetEta(eta);
        SetPhi(phi);
        SetQ(q);
    }
    void SetPt(float pt) { m_pt = pt; }
    void SetEta(float eta) { m_eta = eta; }
    void SetPhi(float phi) { m_phi = phi; }
    void SetQ(float q) { m_q = q; }

    // Calculator
    float DeltaR(const OfflineMuon& mu2) const {
        return std::hypot(m_eta - mu2.Eta(),
                          xAOD::P4Helpers::deltaPhi(m_phi, -mu2.Phi()));
    }

   private:
    float m_pt{0};  // allways in GeV
    float m_eta{0};
    float m_phi{0};
    float m_q{0};

    float m_pt_default_if_max{50.1};
    float m_pt_max{50.};
};

//================================================================================================================================
class CoincidenceData {
   public:
    CoincidenceData() = default;
    ~CoincidenceData() = default;
    // Getter
    bool IsLow() const { return ((m_lowpt == 1) ? true : false); }
    int LowHighIndex() const {
        return ((m_lowpt == 1) ? 0 : 1);
    }  // because only valid stuff is inside this works
    int Threshold() const {
        return m_threshold;
    }  // each coin matrix has three thresholds
    float Phi() const { return m_phi; }
    float Eta() const { return m_eta; }

    // Calculator
    float DeltaR(const OfflineMuon& offmu) const {
        return std::hypot(m_eta - offmu.Eta(),
                          xAOD::P4Helpers::deltaPhi(m_phi, offmu.Phi()));
    }

    // Setter
    void SetThresholdLowHigh(int threshold, int low_pt, int high_pt) {
        m_threshold = threshold;
        m_lowpt = low_pt;
        m_highpt = high_pt;
    }
    void SetEtaPhi(float eta, float phi) {
        m_eta = eta;
        m_phi = phi;
    }

   private:
    int m_lowpt{0};
    int m_highpt{0};
    int m_threshold{0};
    float m_phi{0.f};
    float m_eta{0.f};
};

// for sorting Trigger according to dR to offline muons, shortest dR first
struct CompareDR {
    CompareDR(const OfflineMuon& offmu) : m_offmu(offmu) {}
    const OfflineMuon& m_offmu;
    bool operator()(const CoincidenceData* coin1,
                    const CoincidenceData* coin2) const {
        return (coin1->DeltaR(m_offmu) < coin2->DeltaR(m_offmu));
    }
};

//================================================================================================================================
class RpcLv1RawDataEfficiency : public ManagedMonitorToolBase {
   public:
    RpcLv1RawDataEfficiency(const std::string& type, const std::string& name,
                            const IInterface* parent);
    virtual ~RpcLv1RawDataEfficiency() = default;

    StatusCode initialize();

    StatusCode readRpcCoinDataContainer();

    virtual StatusCode bookHistogramsRecurrent();
    virtual StatusCode fillHistograms();

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};

    // Trigger type stuff
    StatusCode StoreTriggerType();
    int GetTriggerType() { return m_trigtype; }
    // helper function for the different types of histograms
    std::stringstream m_ss;

    // for Sector Hits histograms

    int m_trigtype{0};
    int m_event{0};
    int m_lumiblock{0};
    int m_BCID{0};

    bool m_isMC{false};

    SG::ReadHandleKey<Muon::RpcCoinDataContainer> m_rpcCoinKey{
        this, "RpcCoinKey", "RPC_triggerHits", "RPC coincidences"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo",
                                                   "EventInfo", "EventInfo"};
    SG::ReadHandleKey<RpcSectorLogicContainer> m_sectorLogicContainerKey{
        this, "RPCSec", "RPC_SECTORLOGIC", "RPC sector logic"};

    // muon informations for offline muons and trigger hits
    std::vector<OfflineMuon> m_OfflineMuons;
    std::vector<CoincidenceData*> m_CoincidenceData;

    // Declare Histograms
    std::array<TH2I*, 6> m_rpclv1_sectorhits_A{};
    std::array<TH2I*, 6> m_rpclv1_sectorhits_C{};
    std::array<TH2I*, 6> m_rpclv1_sectorhits_all{};
};

#endif
