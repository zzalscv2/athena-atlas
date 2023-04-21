/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////
// Package : MdtRawDataMonitoring
// Author:   N. Benekos(Illinois) - G. Dedes(MPI) - Orin Harris (University of Washington)
// Sept. 2007
//
// DESCRIPTION:
// Subject: MDT-->Offline Muon Data Quality
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef MdtRawDataValAlg_H
#define MdtRawDataValAlg_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

#include "AthenaMonitoring/DQAtlasReadyFilterTool.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MDTChamber.h"
#include "MDTMonGroupStruct.h"
#include "MDTNoisyTubes.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonChamberIDSelector.h"
#include "MuonDQAUtils/MuonDQAHistMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkSegment/SegmentCollection.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"

class MuonDQAHistList;

namespace Muon {
    class MdtPrepData;
}

// stl includes
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// root includes
class TH1;
class TH2;
class TString;
class TH1F_LW;
class TH2F_LW;
class TColor;

enum { L1_UNKNOWN, L1_BARREL, L1_ENDCAP };

class TubeTraversedBySegment {
public:
    TubeTraversedBySegment(const std::string& hn, int tb, bool ih, IdentifierHash idh) {
        hardware_name = hn;
        tubeBin = tb;
        isHit = ih;
        idHash = idh;
    }
    std::string hardware_name;
    int tubeBin;
    bool isHit;
    IdentifierHash idHash;
};

// Be careful here -- changes to this can break whether insertions to set are unique
// JG remove expensive hardware_name comparison w/ idHash comparison
// Hope I was careful ;)
struct TubeTraversedBySegment_cmp {
    bool operator()(const TubeTraversedBySegment& A, const TubeTraversedBySegment& B) const {
        if (A.idHash > B.idHash) {
            return true;
        } else {
            if ((A.tubeBin > B.tubeBin) && (A.idHash == B.idHash)) {
                return true;
            } else {
                if ((A.isHit > B.isHit) && (A.tubeBin == B.tubeBin) && (A.idHash == B.idHash)) { return true; }
            }
        }
        return false;
    }
};

template <class ConcreteAlgorithm> class AlgFactory;
class MdtRawDataValAlg : public ManagedMonitorToolBase {
public:
    MdtRawDataValAlg(const std::string& type, const std::string& name, const IInterface* parent);

    virtual ~MdtRawDataValAlg();
    StatusCode initialize();
    virtual StatusCode bookHistogramsRecurrent(/* bool newLumiBlock, bool newRun */);
    virtual StatusCode fillHistograms();
    virtual StatusCode procHistograms();

private:
    std::string m_title;

    bool m_atlas_ready{false};
    bool isATLASReady() const { return m_atlas_ready; }
    void setIsATLASReady();

    std::unique_ptr<MDTMonGroupStruct> m_mg;
    std::unique_ptr<MDTNoisyTubes> m_masked_tubes;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool;

    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

    virtual StatusCode bookMDTHistograms(MDTChamber* chamber, Identifier digcoll_id);  // book chamber by chamber histos
    virtual StatusCode fillMDTHistograms(const Muon::MdtPrepData*);                    // fill chamber by chamber histos
    virtual StatusCode bookMDTSummaryHistograms(bool newLumiBlock, bool newRun);       // Those over barrel/encap layer etc.
    virtual StatusCode fillMDTSummaryHistograms(const Muon::MdtPrepData*, std::set<std::string>, bool& isNoiseBurstCandidate);
    virtual StatusCode bookMDTOverviewHistograms(bool newLumiBlock, bool newRun);
    virtual StatusCode fillMDTOverviewHistograms(const Muon::MdtPrepData*, bool& isNoiseBurstCandidate);
    StatusCode handleEvent_effCalc(const Trk::SegmentCollection* segms);  //, const Muon::MdtPrepDataContainer* mdt_container );

    static bool AinB(int A, std::vector<int>& B);
    virtual StatusCode binMdtGlobal(TH2*, char ecap);
    virtual StatusCode binMdtRegional(TH2*, std::string_view xAxis);
    virtual StatusCode binMdtGlobal_byLayer(TH2*, TH2*, TH2*);
    virtual StatusCode binMdtOccVsLB(TH2*& h, int region, int layer);
    virtual StatusCode binMdtOccVsLB_Crate(TH2*& h, int region, int crate);
    static void TubeID_to_ID_L_ML(int& tubeID, std::string_view hardware_name, int& tube, int& layer, int& ML, int max);
    static void ChamberTubeNumberCorrection(int& tubeNum, std::string_view hardware_name, int tubePos, int numLayers);
    static void CorrectTubeMax(const std::string& hardware_name, int& numTubes);
    static void CorrectLayerMax(const std::string& hardware_name, int& numLayers);
    virtual StatusCode bookMDTHisto_overview(TH1*&, TString, TString, TString, int, float, float, MonGroup&);
    virtual StatusCode bookMDTHisto_chambers(TH1F_LW*&, TString, TString, TString, int, float, float, MonGroup&);
    virtual StatusCode bookMDTHisto_overview_2D(TH2*& h, TString, TString, TString, int, float, float, int, float, float, MonGroup&);
    virtual StatusCode bookMDTHisto_chambers_2D(TH2F_LW*& h, TString, TString, TString, int, float, float, int, float, float, MonGroup&);
    virtual StatusCode bookMDTHisto_OccVsLB(TH2*& h, TString, TString, TString, int, float, float, int, float, float, MonGroup&);
    virtual StatusCode fillMDTMaskedTubes(IdentifierHash, const std::string&, TH1F_LW*& h);
    static void putBox(TH2* h, float x1, float y1, float x2, float y2);
    static void putLine(TH2* h, float x1, float y1, float x2, float y2, Color_t c = kBlack);
    static int get_bin_for_LB_hist(int region, int layer, int phi, int eta, bool isBIM);
    int get_bin_for_LB_crate_hist(int region, int layer, int phi, int eta, std::string_view chamber);
    // private function to initialize the selection of a certain region
    void mdtchamberId();
    // private function to find mdt mezz cards
    int mezzmdt(Identifier);
    int GetTubeMax(const Identifier& digcoll_id, std::string_view hardware_name);
    StatusCode StoreTriggerType();
    void StoreTriggerType(int type);
    int GetTriggerType() { return m_trigtype; }
    bool HasTrigBARREL() { return m_trig_BARREL; }
    bool HasTrigENDCAP() { return m_trig_ENDCAP; }
    StatusCode fillLumiBlock();
    StatusCode GetTimingInfo();
    StatusCode GetEventNum();
    void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);

    ToolHandleArray<IDQFilterTool> m_DQFilterTools;
    int m_lumiblock{0};
    int m_eventNum{0};
    int m_firstEvent{0};
    uint32_t m_time{0U};
    uint32_t m_firstTime{0U};
    int m_numberOfEvents{0};

    SG::ReadHandleKey<Trk::SegmentCollection> m_segm_type{this, "Eff_segm_type", "TrackMuonSegments", "muon segments"};

    static int returnInt(const std::string& s) {
        std::stringstream ss(s);
        int n;
        ss >> n;
        return n;
    }

    std::vector<Identifier> m_chambersId;
    std::vector<IdentifierHash> m_chambersIdHash;
    std::set<std::string> m_hardware_name_list;
    std::set<int> m_hashId_list;
    std::map<std::string, float> m_hitsperchamber_map;

    std::map<std::string, float> m_tubesperchamber_map;

    bool m_doMdtESD;

    // to book or not bookMDTTDCplots -->   /Chambers/tmp/ directory
    bool m_doChamberHists;
    bool m_isOnline;
    bool m_maskNoisyTubes;
    bool m_chamber_2D;  // Set this to true/false in the Job Options in order to see/not see chamber by chamber 2d adc vs tdc plots.

    std::string m_chamberName;
    std::string m_StationSize;
    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_key_mdt{this, "MdtPrepDataContainer", "MDT_DriftCircles", "MDT PRDs"};
    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_key_rpc{this, "RpcPrepDataContainer", "RPC_Measurements", "RPC PRDs"};
    SG::ReadHandleKey<xAOD::MuonRoIContainer> m_l1RoiKey{this, "L1RoiKey", "LVL1MuonRoIs", "L1 muon ROIs"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons", "muons"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo", "EventInfo", "event info"};
    int m_StationEta;
    int m_StationPhi;

    int m_trigtype = 0;
    bool m_trig_BARREL{false};
    bool m_trig_ENDCAP{false};
    // Define configurable adccut and TGC/RPC keys
    float m_ADCCut;
    float m_ADCCut_Bkgrd;
    float m_curTime = 0.0F;

    // From Old BS
    TH2* m_overalltdcadcLumi{};               // all chambers tdc vs adc superimposed
    TH2* m_overalltdcadcPRLumi[4]{};          // all chambers tdc vs adc superimposed
    TH1* m_overalltdccutLumi{};               // all chambers tdc superimposed with adc cut
    TH1* m_overalltdccut_segm_Lumi{};         // all chambers tdc superimposed with adc cut
    TH1* m_overalladc_segm_Lumi{};            // all chambers adc on segm
    TH1* m_overalladc_Lumi{};                 // all chambers adc
    TH1* m_overalltdccut_segm_PR_Lumi[4]{};   // all chambers tdc superimposed with adc cut per region
    TH1* m_overalltdccutPRLumi[4]{};          // all chambers tdc superimposed with adc cut per region
    TH1* m_overalladc_segm_PR_Lumi[4]{};      // all chambers adc superimposed per region
    TH1* m_overalladcPRLumi[4]{};             // all chambers adc superimposed per region
    TH1* m_overalladccutPRLumi[4]{};          // all chambers adc superimposed per region with adc noise cut
    TH1* m_overalltdccutPRLumi_RPCtrig[4]{};  // all chambers tdc superimposed with adc cut per region
    TH1* m_overalltdccutPRLumi_TGCtrig[4]{};  // all chambers tdc superimposed with adc cut per region

    TH2* m_overalltdcadcHighOcc{};            // all chambers tdc vs adc superimposed, events with > m_HighOccThreshold hits
    TH1* m_overalltdcHighOcc{};               // all chambers tdc superimposed, events with > m_HighOccThreshold hits
    TH1* m_overalltdcHighOcc_ADCCut{};        // all chambers tdc (with ADC>80) superimposed, events with > m_HighOccThreshold hits
    TH1* m_overalladc_HighOcc{};              // all chambers adc superimposed, events with > m_HighOccThreshold hits
    TH2* m_overalltdcadcPR_HighOcc[4]{};      // all chambers tdc vs adc superimposed
    TH1* m_overalltdcPR_HighOcc[4]{};         // all chambers tdc superimposed per region
    TH1* m_overalltdcPR_HighOcc_ADCCut[4]{};  // all chambers tdc superimposed with adc cut per region
    TH1* m_overalladcPR_HighOcc[4]{};         // all chambers tdc superimposed with adc cut per region

    TH2* m_overall_mdt_DRvsDT{};
    TH2* m_overall_mdt_DRvsSegD{};
    TH2* m_overallPR_mdt_DRvsDT[4]{};
    TH2* m_overallPR_mdt_DRvsSegD[4]{};
    TH2* m_MdtNHitsvsRpcNHits{};

    TH1* m_mdteventscutLumi{};      // Total number of MDT digits with a cut on ADC
    TH1* m_mdteventscutLumi_big{};  // Total number of MDT digits with a cut on ADC (for high mult. evt)
    TH1* m_mdteventsLumi{};         // Total number of MDT digits without a cut on ADC
    TH1* m_mdteventsLumi_big{};     // Total number of MDT digits without a cut on ADC (for high mult. evt)

    TH1* m_mdtglobalhitstime{};

    TH1* m_nummdtchamberswithhits{};         // Number of MDT chambers with hits
    TH1* m_nummdtchamberswithhits_ADCCut{};  // Number of MDT chambers with hits
    TH1* m_nummdtchamberswithHighOcc{};      // Number of MDT chambers with > 1% occupancy

    TH1* m_mdtchamberstat{};
    TH1* m_mdtchamberstatphislice[16]{};
    TH1* m_mdtChamberHits[4][4][16]{};
    TH2* m_mdtxydet[3]{};
    TH2* m_mdtrzdet[3]{};
    TH2* m_mdthitspermultilayerLumi[4][4]{};
    TH2* m_mdteffpermultilayer[4][4]{};
    TH2* m_mdthitsperchamber_InnerMiddleOuterLumi[2]{};
    TH2* m_mdthitsperchamber_InnerMiddleOuter_HighOcc[2]{};
    TH2* m_mdthitsperchamber_onSegm_InnerMiddleOuterLumi[2]{};
    TH2* m_mdteffperchamber_InnerMiddleOuter[4]{};
    TH2* m_mdthitsperML_byLayer[3]{};  // These are alternative Global hit coverage plots
    TH2* m_mdtoccvslb[4][3]{};
    TH2* m_mdtoccvslb_by_crate[4][4]{};
    TH2* m_mdtoccvslb_ontrack_by_crate[4][4]{};
    TH2* m_mdtoccvslb_summaryPerSector{};

    /////End from old BS

    ///////////For t0 calculations//////////
    TH1* m_mdttdccut_sector[4][4][16]{};  ////  [endcap/barrel A/C][layer][sector]

    // Chamber by Chamber Plots
    std::vector<std::unique_ptr<MDTChamber>> m_hist_hash_list{};

    std::string getChamberName(const Muon::MdtPrepData*);
    std::string getChamberName(const Identifier& id);
    StatusCode getChamber(IdentifierHash id, MDTChamber*& chamber);

    // Control for which histograms to add
    bool m_do_mdtchamberstatphislice;
    bool m_do_mdtChamberHits;
    bool m_do_mdttdccut_sector;
    bool m_do_mdttdc;
    bool m_do_mdttdccut_ML1;
    bool m_do_mdttdccut_ML2;
    bool m_do_mdtadc_onSegm_ML1;
    bool m_do_mdtadc_onSegm_ML2;
    bool m_do_mdttdccut_RPCtrig_ML1;
    bool m_do_mdttdccut_TGCtrig_ML1;
    bool m_do_mdttdccut_RPCtrig_ML2;
    bool m_do_mdttdccut_TGCtrig_ML2;
    bool m_do_mdtadc;
    bool m_do_mdttdcadc;
    bool m_do_mdtmultil;
    bool m_do_mdtlayer;
    bool m_do_mdttube;
    bool m_do_mdttube_bkgrd;
    bool m_do_mdttube_fornoise;
    bool m_do_mdttube_masked;
    bool m_do_mdtmezz;
    bool m_do_mdt_effEntries;
    bool m_do_mdt_effCounts;
    bool m_do_mdt_effPerTube;
    bool m_do_mdt_DRvsDT;
    bool m_do_mdt_DRvsDRerr;
    bool m_do_mdt_DRvsSegD;
    bool m_do_mdttubenoise;
    bool m_do_mdttdctube;

    float m_nb_hits;           // minimum number of hits in segment
    float m_road_width;        // road width for pattern recognition
    float m_chi2_cut;          // track chi2 cut;
    float m_HighOccThreshold;  // minimum number of hits to consider an event a possible noise burst
    bool m_BMGpresent{false};
    int m_BMGid{-1};
    std::map<Identifier, std::vector<Identifier> > m_DeadChannels;

    int cachedTubeMax(const Identifier& id) const;
    int cachedTubeLayerMax(const Identifier& id) const;
    const MuonGM::MuonDetectorManager* m_detMgr{nullptr};
};

#endif
