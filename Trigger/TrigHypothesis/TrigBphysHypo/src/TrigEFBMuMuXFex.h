/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// vim: tabstop=2:shiftwidth=2:expandtab
/**************************************************************************
**
**   File: Trigger/TrigHypothesis/TrigBphysHypo/TrigEFBMuMuXFex.h
**
**   Description: EF hypothesis algorithms for 
**                B^0_{d,s},\Lambda_b \to X \mu^+ \mu^-
**               
**
**   Author: J.Kirk
**   Author: Semen Turchikhin <Semen.Turchikhin@cern.ch>
**
**   Created:   12.09.2007
**   Modified:  08.04.2012
**   Modified:  26.09.2012 : "B_c -> D_s(*) (->Phi(->K+K-)) mu+mu-" added (Leonid Gladilin <gladilin@mail.cern.ch>)
**
***************************************************************************/
            
#ifndef TRIG_TrigEFBMuMuXFex_H 
#define TRIG_TrigEFBMuMuXFex_H

// standard stuff
#include <string>
#include <map>
#include <cmath> 
// general athena stuff
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
// trigger includes
#include "TrigInterfaces/ComboAlgo.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrigParticle/TrigEFBphys.h"
#include "TrigParticle/TrigEFBphysContainer.h"
//#include "TrigMuonEvent/TrigMuonEF.h"
//Ntuples
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"

//#include "TrigTimeAlgs/TrigTimerSvc.h"
//#include "TrigTimeAlgs/TrigTimer.h"

#include "TrigBphysHypo/Constants.h"
#include "BtrigUtils.h"
#include "TrigBphysHelperUtilsTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTrigBphys/TrigBphys.h"
#include "xAODTrigBphys/TrigBphysContainer.h"
#include "xAODTrigBphys/TrigBphysAuxContainer.h"

class StoreGateSvc;
class TriggerElement;
class CombinedMuonFeature;

class TrigBphysHelperUtilsTool;

class TrigEFBMuMuXFex: public HLT::ComboAlgo  {
  
  public:
    TrigEFBMuMuXFex(const std::string & name, ISvcLocator* pSvcLocator);
    ~TrigEFBMuMuXFex();
    HLT::ErrorCode hltInitialize();
    HLT::ErrorCode hltFinalize();     
    HLT::ErrorCode acceptInputs(HLT::TEConstVec& inputTE, bool& pass );
    HLT::ErrorCode hltExecute(HLT::TEConstVec& inputTE, HLT::TriggerElement* outputTE);
    
  private:
    
    ToolHandle < Trk::IVertexFitter  >       m_fitterSvc;
    
    ToolHandle <TrigBphysHelperUtilsTool> m_bphysHelperTool;
    
    // Cuts and properties
    bool m_oppositeCharge;
    float m_lowerMuMuMassCut;
    float m_upperMuMuMassCut;
//     float m_lowerMuVtxMassCut;
//     float m_upperMuVtxMassCut;
    float m_muVtxChi2Cut;
    
    // Maximum number of track combinations to try -- protection against TimeOuts
    int   m_maxNcombinations;
    
    bool m_doB_KMuMuDecay;
    float m_lowerKMuMuMassCut;
    float m_upperKMuMuMassCut;
//     float m_lowerBVtxMassCut;
//     float m_upperBVtxMassCut;
    bool m_doB_KMuMuVertexing;
    float m_bVtxChi2Cut;
    int m_maxBpToStore;
    
    bool m_doBd_KstarMuMuDecay;
    float m_lowerKstar_KaonMassCut;
    float m_upperKstar_KaonMassCut;
    float m_lowerBd_KstarMuMuMassCut;
    float m_upperBd_KstarMuMuMassCut;
//     float m_lowerKstarVtxMassCut;
//     float m_upperKstarVtxMassCut;
//     float m_lowerBdVtxMassCut;
//     float m_upperBdVtxMassCut;
    bool m_doKstar_KPiVertexing;
    bool m_doBd_KstarMuMuVertexing;
    float m_kStarVtxChi2Cut;
    float m_bDVtxChi2Cut;
    int m_maxBdToStore;
    
    bool m_doBs_Phi1020MuMuDecay;
    float m_lowerPhi1020_KaonMassCut;
    float m_upperPhi1020_KaonMassCut;
    float m_lowerBs_Phi1020MuMuMassCut;
    float m_upperBs_Phi1020MuMuMassCut;
//     float m_lowerPhi1020VtxMassCut;
//     float m_upperPhi1020VtxMassCut;
//     float m_lowerBsVtxMassCut;
//     float m_upperBsVtxMassCut;
    bool m_doPhi1020_KKVertexing;
    bool m_doBs_Phi1020MuMuVertexing;
    float m_phi1020VtxChi2Cut;
    float m_bSVtxChi2Cut;
    int m_maxBsToStore;
    
    bool m_doLb_LambdaMuMuDecay;
    float m_lowerLambda_PrPiMassCut;
    float m_upperLambda_PrPiMassCut;
    float m_lowerLb_LambdaMuMuMassCut;
    float m_upperLb_LambdaMuMuMassCut;
//     float m_lowerLambdaVtxMassCut;
//     float m_upperLambdaVtxMassCut;
//     float m_lowerLbVtxMassCut;
//     float m_upperLbVtxMassCut;
    bool m_doLambda_PPiVertexing;
    bool m_doLb_LambdaMuMuVertexing;
    float m_lambdaVtxChi2Cut;
    float m_lBVtxChi2Cut;
    int m_maxLbToStore;
    
    bool m_doBc_DsMuMuDecay;
    float m_lowerPhiDs_MassCut;
    float m_upperPhiDs_MassCut;
    float m_lowerDs_MassCut;
    float m_upperDs_MassCut;
    float m_lowerBc_DsMuMuMassCut;
    float m_upperBc_DsMuMuMassCut;
    bool m_doDs_Vertexing;
    bool m_doBc_DsMuMuVertexing;
    float m_DsVtxChi2Cut;
    float m_bCVtxChi2Cut;
    int m_maxBcToStore;
    
    // FTK Flag
    bool m_FTK;

    // Monitoring variables and containers
    //   General
    std::vector<int>   mon_Errors;
    std::vector<int>   mon_Acceptance;
    int                mon_nTriedCombinations;
    //   Timing
    float              mon_TotalRunTime;
    float              mon_VertexingTime;
    //   RoIs
    std::vector<float> mon_RoI_RoI1Eta;
    std::vector<float> mon_RoI_RoI2Eta;
    std::vector<float> mon_RoI_RoI1Phi;
    std::vector<float> mon_RoI_RoI2Phi;
    std::vector<float> mon_RoI_dEtaRoI;
    std::vector<float> mon_RoI_dPhiRoI;
    //   DiMuon
    int                mon_DiMu_n;
    std::vector<float> mon_DiMu_Pt_Mu1;
    std::vector<float> mon_DiMu_Pt_Mu2;
    std::vector<float> mon_DiMu_Eta_Mu1;
    std::vector<float> mon_DiMu_Eta_Mu2;
    std::vector<float> mon_DiMu_Phi_Mu1;
    std::vector<float> mon_DiMu_Phi_Mu2;
    std::vector<float> mon_DiMu_dEtaMuMu;
    std::vector<float> mon_DiMu_dPhiMuMu;
    std::vector<float> mon_DiMu_pTsumMuMu;
    std::vector<float> mon_DiMu_InvMassMuMu;
    std::vector<float> mon_DiMu_VtxMassMuMu;
    std::vector<float> mon_DiMu_Chi2MuMu;
    //   Tracks
    int                mon_Tracks_n;
    std::vector<float> mon_Tracks_Eta;
    std::vector<float> mon_Tracks_Pt;
    std::vector<float> mon_Tracks_Phi;
    //   B+
    int                mon_BMuMuK_n;
    std::vector<float> mon_BMuMuK_Pt_K;  
    std::vector<float> mon_BMuMuK_Eta_K;
    std::vector<float> mon_BMuMuK_Phi_K;
    std::vector<float> mon_BMuMuK_InvMass_B;
    std::vector<float> mon_BMuMuK_VtxMass_B;
    std::vector<float> mon_BMuMuK_Chi2_B;
    //   Bd
    int                mon_BdMuMuKs_n;
    std::vector<float> mon_BdMuMuKs_Pt_K;
    std::vector<float> mon_BdMuMuKs_Eta_K;
    std::vector<float> mon_BdMuMuKs_Phi_K;
    std::vector<float> mon_BdMuMuKs_Pt_Pi;
    std::vector<float> mon_BdMuMuKs_Eta_Pi;
    std::vector<float> mon_BdMuMuKs_Phi_Pi;
    std::vector<float> mon_BdMuMuKs_InvMass_Kstar;
    std::vector<float> mon_BdMuMuKs_VtxMass_Kstar;
    std::vector<float> mon_BdMuMuKs_Chi2_Kstar;
    std::vector<float> mon_BdMuMuKs_InvMass_Bd;
    std::vector<float> mon_BdMuMuKs_VtxMass_Bd;
    std::vector<float> mon_BdMuMuKs_Chi2_Bd;
    //   Bs
    int                mon_BsMuMuPhi_n;
    std::vector<float> mon_BsMuMuPhi_Pt_K1;
    std::vector<float> mon_BsMuMuPhi_Eta_K1;
    std::vector<float> mon_BsMuMuPhi_Phi_K1;
    std::vector<float> mon_BsMuMuPhi_Pt_K2;
    std::vector<float> mon_BsMuMuPhi_Eta_K2;
    std::vector<float> mon_BsMuMuPhi_Phi_K2;
    std::vector<float> mon_BsMuMuPhi_InvMass_Phi1020;
    std::vector<float> mon_BsMuMuPhi_VtxMass_Phi1020;
    std::vector<float> mon_BsMuMuPhi_Chi2_Phi1020;
    std::vector<float> mon_BsMuMuPhi_InvMass_Bs;
    std::vector<float> mon_BsMuMuPhi_VtxMass_Bs;
    std::vector<float> mon_BsMuMuPhi_Chi2_Bs;
    //   Lambda_b
    int                mon_LbMuMuLambda_n;
    std::vector<float> mon_LbMuMuLambda_Pt_P;
    std::vector<float> mon_LbMuMuLambda_Eta_P;
    std::vector<float> mon_LbMuMuLambda_Phi_P;
    std::vector<float> mon_LbMuMuLambda_Pt_Pi;
    std::vector<float> mon_LbMuMuLambda_Eta_Pi;
    std::vector<float> mon_LbMuMuLambda_Phi_Pi;
    std::vector<float> mon_LbMuMuLambda_InvMass_Lambda;
    std::vector<float> mon_LbMuMuLambda_VtxMass_Lambda;
    std::vector<float> mon_LbMuMuLambda_Chi2_Lambda;
    std::vector<float> mon_LbMuMuLambda_InvMass_Lb;
    std::vector<float> mon_LbMuMuLambda_VtxMass_Lb;
    std::vector<float> mon_LbMuMuLambda_Chi2_Lb;
    //   Bc
    int                mon_BcMuMuDs_n;
    std::vector<float> mon_BcMuMuDs_Pt_K1;
    std::vector<float> mon_BcMuMuDs_Eta_K1;
    std::vector<float> mon_BcMuMuDs_Phi_K1;
    std::vector<float> mon_BcMuMuDs_Pt_K2;
    std::vector<float> mon_BcMuMuDs_Eta_K2;
    std::vector<float> mon_BcMuMuDs_Phi_K2;
    std::vector<float> mon_BcMuMuDs_Pt_pi;
    std::vector<float> mon_BcMuMuDs_Eta_pi;
    std::vector<float> mon_BcMuMuDs_Phi_pi;
    std::vector<float> mon_BcMuMuDs_InvMass_PhiDs;
    std::vector<float> mon_BcMuMuDs_InvMass_Ds;
    std::vector<float> mon_BcMuMuDs_VtxMass_Ds;
    std::vector<float> mon_BcMuMuDs_Chi2_Ds;
    std::vector<float> mon_BcMuMuDs_InvMass_Bc;
    std::vector<float> mon_BcMuMuDs_VtxMass_Bc;
    std::vector<float> mon_BcMuMuDs_Chi2_Bc;


    // to set Accept-All mode: should be done with force-accept when possible
    bool m_acceptAll;


    // TrigEFBphysContainer* m_trigBphysColl_b;
    // TrigEFBphysContainer* m_trigBphysColl_X;

    xAOD::TrigBphysContainer * mTrigBphysColl_b;
    xAOD::TrigBphysContainer * mTrigBphysColl_X;
    
    // Timers
    TrigTimer* m_TotTimer;
    TrigTimer* m_VtxFitTimer;
    
    Trk::TrkVKalVrtFitter* m_VKVFitter;       
    
    //Counters
    int m_lastEvent;
    int m_lastEventPassed;
    unsigned int m_countTotalEvents;
    unsigned int m_countTotalRoI;
    unsigned int m_countPassedEvents;
    unsigned int m_countPassedRoIs;
    
    int m_lastEventPassedBplus;
    int m_lastEventPassedBd;
    int m_lastEventPassedBs;
    int m_lastEventPassedLb;
    int m_lastEventPassedBc;
    unsigned int m_countPassedEventsBplus;
    unsigned int m_countPassedEventsBs;
    unsigned int m_countPassedEventsBd;
    unsigned int m_countPassedEventsLb;
    unsigned int m_countPassedEventsBc;
    
    unsigned int m_countPassedMuMuID;
    unsigned int m_countPassedMuMuOS;
    unsigned int m_countPassedMuMuMass;
    unsigned int m_countPassedMuMuVtx;
    unsigned int m_countPassedMuMuVtxChi2;
    
    unsigned int m_countPassedBplusMass;
    unsigned int m_countPassedBplusVtx;
    unsigned int m_countPassedBplusVtxChi2;
    int m_countBpToStore;
    
    unsigned int m_countPassedKstarMass;
    unsigned int m_countPassedBdMass;
    unsigned int m_countPassedKstarVtx;
    unsigned int m_countPassedKstarVtxChi2;
    unsigned int m_countPassedBdVtx;
    unsigned int m_countPassedBdVtxChi2;
    int m_countBdToStore;
    
    unsigned int m_countPassedPhi1020Mass;
    unsigned int m_countPassedBsMass;
    unsigned int m_countPassedPhi1020Vtx;
    unsigned int m_countPassedPhi1020VtxChi2;
    unsigned int m_countPassedBsVtx;
    unsigned int m_countPassedBsVtxChi2;
    int m_countBsToStore;
    
    unsigned int m_countPassedLambdaMass;
    unsigned int m_countPassedLbMass;
    unsigned int m_countPassedLambdaVtx;
    unsigned int m_countPassedLambdaVtxChi2;
    unsigned int m_countPassedLbVtx;
    unsigned int m_countPassedLbVtxChi2;
    int m_countLbToStore;
    
    unsigned int m_countPassedPhiDsMass;
    unsigned int m_countPassedDsMass;
    unsigned int m_countPassedBcMass;
    unsigned int m_countPassedDsVtx;
    unsigned int m_countPassedDsVtxChi2;
    unsigned int m_countPassedBcVtx;
    unsigned int m_countPassedBcVtxChi2;
    int m_countBcToStore;

    
    void addUnique(std::vector<const Trk::Track*>&, const Trk::Track*);
    void addUnique(std::vector<const xAOD::TrackParticle*>&, const xAOD::TrackParticle*);
    void addUnique(std::vector<ElementLink<xAOD::TrackParticleContainer> >&, const ElementLink<xAOD::TrackParticleContainer>&);
//   double invariantMass(const TrigMuonEF* , const TrigMuonEF* );
//   double invariantMass(const Trk::Track* , const Trk::Track* );
    bool isUnique(const  xAOD::TrackParticle* id1, const  xAOD::TrackParticle* id2) const;

//     double XMass(const Trk::Track* particle1, const Trk::Track* particle2, int decay);
//     double KMuMuMass(const Trk::Track* mu1, const Trk::Track* mu2, const Trk::Track* kaon);
//     double XMuMuMass(const Trk::Track* mu1, const Trk::Track* mu2, const Trk::Track* particle1, const Trk::Track* particle2, int decay);
//     double X3Mass(const Trk::Track* particle1, const Trk::Track* particle2, const Trk::Track* particle3 );
//     double X3MuMuMass(const Trk::Track* mu1, const Trk::Track* mu2, const Trk::Track* particle1, const Trk::Track* particle2, const Trk::Track* particle3 );
//     TrigEFBphys* checkBMuMu2X(const Trk::Track* mu1, const Trk::Track* mu2, const Trk::Track* track1, const Trk::Track* track2, int decay, TrigEFBphys* & trigPartX);
//     TrigEFBphys* checkBcMuMuDs(const Trk::Track* mu1, const Trk::Track* mu2, const Trk::Track* track1, const Trk::Track* track2, double xPhiMass, const Trk::Track* track3, TrigEFBphys* & trigPartX);
//     TrigEFBphys* checkBplusMuMuKplus(const Trk::Track* mu1, const Trk::Track* mu2, const Trk::Track* track1);

    
    double XMass(const xAOD::TrackParticle* particle1, const xAOD::TrackParticle* particle2, int decay); /// checking the mass
    double X3Mass(const xAOD::TrackParticle* particle1, const xAOD::TrackParticle* particle2, const xAOD::TrackParticle* particle3 );
    
    double KMuMuMass( const xAOD::TrackParticle* mu1, const xAOD::TrackParticle* mu2, const xAOD::TrackParticle* kaon);
    double XMuMuMass( const xAOD::TrackParticle* mu1, const xAOD::TrackParticle* mu2, const xAOD::TrackParticle* particle1,
                     const xAOD::TrackParticle* particle2, int decay);
    double X3MuMuMass(const xAOD::TrackParticle* mu1, const xAOD::TrackParticle* mu2, const xAOD::TrackParticle* particle1,
                      const xAOD::TrackParticle* particle2, const xAOD::TrackParticle* particle3 );
    
    xAOD::TrigBphys* checkBplusMuMuKplus(const ElementLink<xAOD::TrackParticleContainer> & eltrack1,
                                         const ElementLink<xAOD::TrackParticleContainer> & elmu1,
                                         const ElementLink<xAOD::TrackParticleContainer> & elmu2);
    
    xAOD::TrigBphys* checkBMuMu2X(const ElementLink<xAOD::TrackParticleContainer> & eltrack1,
                                  const ElementLink<xAOD::TrackParticleContainer> & eltrack2,
                                  const ElementLink<xAOD::TrackParticleContainer> & elmu1,
                                  const ElementLink<xAOD::TrackParticleContainer> & elmu2,
                                  int decay, xAOD::TrigBphys* & trigPartX);
    
    xAOD::TrigBphys* checkBcMuMuDs(const ElementLink<xAOD::TrackParticleContainer> & eltrack1,
                                   const ElementLink<xAOD::TrackParticleContainer> & eltrack2,
                                   const ElementLink<xAOD::TrackParticleContainer> & eltrack3,
                                   const ElementLink<xAOD::TrackParticleContainer> & elmu1,
                                   const ElementLink<xAOD::TrackParticleContainer> & elmu2,
                                   double xPhiMass,
                                   xAOD::TrigBphys* & trigPartX);

    
};

// Define the bins for error-monitoring histogram
#define ERROR_No_EventInfo           0
#define ERROR_Not_2_InputTEs         1
#define ERROR_No_RoIs                2
#define ERROR_No_MuonEFInfoContainer 3
#define ERROR_No_MuonCandidate       4

#define ERROR_SameMuon               5
#define ERROR_DiMuVtxFit_Fails       6
#define ERROR_DiMuVtxMass_Fails      7

#define ERROR_No_TrackColl           8
#define ERROR_No_MuonTrackMatch      17

#define ERROR_BplusVtxFit_Fails      9
#define ERROR_BplusVtxMass_Fails     10
#define ERROR_WrongDecayID           11
#define ERROR_XVtxFit_Fails          12
#define ERROR_XVtxMass_Fails         13
#define ERROR_XMuMuVtxFit_Fails      14
#define ERROR_XMuMuVtxMass_Fails     15

#define ERROR_BphysCollStore_Fails   16

#define ERROR_TooManyComb_Acc        18
#define ERROR_TooManyComb_Rej        19

#define ERROR_MaxNumBpReached        20
#define ERROR_MaxNumBdReached        21
#define ERROR_MaxNumBsReached        22
#define ERROR_MaxNumLbReached        23
#define ERROR_MaxNumBcReached        24

// // Define the bins for acceptance-monitoring histogram
// #define ACCEPT_Input                 0
// #define ACCEPT_AcceptAll             1
// #define ACCEPT_Got_RoIs              2
// #define ACCEPT_Got_Muons             3
// #define ACCEPT_Got_TrackColl         4
// #define ACCEPT_First_TrackColl       5
// #define ACCEPT_Both_TrackColls       6
// #define ACCEPT_Full_IDTracks         7
// #define ACCEPT_Pass_OppChargeC       8
// #define ACCEPT_MuonTracks_Added      8
// #define ACCEPT_Muon_Vertexing        9
// #define ACCEPT_CalcInvMass          10 
// #define ACCEPT_MuonVtx_Part         11
// #define ACCEPT_MuMu_Mass            12
// #define ACCEPT_MotherVtxCreated     13
// #define ACCEPT_BphysCollParticle    14

#endif
