/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/IJobOptionsSvc.h"
#include "AthenaMonitoring/AthenaMonManager.h"
#include "AthenaMonitoring/ManagedMonitorToolTest.h"
#include "AnalysisUtils/AnalysisMisc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/IToolSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "EventInfo/TriggerInfo.h"
#include "TrigSteeringEvent/HLTResult.h"
#include "EventInfo/EventInfo.h"
#include <EventInfo/EventID.h>
#include "xAODEventInfo/EventInfo.h"

#include "TrigDecisionTool/FeatureContainer.h"
#include "TrigDecisionTool/Feature.h"
#include "TrigSteeringEvent/TrigOperationalInfo.h"
#include "TrigSteeringEvent/TrigOperationalInfoCollection.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "TrigSteeringEvent/TrigOperationalInfoCollection.h"

#include "TrigConfL1Data/PrescaleSet.h"

#include "TrigTauEmulation/Level1EmulationTool.h"
#include "TrigTauEmulation/HltEmulationTool.h"

#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauDefs.h"

#include "xAODTrigger/EmTauRoI.h"
#include "xAODTrigger/EmTauRoIContainer.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"

#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

#include "VxVertex/VxContainer.h"

#include "TROOT.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH2I.h"
#include "TH2F.h"
#include "TEfficiency.h"
#include "TProfile.h"

#include <vector>
#include <iostream>
#include <fstream>
//#define _USE_MATH_DEFINES
#include <math.h>
#include "TrigHLTMonitoring/IHLTMonTool.h"
#include "HLTTauMonTool.h"


using namespace std;
using namespace AnalysisUtils;

const float PI=2.0*acos(0.);
const float TWOPI=2.0*PI;


///////////////////////////////////////////////////////////////////
void HLTTauMonTool::bookHistogramsForItem(const std::string & trigItem){
    
    const int nbin_pt = 13;
    double bins_pt[nbin_pt] = {10.,20.,25.,30.,35.,40.,45.,50.,60.,70.,100.,150.,200.};
    const int nbin_leppt = 32;
    double bins_leppt[nbin_leppt] = {10.,11.,12.,13.,14.,15.,16.,17.,18.,19.,20.,21.,22.,23.,24.,25.,26.,27.,28.,29.,30.,32.,34.,36.,38.,40.,45.,50.,60.,70.,80.,100.};
    const int nbin_eta = 9;
    double bins_eta[nbin_eta] = {-2.47,-1.52,-1.37,-0.69,0.,0.69,1.37,1.52,2.47};
    const int nbin_nvtx = 6;
    double bins_nvtx[nbin_nvtx] = {0.,5.,10.,15.,20.,25.};
    const int nbin_mu = 21;
    float bins_mu[nbin_mu] = {0.,2.,4.,6.,8.,10.,12.,14.,16.,18.,20.,22.,24.,26.,28.,30.,32.,34.,36.,38.,40.};
    const int nbin_met = 14;
    double bins_met[nbin_met] = {0.,5.,10.,20.,25.,30.,35.,40.,45.,50.,60.,70.,100.,150.};
    const int nbin_dr = 18;
    double bins_dr[nbin_dr] = {0.,0.5,1.,1.5,1.8,2.,2.2,2.3,2.4,2.5,2.6,2.7,2.8,2.9,3.0,3.1,3.2,3.4};
    
    // define here all histograms
    //L1 Roi
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/L1RoI",run));
    addHistogram(new TH1F("hL1RoIEta","L1 RoI Eta ; #eta; N RoI",100,-2.6,2.6));
    addHistogram(new TH1F("hL1RoIPhi","L1 RoI Phi ; #phi; N RoI",100,-3.2,3.2));
    
    addHistogram(new TH2F("hL1EtaVsPhi","L1 RoI Eta vs Phi; #eta; #phi",100,-2.6,2.6,100,-3.2,3.2));
    addHistogram(new TH1F("hL1RoIisol","L1 RoI Isolation; RoI Isolation Bit; N RoI",10,0.5,9.5));
    addHistogram(new TH1F("hL1RoIeT","L1 RoI Tau Clust Energy; E_{T}[GeV]; N RoI",200,0.,100.));
    addHistogram(new TH1F("hL1RoITauClus","L1 RoI Tau Clust Energy; E_{T}[GeV]; N RoI",200,0.,100.));
    addHistogram(new TH1F("hL1RoITauClus2","L1 RoI Tau Clust Energy; E_{T}[GeV]; N RoI",200,0.,1000.));
    addHistogram(new TH1F("hL1RoIEMIso","L1 RoI EM Isol ; E_{T}^{EM Isol}[GeV]; N RoI",16,-2,30));
    addHistogram(new TH1F("hL1RoIHadCore","L1 RoI HAD Core ; E_{T}^{HAD}[GeV]; N RoI",16,-2,30));
    addHistogram(new TH1F("hL1RoIHadIsol","L1 RoI HAD Isol ; E_{T}^{HAD Isol}[GeV]; N RoI",16,-2,30));
    addHistogram(new TH2F("hL1RoITauClusEMIso","L1 RoI TauClus vs EMiso ; E_{T}[GeV]; E_{T}^{EM Isol}[GeV]",25,0.,100.,16,-2,30));
    addHistogram(new TH2F("hL1RoITauVsJet","L1 RoI Tau Et vs Jet Et ; Tau E_{T} [GeV]; Jet E_{T} [GeV]",200,0.,100.,200,0.,100));
    addHistogram(new TH2F("hL1RoITauVsJetMismatch","L1 RoI Tau-Jet deta-dphi if Jet Et< Tau Et ; d#eta; d#phi",50,-0.3,0.3,50,-0.3,0.3));
    addHistogram(new TH2F("hL1RoITauVsJetDEt","L1 RoI Tau-Jet dEt if Jet Et< Tau Et ; Tau E_{t}; dE_{T}",200,0.,100.,50,0.,25.));
    
    //--------------------
    //Pre-selection Tau
    //--------------------
    
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/PreselectionTau",run));
    
    addHistogram(new TH1F("hEFEt","EF Et;E_{T}[GeV];Nevents",40,0.0,100.0));
    addHistogram(new TH1F("hEFEt2","EF Et;E_{T}[GeV];Nevents",100,0.0,1000.0));
    addHistogram(new TH1F("hFTFnTrack","EF number of tracks;number of tracks;Nevents",10,0,10));
    addHistogram(new TH1F("hEta","EF TrigCaloCluster Eta; #eta ; Nevents",26,-2.6,2.6));
    addHistogram(new TH1F("hPhi","EF TrigCaloCluster Phi; #phi ; Nevents",32,-3.2,3.2));
    addHistogram(new TH1F("hdRmax","EF deltaR max; dRmax ; Nevents",52,-0.02,0.5));
    addHistogram(new TH2F("hEFEtaVsPhi","EF TrigCaloCluster Eta vs Phi; #eta ; #phi ; Nevents",
                          26,-2.6,2.6,32,-3.2,3.2));
    addHistogram(new TH2F("hEtVsEta","Et from tau Jet vs #eta; #eta^{EF}; Raw E_{T}[GeV]",
                          26,-2.6,2.6,40,0.0,100.0));
    addHistogram(new TH2F("hEtVsPhi","Et from tau Jet vs #phi; #phi^{EF}; Raw E_{T} [GeV]",
                          32,-3.2,3.2,40,0.0,100.0));
    addHistogram(new TH1F("hFTFnWideTrack","EF number of wide tracks;number of tracks;Nevents",10,0,10));
    
    //--------------------
    // EF
    //--------------------
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFTau",run, ATTRIB_MANAGED, ""));
    //Basic kinematic variables
    addHistogram(new TH1F("hEFEt","EF Et;E_{T}[GeV];Nevents",50,0.0,100.0));
    addHistogram(new TH1F("hEFEta","EF TrigCaloCluster Eta; #eta ; Nevents",26,-2.6,2.6));
    addHistogram(new TH1F("hEFPhi","EF TrigCaloCluster Phi; #phi ; Nevents",16,-3.2,3.2));
    addHistogram(new TH1F("hEFnTrack","EF number of tracks;number of tracks;Nevents",10,0,10));
    addHistogram(new TH2F("hEFEtaVsPhi","EF TrigCaloCluster Eta vs Phi; #eta ; #phi ; Nevents",26,-2.6,2.6,16,-3.2,3.2));
    addHistogram(new TH2F("hEFEtVsPhi","Et from tau Jet vs #phi; #phi^{EF}; Raw E_{T} [GeV]",16,-3.2,3.2,50,0.0,100.0));
    addHistogram(new TH2F("hEFEtVsEta","Et from tau Jet vs #eta; #eta^{EF}; Raw E_{T}[GeV]",26,-2.6,2.6,50,0.0,100.0));
    addHistogram(new TH1F("hEFEtRaw","EF Et Raw;Uncalibrated E_{T}[GeV];Nevents",50,0.,100.));
    addHistogram(new TH1F("hEFnWideTrack","EF number of wide tracks;number of tracks;Nevents",10,0,10));
    //other variables not used in BDT
    //addHistogram(new TH1F("hEFEMRadius","EF EMRadius;EM Radius;Clusters",50,-0.1,0.5));
    //addHistogram(new TH1F("hEFHADRadius","EF HADRadius;HAD Radius;Clusters",50,-0.1,0.5));
    addHistogram(new TH1F("hEFIsoFrac", "Iso Fraction at EF; isoFrac at EF; Candidates",50,-0.1,1.1));
    //addHistogram(new TH1F("hEFPSSFraction", "PSS Fraction at EF; PSS at EF; Candidates",50,-0.05,1.1));
    addHistogram(new TH1F("hEFEMFraction", "Em Fraction at EF; EM Fraction at EF; Candidates",50,-0.05,1.1));
    addHistogram(new TH1F("hScore1p", "1p BDT Score; HLT BDT Score; Candidates",50,0.,1.));
    addHistogram(new TH1F("hScoremp", "mp BDT Score; HLT BDT Score; Candidates",50,0.,1.));
    //BDT inputs for 1-prong Non-Corrected
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFTau/BDT/1p_nonCorrected",run, ATTRIB_MANAGED, ""));
    addHistogram(new TH1F("hEFinnerTrkAvgDist1PNCorr", "Inner Track Average Distance at EF 1-prong non-corrected; innertrkAvgDist at EF; Candidates",50,-0.05,0.5));
    addHistogram(new TH1F("hEFetOverPtLeadTrk1PNCorr", "Et over Lead Track Pt at EF 1-prong non-corrected; etOverPtLeadTrk at EF; Candidates",51,-0.1,25.0));
    addHistogram(new TH1F("hEFipSigLeadTrk1PNCorr", "IpSigLeadTrk at EF 1-prong non-corrected; ipSigLeadTrk at EF; Candidates",50,-20.0,20.0));
    addHistogram(new TH1F("hEFSumPtTrkFrac1PNCorr", "SumPtTrkFrac at EF 1-prong non-corrected; SumPtTrkFrac at EF; Candidates",50,-0.5,1.1));
    addHistogram(new TH1F("hEFChPiEMEOverCaloEME1PNCorr", "ChPiEMEOverCaloEME at EF 1-prong non-corrected; ChPiEMEOverCaloEME at EF; Candidates",51,-20.0,20.0));
    addHistogram(new TH1F("hEFEMPOverTrkSysP1PNCorr", "EMPOverTrkSysP at EF 1-prong non-corrected; EMPOverTrkSysP at EF; Candidates", 41,0.0,40.0));
    addHistogram(new TH1F("hEFcentFrac1PNCorr", "Centrality Fraction at EF 1-prong non-corrected; centFrac at EF; Candidates",50,-0.05,1.2));
    addHistogram(new TH1F("hEFptRatioEflowApprox1PNCorr", "ptRatioEflowApprox at EF 1-prong non-corrected; ptRatioEflowApprox at EF; Candidates",50,0.0,2.0));
    addProfile(new TProfile("hEFinnerTrkAvgDist1PNCmu", "InnerTrkAvgDist at EF vs mu 1-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFetOverPtLeadTrk1PNCmu", "EtOverPtLeadTrk at EF vs mu 1-prong non-corrected;Average interactions per bunch crossing ;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFipSigLeadTrk1PNCmu", "IpSigLeadTrk at EF vs mu 1-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFSumPtTrkFrac1PNCmu", "SumPtTrkFrac at EF vs mu 1-prong non-corrected;Average interactions per bunch crossing ;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFChPiEMEOverCaloEME1PNCmu", "ChPiEMEOverCaloEME at EF vs mu 1p non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFEMPOverTrkSysP1PNCmu", "EMPOverTrkSysP at EF vs mu 1-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFcentFrac1PNCmu", "Centrality Fraction at EF vs mu 1-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFptRatioEflowApprox1PNCmu", "ptRatioEflowApprox at EF vs mu 1p non-corrected;Average interactions per bunch crossing ;",nbin_mu-1,bins_mu));
    //BDT inputs for 3-prong Non-Corrected
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFTau/BDT/mp_nonCorrected",run, ATTRIB_MANAGED, ""));
    addHistogram(new TH1F("hEFinnerTrkAvgDistMPNCorr", "Inner Track Average Distance at EF m-prong non-corrected; innertrkAvgDist at EF; Candidates",50,-0.05,0.5));
    addHistogram(new TH1F("hEFetOverPtLeadTrkMPNCorr", "Et over Lead Track Pt at EF m-prong non-corrected; etOverPtLeadTrk at EF; Candidates",51,-0.1,25.0));
    addHistogram(new TH1F("hEFChPiEMEOverCaloEMEMPNCorr", "ChPiEMEOverCaloEME at EF m-prong non-corrected; ChPiEMEOverCaloEME at EF; Candidates",51,-20.0,20.0));
    addHistogram(new TH1F("hEFEMPOverTrkSysPMPNCorr", "EMPOverTrkSysP at EF m-prong non-corrected; EMPOverTrkSysP at EF; Candidates", 41,0.0,40.0));
    addHistogram(new TH1F("hEFcentFracMPNCorr", "Centrality Fraction at EF m-prong non-corrected; centFrac at EF; Candidates",50,-0.05,1.2));
    addHistogram(new TH1F("hEFptRatioEflowApproxMPNCorr", "ptRatioEflowApprox at EF m-prong non-corrected; ptRatioEflowApprox at EF; Candidates",50,0.0,2.0));
    addHistogram(new TH1F("hEFdRmaxMPNCorr", "Max dR of associated tracks at EF m-prong non-corrected; dRmax at EF; Candidates",50,-0.1,0.3));
    addHistogram(new TH1F("hEFtrFlightPathSigMPNCorr", "TrFlightPathSig at EF m-prong non-corrected; trFlightPathSig at EF; Candidates",50,-20.0,20.0));
    addHistogram(new TH1F("hEFmassTrkSysMPNCorr", "MassTrkSys at EF m-prong non-corrected; MassTrkSys at EF [GeV]; Candidates",50,-0.1,15.0));
    addHistogram(new TH1F("hEFmEflowApproxMPNCorr", "mEflowApprox at EF m-prong non-corrected;  mEflowApprox at EF ; Candidates",61,-0.2,60.2));
    addProfile(new TProfile("hEFinnerTrkAvgDistMPNCmu", "InnerTrkAvgDist at EF vs mu m-prong non-corrected; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFetOverPtLeadTrkMPNCmu", "EtOverPtLeadTrk at EF vs mu m-prong non-corrected; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFChPiEMEOverCaloEMEMPNCmu", "ChPiEMEOverCaloEME at EF vs mu mp non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFEMPOverTrkSysPMPNCmu", "EMPOverTrkSysP at EF vs mu m-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFcentFracMPNCmu", "Centrality Fraction at EF vs mu m-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFptRatioEflowApproxMPNCmu", "ptRatioEflowApprox at EF vs mu mp non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFdRmaxMPNCmu", "Max dR of associated tracks at EF vs mu m-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFtrFlightPathSigMPNCmu", "TrFlightPathSig at EF vs mu m-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFmassTrkSysMPNCmu", "MassTrkSys at EF vs mu m-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFmEflowApproxMPNCmu", "mEflowApprox at EF vs mu m-prong non-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    //BDT inputs for 1-prong mu-Corrected
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFTau/BDT/1p_Corrected",run, ATTRIB_MANAGED, ""));
    addHistogram(new TH1F("hEFinnerTrkAvgDist1PCorr", "Inner Track Average Distance at EF 1-prong mu-corrected; innertrkAvgDist at EF; Candidates",50,-0.05,0.5));
    addHistogram(new TH1F("hEFetOverPtLeadTrk1PCorr", "Et over Lead Track Pt at EF 1-prong mu-corrected; etOverPtLeadTrk at EF; Candidates",51,-0.1,25.0));
    addHistogram(new TH1F("hEFipSigLeadTrk1PCorr", "IpSigLeadTrk at EF 1-prong mu-corrected; IpSigLeadTrk at EF; Candidates",50,-20.0,20.0));
    addHistogram(new TH1F("hEFSumPtTrkFrac1PCorr", "SumPtTrkFrac at EF 1-prong mu-corrected; SumPtTrkFrac at EF; Candidates",50,-0.5,1.1));
    addHistogram(new TH1F("hEFChPiEMEOverCaloEME1PCorr", "ChPiEMEOverCaloEME at EF 1-prong mu-corrected; ChPiEMEOverCaloEME at EF; Candidates", 51,-20.0,20.0));
    addHistogram(new TH1F("hEFEMPOverTrkSysP1PCorr", "EMPOverTrkSysP at EF 1-prong mu-corrected; EMPOverTrkSysP at EF; Candidates", 41,0.0,40.0));
    addHistogram(new TH1F("hEFcentFrac1PCorr", "Centrality Fraction at EF 1-prong mu-corrected; centFrac at EF; Candidates",50,-0.05,1.2));
    addHistogram(new TH1F("hEFptRatioEflowApprox1PCorr", "ptRatioEflowApprox at EF 1-prong mu-corrected; ptRatioEflowApprox at EF; Candidates",50,0.0,2.0));
    addProfile(new TProfile("hEFinnerTrkAvgDist1PCmu", "InnerTrkAvgDist at EF vs mu 1-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFetOverPtLeadTrk1PCmu", "EtOverPtLeadTrk at EF vs mu 1-prong mu-corrected;Average interactions per bunch crossing ;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFipSigLeadTrk1PCmu", "IpSigLeadTrk at EF vs mu 1-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFSumPtTrkFrac1PCmu", "SumPtTrkFrac at EF vs mu 1-prong mu-corrected;Average interactions per bunch crossing ;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFChPiEMEOverCaloEME1PCmu", "ChPiEMEOverCaloEME at EF vs mu 1p mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFEMPOverTrkSysP1PCmu", "EMPOverTrkSysP at EF vs mu 1-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFcentFrac1PCmu", "Centrality Fraction at EF vs mu 1-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFptRatioEflowApprox1PCmu", "ptRatioEflowApprox at EF vs mu 1p mu-corrected;Average interactions per bunch crossing ;",nbin_mu-1,bins_mu));
    //BDT inputs for 3-prong mu-Corrected
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFTau/BDT/mp_Corrected",run, ATTRIB_MANAGED, ""));
    addHistogram(new TH1F("hEFinnerTrkAvgDistMPCorr", "Inner Track Average Distance at EF m-prong mu-corrected; innertrkAvgDist at EF; Candidates",50,-0.05,0.5));
    addHistogram(new TH1F("hEFetOverPtLeadTrkMPCorr", "Et over Lead Track Pt at EF m-prong mu-corrected; etOverPtLeadTrk at EF; Candidates",51,-0.1,25.0));
    addHistogram(new TH1F("hEFChPiEMEOverCaloEMEMPCorr", "ChPiEMEOverCaloEME at EF m-prong mu-corrected; ChPiEMEOverCaloEME at EF; Candidates", 51,-20.0,20.0));
    addHistogram(new TH1F("hEFEMPOverTrkSysPMPCorr", "EMPOverTrkSysP at EF m-prong mu-corrected; EMPOverTrkSysP at EF; Candidates", 41,0.0,40.0));
    addHistogram(new TH1F("hEFcentFracMPCorr", "Centrality Fraction at EF m-prong mu-corrected; centFrac at EF; Candidates",50,-0.05,1.2));
    addHistogram(new TH1F("hEFptRatioEflowApproxMPCorr", "ptRatioEflowApprox at EF m-prong mu-corrected; ptRatioEflowApprox at EF; Candidates",50,0.0,2.0));
    addHistogram(new TH1F("hEFdRmaxMPCorr", "Max dR of associated tracks at EF m-prong mu-corrected; dRmax at EF; Candidates",50,-0.1,0.3));
    addHistogram(new TH1F("hEFtrFlightPathSigMPCorr", "TrFlightPathSig at EF m-prong mu-corrected; trFlightPathSig at EF; Candidates",50,-20.0,20.0));
    addHistogram(new TH1F("hEFmassTrkSysMPCorr", "MassTrkSys at EF m-prong mu-corrected; massTrkSys at EF [GeV]; Candidates",50,-0.1,15.0));
    addHistogram(new TH1F("hEFmEflowApproxMPCorr", "mEflowApprox at EF m-prong mu-corrected;  mEflowApprox at EF ; Candidates",61,-0.2,60.2));
    addProfile(new TProfile("hEFinnerTrkAvgDistMPCmu", "InnerTrkAvgDist at EF vs mu m-prong mu-corrected; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFetOverPtLeadTrkMPCmu", "EtOverPtLeadTrk at EF vs mu m-prong mu-corrected; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFChPiEMEOverCaloEMEMPCmu", "ChPiEMEOverCaloEME at EF vs mu mp mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFEMPOverTrkSysPMPCmu", "EMPOverTrkSysP at EF vs mu m-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFcentFracMPCmu", "Centrality Fraction at EF vs mu m-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFptRatioEflowApproxMPCmu", "ptRatioEflowApprox at EF vs mu mp mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFdRmaxMPCmu", "Max dR of associated tracks at EF vs mu m-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFtrFlightPathSigMPCmu", "TrFlightPathSig at EF vs mu m-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFmassTrkSysMPCmu", "MassTrkSys at EF vs mu m-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEFmEflowApproxMPCmu", "mEflowApprox at EF vs mu m-prong mu-corrected;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    //--------------------
    // L1 vs Offline
    //--------------------
    
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/L1VsOffline",run));
    addHistogram(new TH1F("hL1EtRatio","L1 Et Relative difference; Et relative diff; Et relative diff",50,-0.8,0.8));
    
    //--------------------
    //Pre selection vs Offline
    //--------------------
    
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/PreselectionVsOffline",run));
    addHistogram(new TH2F("hPreselvsOffnTrks","nTrks at FTF vs Off; nTrks off; nTrks FTF",10,0,10,10,0,10));
    addHistogram(new TH2F("hPreselvsOffnWideTrks","nWideTrks at FTF vs Off; nWideTrks off; nWideTrks FTF",10,0,10,10,0,10));
    addHistogram(new TH1F("hEFEtRatio","FTF-Offline Et Relative difference; Et relative diff; Et relative diff",50,-0.3,0.3));
    addHistogram(new TH1F("hEtaRatio","FTF-Offline Eta Relative difference; Eta relative diff; Eta relative diff",50,-0.3,0.3));
    addHistogram(new TH1F("hPhiRatio","FTF-Offline Phi Relative difference; Phi relative diff; Phi relative diff",50,-0.05,0.05));
    
    if(m_truth){
        //----------------
        // EF vs Truth
        //----------------
        addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFVsTruth",run, ATTRIB_MANAGED, ""));
        addProfile(new TProfile("hEtRatiovspt","Relative difference in Et (EF-Truth)/Truth vs Truth Tau pT;Truth Tau pT;",nbin_pt-1,bins_pt));
        addProfile(new TProfile("hEtRatiovseta","Relative difference in Et (EF-Truth)/Truth vs Truth Tau eta;Truth Tau #eta;",nbin_eta-1,bins_eta));
        addProfile(new TProfile("hEtRatiovsphi","Relative difference in Et (EF-Truth)/Truth vs Truth Tau phi;Truth Tau #phi;",40,-3.2,3.2));
        addProfile(new TProfile("hEtRatiovsmu","Relative difference in Et (EF-Truth)/Truth vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    }
    //-------------------
    //EF vs Offline
    //-------------------
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFVsOffline",run, ATTRIB_MANAGED, ""));
    //Basic Kinematic Vars
    addHistogram(new TH1F("hptRatio","Relative difference in pt (EF-Offline)/Offline; pt relative diff;", 100,-0.3,0.3));
    addHistogram(new TH1F("hetaRatio","Relative difference in eta (EF-Offline)/Offline; eta relative diff;", 100,-0.3,0.3));
    addHistogram(new TH1F("hphiRatio","Relative difference in phi (EF-Offline)/Offline; phi relative diff;", 100,-0.2,0.2));
    addHistogram(new TH2F("hEFvsOffnTrks","nTrks at EF vs Off; nTrks off; nTrks EF",10,0,10,10,0,10));
    addHistogram(new TH2F("hEFvsOffnWideTrks","nWideTrks at EF vs Off; nWideTrks off; nWideTrks EF",10,0,10,10,0,10));
    //Other Vars
    //addHistogram(new TH1F("hEMRadiusRatio","Relative difference in EMRadius ;EMRadius relative diff;",50,-0.3,0.3));
    //addHistogram(new TH1F("hHadRadiusRatio","Relative difference in HadRadius ;HadRadius relative diff;",50,-0.3,0.3));
    addHistogram(new TH1F("hIsoFracRatio","Relative difference in IsoFrac;IsoFrac relative diff;",50,-0.3,0.3));
    //addHistogram(new TH1F("hPSSFracRatio","Relative difference in PSSFrac;PSSFrac relative diff;",50,-0.3,0.3));
    addHistogram(new TH1F("hEMFracRatio","Relative difference in EMFrac;EMFrac relative diff;",50,-0.3,0.3));
    addHistogram(new TH1F("hEtRawRatio","Relative difference in EtRaw;EtRaw relative diff;",50,-0.3,0.3));
    addHistogram(new TH1F("hOffEFEMDiff","Relative difference in EM energy (EF-Offline)/Offline; EM energy relative diff;", 50,-0.1,0.1));
    addHistogram(new TH1F("hOffEFHADDiff","Relative difference in HAD energy (EF-Offline)/Offline; HAD energy relative diff;", 50,-0.1,0.1));
    //TProfile
    addProfile(new TProfile("hEtRawRatiovspt","Relative difference in EtRaw (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEtRawRatiovseta","Relative difference in EtRaw (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    addProfile(new TProfile("hEtRawRatiovsphi","Relative difference in EtRaw (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    addProfile(new TProfile("hEtRawRatiovsmu","Relative difference in EtRaw (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    addProfile(new TProfile("hEtRatiovspt","Relative difference in Et (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEtRatiovseta","Relative difference in Et (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    addProfile(new TProfile("hEtRatiovsphi","Relative difference in Et (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    addProfile(new TProfile("hEtRatiovsmu","Relative difference in Et (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    //addProfile(new TProfile("hEMRadiusRatiovspt","Relative difference in EMRadius (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    //addProfile(new TProfile("hEMRadiusRatiovseta","Relative difference in EMRadius (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    //addProfile(new TProfile("hEMRadiusRatiovsphi","Relative difference in EMRadius (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    //addProfile(new TProfile("hEMRadiusRatiovsmu","Relative difference in EMRadius (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    //addProfile(new TProfile("hHADRadiusRatiovspt","Relative difference in HADRadius (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    //addProfile(new TProfile("hHADRadiusRatiovseta","Relative difference in HADRadius (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    //addProfile(new TProfile("hHADRadiusRatiovsphi","Relative difference in HADRadius (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    //addProfile(new TProfile("hHADRadiusRatiovsmu","Relative difference in HADRadius (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    addProfile(new TProfile("hIsoFracRatiovspt","Relative difference in IsoFrac (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hIsoFracRatiovseta","Relative difference in IsoFrac (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    addProfile(new TProfile("hIsoFracRatiovsphi","Relative difference in IsoFrac (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    addProfile(new TProfile("hIsoFracRatiovsmu","Relative difference in IsoFrac (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    addProfile(new TProfile("hCentFracRatiovspt","Relative difference in CentFrac (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hCentFracRatiovseta","Relative difference in CentFrac (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    addProfile(new TProfile("hCentFracRatiovsphi","Relative difference in CentFrac (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    addProfile(new TProfile("hCentFracRatiovsmu","Relative difference in CentFrac (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    addProfile(new TProfile("hEMFracRatiovspt","Relative difference in EMFrac (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEMFracRatiovseta","Relative difference in EMFrac (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    addProfile(new TProfile("hEMFracRatiovsphi","Relative difference in EMFrac (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    addProfile(new TProfile("hEMFracRatiovsmu","Relative difference in EMFrac (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    //addProfile(new TProfile("hPSSFracRatiovspt","Relative difference in PSSFrac (EF-Offline)/Offline vs Offline Tau pT;Offline Tau pT;",nbin_pt-1,bins_pt));
    //addProfile(new TProfile("hPSSFracRatiovseta","Relative difference in PSSFrac (EF-Offline)/Offline vs Offline Tau eta;Offline Tau #eta;",nbin_eta-1,bins_eta));
    //addProfile(new TProfile("hPSSFracRatiovsphi","Relative difference in PSSFrac (EF-Offline)/Offline vs Offline Tau phi;Offline Tau #phi;",40,-3.2,3.2));
    //addProfile(new TProfile("hPSSFracRatiovsmu","Relative difference in PSSFrac (EF-Offline)/Offline vs mu;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    //BDT inputs 1p non corrected
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFVsOffline/BDT/1p_nonCorrected",run, ATTRIB_MANAGED, ""));
    addHistogram(new TH1F("hInnerTrkAvgDistRatio1P","Relative diff in innertrkAvgDist (EF-Offline)/Offline; InnerTrkAvgDist 1-prong relative diff;",40,-2.0,2.0));
    addHistogram(new TH1F("hEtOverPtLeadTrkRatio1P","Relative diff in EtOverPtLeadTrk ;EtOverPtLeadTrack 1-prong relative diff;",40,-0.2,0.2));
    addHistogram(new TH1F("hIpSigLeadTrkRatio1P","Relative diff in IpSigLeadTrk (EF-Offline)/Offline; IpSigLeadTrk 1-prong relative diff;",40,-2.0,2.0));
    addHistogram(new TH1F("hSumPtTrkFracRatio1P","Relative diff in SumPtTrkFrac (EF-Offline)/Offline; SumPtTrkFrac 1-prong relative diff;",40,-2.0,2.0));
    addHistogram(new TH1F("hChPiEMEOverCaloEMERatio1P","Relative diff in ChPiEMEOverCaloEME (EF-Offline)/Offline; ChPiEMEOverCaloEME 1-prong relative diff;",40,-1,1));
    addHistogram(new TH1F("hEMPOverTrkSysPRatio1P","Relative diff in EMPOverTrkSysP (EF-Offline)/Offline; EMPOverTrkSysP 1-prong relative diff;",40,-0.2,0.2));
    addHistogram(new TH1F("hCentFracRatio1P","Relative diff in Centrality Fraction (EF-Offline)/Offline; CentFrac 1-prong relative diff;",40,-0.1,0.1));
    addHistogram(new TH1F("hPtRatioEflowApproxRatio1P","Relative diff in ptRatioEflowApprox (EF-Offline)/Offline; ptRatioEflowApprox 1-prong rel diff;", 40,-0.2,0.2));
    addHistogram(new TH1F("hDRmaxRatio1P","Relative diff in DRmax (EF-Offline)/Offline; DRmax 1-prong relative diff;", 40,-2.0,2.0));
    addHistogram(new TH1F("hTopoInvMassRatio1P","Relative diff in TopoInvMass (EF-Offline)/Offline; TopoInvMass 1-prong relative diff;", 40,-1.0,1.0));
    addProfile(new TProfile("hInnerTrkAvgDistVspt1P","Relative diff in innertrkAvgDist (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hInnerTrkAvgDistVsmu1P","Rel diff in innertrkAvgDist (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEtOverPtLeadTrkVspt1P","Relative diff in EtOverPtLeadTrk (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEtOverPtLeadTrkVsmu1P","Rel diff in EtOverPtLeadTrk (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hIpSigLeadTrkVspt1P","Relative diff in IpSigLeadTrk (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hIpSigLeadTrkVsmu1P","Relative diff in IpSigLeadTrk (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hSumPtTrkFracVspt1P","Relative diff in SumPtTrkFrac (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hSumPtTrkFracVsmu1P","Relative diff in SumPtTrkFrac (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hChPiEMEOvCaloEMEVspt1P","Relative diff in ChPiEMEOverCaloEME (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hChPiEMEOvCaloEMEVsmu1P","Rel diff in ChPiEMEOvCaloEME(EF-Off)/Off vs mu 1p;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEMPOverTrkSysPVspt1P","Relative diff in EMPOverTrkSysP (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEMPOverTrkSysPVsmu1P","Rel diff in EMPOverTrkSysP (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hCentFracVspt1P","Relative diff in CentFrac (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hCentFracVsmu1P","Relative diff in CentFrac (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hPtRatioEflowApproxVspt1P","Relative diff in PtRatioEflowApprox (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hPtRatioEflowApproxVsmu1P","Rel diff in PtRatioEflowApprox(EF-Off)/Off vs mu 1p;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hDRmaxVspt1P","Relative diff in DRmax (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hDRmaxVsmu1P","Relative diff in DRmax (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hTopoInvMassVspt1P","Relative diff in TopoInvMass (EF-Off)/Off vs Off Tau pT 1-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hTopoInvMassVsmu1P","Relative diff in TopoInvMass (EF-Off)/Off vs mu 1-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    //BDT inputs mp non corrected
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/"+trigItem+"/EFVsOffline/BDT/mp_nonCorrected",run, ATTRIB_MANAGED, ""));
    addHistogram(new TH1F("hInnerTrkAvgDistRatioMP","Relative diff in innertrkAvgDist (EF-Offline)/Offline; InnerTrkAvgDist m-prong relative diff;",40,-2.0,2.0));
    addHistogram(new TH1F("hEtOverPtLeadTrkRatioMP","Relative diff in etOverPtLeadTrk (EF-Offline)/Offline; etOverPtLeadTrk m-prong relative diff;",40,-0.2,0.2));
    addHistogram(new TH1F("hChPiEMEOverCaloEMERatioMP","Relative diff in ChPiEMEOverCaloEME (EF-Offline)/Offline; ChPiEMEOverCaloEME m-prong relative diff;",40,-1,1));
    addHistogram(new TH1F("hEMPOverTrkSysPRatioMP","Relative diff in EMPOverTrkSysP (EF-Offline)/Offline; EMPOverTrkSysP m-prong relative diff;",40,-0.2,0.2));
    addHistogram(new TH1F("hCentFracRatioMP","Relative diff in Centrality Fraction (EF-Offline)/Offline; centFrac m-prong relative diff;",40,-0.1,0.1));
    addHistogram(new TH1F("hPtRatioEflowApproxRatioMP","Relative diff in ptRatioEflowApprox (EF-Offline)/Offline; ptRatioEflowApprox m-prong rel diff;", 40,-0.2,0.2));
    addHistogram(new TH1F("hDRmaxRatioMP","Relative diff in DRmax (EF-Offline)/Offline; dRmax m-prong relative diff;", 40,-1.0,1.0));
    addHistogram(new TH1F("hTrFlightPathSigRatioMP","Relative diff in TrFlightPathSig (EF-Offline)/Offline; trFlightPathSig m-prong relative diff;", 40,-1.0,1.0));
    addHistogram(new TH1F("hMassTrkSysRatioMP","Relative diff in MassTrkSys (EF-Offline)/Offline; MassTrkSys m-prong relative diff;", 40,-0.5,0.5));
    addHistogram(new TH1F("hMEflowApproxRatioMP","Relative diff in mEflowApprox (EF-Offline)/Offline; mEflowApprox m-prong rel diff;", 40,-0.5,0.5));
    addProfile(new TProfile("hInnerTrkAvgDistVsptMP","Relative diff in innertrkAvgDist (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hInnerTrkAvgDistVsmuMP","Rel diff in innertrkAvgDist (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEtOverPtLeadTrkVsptMP","Relative diff in EtOverPtLeadTrk (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEtOverPtLeadTrkVsmuMP","Rel diff in EtOverPtLeadTrk (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hChPiEMEOvCaloEMEVsptMP","Relative diff in ChPiEMEOverCaloEME (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hChPiEMEOvCaloEMEVsmuMP","Rel diff in ChPiEMEOvCaloEME(EF-Off)/Off vs mu mp;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hEMPOverTrkSysPVsptMP","Relative diff in EMPOverTrkSysP (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hEMPOverTrkSysPVsmuMP","Rel diff in EMPOverTrkSysP (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hCentFracVsptMP","Relative diff in CentFrac (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hCentFracVsmuMP","Relative diff in CentFrac (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hPtRatioEflowApproxVsptMP","Relative diff in PtRatioEflowApprox (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hPtRatioEflowApproxVsmuMP","Rel diff in PtRatioEflowApprox(EF-Off)/Off vs mu mp;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hDRmaxVsptMP","Relative diff in DRmax (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hDRmaxVsmuMP","Relative diff in DRmax (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hTrFlightPathSigVsptMP","Relative diff in TrFlightPathSig (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hTrFlightPathSigVsmuMP","Rel diff in TrFlightPathSig (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hMassTrkSysVsptMP","Relative diff in MassTrkSys (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hMassTrkSysVsmuMP","Relative diff in MassTrkSys (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addProfile(new TProfile("hMEflowApproxVsptMP","Relative diff in mEflowApprox (EF-Off)/Off vs Off Tau pT m-prong;Offline Tau pT;",nbin_pt-1,bins_pt));
    addProfile(new TProfile("hMEflowApproxVsmuMP","Relative diff in mEflowApprox (EF-Off)/Off vs mu m-prong;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    
    //--------------------
    //Turn On Curves
    //--------------------
    
    if(m_turnOnCurves)
    {
        
        if(m_truth){
            //Truth
            addMonGroup( new MonGroup(this, "HLT/TauMon/Expert/"+trigItem+"/TurnOnCurves/TruthEfficiency",run, ATTRIB_MANAGED, "") );
            
            addHistogram(new TH1F("hTrueTauPtDenom",";Truth p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueTauPt1PDenom",";Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueTauPt3PDenom",";Truth 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueTauEtaDenom",";#eta;",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTrueTauPhiDenom",";#phi;",16,-3.2,3.2));
            addHistogram(new TH1F("hTrueTauNTrackDenom",";Number of tracks;",10,0,10));
            addHistogram(new TH1F("hTrueTauNVtxDenom",";Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTrueTauMuDenom",";Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTrueTauEtaVsPhiDenom",";#eta; #phi;",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTrueL1PtNum","L1 vs Truth;Truth p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueL1Pt1PNum","L1 vs Truth;Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueL1Pt3PNum","L1 vs Truth;Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueL1EtaNum","L1 vs Truth; #eta;",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTrueL1PhiNum","L1 vs Truth;#phi;",16,-3.2,3.2));
            addHistogram(new TH1F("hTrueL1NTrackNum","L1 vs Truth;Number of tracks;",10,0,10));
            addHistogram(new TH1F("hTrueL1NVtxNum","L1 vs Truth;Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTrueL1MuNum","L1 vs Truth;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTrueL1EtaVsPhiNum","L1 vs Truth;#eta;#phi;",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTrueHLTPtNum","HLT vs Truth;Truth p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueHLTPt1PNum","HLT vs Truth;Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueHLTPt3PNum","HLT vs Truth;Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueHLTEtaNum","HLT vs Truth; #eta;",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTrueHLTPhiNum","HLT vs Truth;#phi;",16,-3.2,3.2));
            addHistogram(new TH1F("hTrueHLTNTrackNum","HLT vs Truth;Number of tracks;",10,0,10));
            addHistogram(new TH1F("hTrueHLTNVtxNum","HLT vs Truth;Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTrueHLTMuNum","HLT vs Truth;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTrueHLTEtaVsPhiNum","HLT vs Truth;#eta;#phi;",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTrueL1PtEfficiency","L1 vs Truth Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueL1Pt1PEfficiency","L1 vs Truth Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueL1Pt3PEfficiency","L1 vs Truth Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueL1EtaEfficiency","L1 vs Truth Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTrueL1PhiEfficiency","L1 vs Truth Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addHistogram(new TH1F("hTrueL1NTrackEfficiency","L1 vs Truth Efficiency; Number of tracks; Efficiency",10,0,10));
            addHistogram(new TH1F("hTrueL1NVtxEfficiency","L1 vs Truth Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTrueL1MuEfficiency","L1 vs Truth Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTrueL1EtaVsPhiEfficiency","L1 vs Truth in Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTrueHLTPtEfficiency","HLT vs Truth Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueHLTPt1PEfficiency","HLT vs Truth Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueHLTPt3PEfficiency","HLT vs Truth Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTrueHLTEtaEfficiency","HLT vs Truth Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTrueHLTPhiEfficiency","HLT vs Truth Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addHistogram(new TH1F("hTrueHLTNTrackEfficiency","HLT vs Truth Efficiency; Number of tracks; Efficiency",10,0,10));
            addHistogram(new TH1F("hTrueHLTNVtxEfficiency","HLT vs Truth Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTrueHLTMuEfficiency","HLT vs Truth Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTrueHLTEtaVsPhiEfficiency","HLT vs truth in Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addProfile(new TProfile("TProfTrueL1PtEfficiency","L1 vs Truth Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTrueL1Pt1PEfficiency","L1 vs Truth Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTrueL1Pt3PEfficiency","L1 vs Truth Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTrueL1EtaEfficiency","L1 vs Truth Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addProfile(new TProfile("TProfTrueL1PhiEfficiency","L1 vs Truth Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addProfile(new TProfile("TProfTrueL1NTrackEfficiency","L1 vs Truth Efficiency; Number of tracks; Efficiency",10,0,10));
            addProfile(new TProfile("TProfTrueL1NVtxEfficiency","L1 vs Truth Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addProfile(new TProfile("TProfTrueL1MuEfficiency","L1 vs Truth Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            
            addProfile(new TProfile("TProfTrueHLTPtEfficiency","HLT vs Truth Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTrueHLTPt1PEfficiency","HLT vs Truth Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTrueHLTPt3PEfficiency","HLT vs Truth Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTrueHLTEtaEfficiency","HLT vs Truth Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addProfile(new TProfile("TProfTrueHLTPhiEfficiency","HLT vs Truth Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addProfile(new TProfile("TProfTrueHLTNTrackEfficiency","HLT vs Truth Efficiency; Number of tracks; Efficiency",10,0,10));
            addProfile(new TProfile("TProfTrueHLTNVtxEfficiency","HLT vs Truth Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addProfile(new TProfile("TProfTrueHLTMuEfficiency","HLT vs Truth Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            
            
            //Truth+Reco
            addMonGroup( new MonGroup(this, "HLT/TauMon/Expert/"+trigItem+"/TurnOnCurves/Truth+RecoEfficiency",run, ATTRIB_MANAGED, "") );
            
            addHistogram(new TH1F("hTruthRecoTauPtDenom","Truth p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoTauPt1PDenom",";Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoTauPt3PDenom",";Truth 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoTauEtaDenom",";#eta;",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTruthRecoTauPhiDenom",";#phi;",16,-3.2,3.2));
            addHistogram(new TH1F("hTruthRecoTauNTrackDenom",";Number of tracks;",10,0,10));
            addHistogram(new TH1F("hTruthRecoTauNVtxDenom",";Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTruthRecoTauMuDenom",";Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTruthRecoTauEtaVsPhiDenom",";#eta;#phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTruthRecoL1PtNum","L1 vs Truth+Reco; Truth p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoL1Pt1PNum","L1 vs Truth+Reco; Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoL1Pt3PNum","L1 vs Truth+Reco; Truth 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoL1EtaNum","L1 vs Truth+Reco; #eta;",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTruthRecoL1PhiNum","L1 vs Truth+Reco #phi;;",16,-3.2,3.2));
            addHistogram(new TH1F("hTruthRecoL1NTrackNum","L1 vs Truth+Reco; Number of tracks;",10,0,10));
            addHistogram(new TH1F("hTruthRecoL1NVtxNum","L1 vs Truth+Reco; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTruthRecoL1MuNum","L1 vs Truth+Reco;Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTruthRecoL1EtaVsPhiNum","L1 vs Truth+Reco;#eta;#phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTruthRecoHLTPtNum","HLT vs Truth+Reco; Truth p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoHLTPt1PNum","HLT vs Truth+Reco; Truth 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoHLTPt3PNum","HLT vs Truth+Reco; Truth 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoHLTEtaNum","HLT vs Truth+Reco; #eta;",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTruthRecoHLTPhiNum","HLT vs Truth+Reco; #phi;",16,-3.2,3.2));
            addHistogram(new TH1F("hTruthRecoHLTNTrackNum","HLT vs Truth+Reco; Number of tracks;",10,0,10));
            addHistogram(new TH1F("hTruthRecoHLTNVtxNum","HLT vs Truth+Reco; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTruthRecoHLTMuNum","HLT vs Truth+Reco; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTruthRecoHLTEtaVsPhiNum","HLT vs Truth+Reco; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTruthRecoL1PtEfficiency","L1 vs Truth+Reco Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoL1Pt1PEfficiency","L1 vs Truth+Reco Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoL1Pt3PEfficiency","L1 vs Truth+Reco Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoL1EtaEfficiency","L1 vs Truth+Reco Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTruthRecoL1PhiEfficiency","L1 vs Truth+Reco Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addHistogram(new TH1F("hTruthRecoL1NTrackEfficiency","L1 vs Truth+Reco Efficiency; Number of tracks; Efficiency",10,0,10));
            addHistogram(new TH1F("hTruthRecoL1NVtxEfficiency","L1 vs Truth+Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTruthRecoL1MuEfficiency","L1 vs Truth+Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTruthRecoL1EtaVsPhiEfficiency","L1 vs Truth+Reco in Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addHistogram(new TH1F("hTruthRecoHLTPtEfficiency","HLT vs Truth+Reco Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoHLTPt1PEfficiency","HLT vs Truth+Reco Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoHLTPt3PEfficiency","HLT vs Truth+Reco Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hTruthRecoHLTEtaEfficiency","HLT vs Truth+Reco Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addHistogram(new TH1F("hTruthRecoHLTPhiEfficiency","HLT vs Truth+Reco Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addHistogram(new TH1F("hTruthRecoHLTNTrackEfficiency","HLT vs Truth+Reco Efficiency; Number of tracks; Efficiency",10,0,10));
            addHistogram(new TH1F("hTruthRecoHLTNVtxEfficiency","HLT vs Truth+Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addHistogram(new TH1F("hTruthRecoHLTMuEfficiency","HLT vs Truth+Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            addHistogram(new TH2F("hTruthRecoHLTEtaVsPhiEfficiency","HLT vs Truth+Reco in  Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
            
            addProfile(new TProfile("TProfTruthRecoL1PtEfficiency","L1 vs Truth+Reco Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTruthRecoL1Pt1PEfficiency","L1 vs Truth+Reco Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTruthRecoL1Pt3PEfficiency","L1 vs Truth+Reco Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTruthRecoL1EtaEfficiency","L1 vs Truth+Reco Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addProfile(new TProfile("TProfTruthRecoL1PhiEfficiency","L1 vs Truth+Reco Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addProfile(new TProfile("TProfTruthRecoL1NTrackEfficiency","L1 vs Truth+Reco Efficiency; Number of tracks; Efficiency",10,0,10));
            addProfile(new TProfile("TProfTruthRecoL1NVtxEfficiency","L1 vs Truth+Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addProfile(new TProfile("TProfTruthRecoL1MuEfficiency","L1 vs Truth+Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            
            addProfile(new TProfile("TProfTruthRecoHLTPtEfficiency","HLT vs Truth+Reco Efficiency; Truth p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTruthRecoHLTPt1PEfficiency","HLT vs Truth+Reco Efficiency; Truth 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTruthRecoHLTPt3PEfficiency","HLT vs Truth+Reco Efficiency; Truth 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfTruthRecoHLTEtaEfficiency","HLT vs Truth+Reco Efficiency; Truth #eta; Efficiency",nbin_eta-1,bins_eta));
            addProfile(new TProfile("TProfTruthRecoHLTPhiEfficiency","HLT vs Truth+Reco Efficiency; Truth #phi; Efficiency",16,-3.2,3.2));
            addProfile(new TProfile("TProfTruthRecoHLTNTrackEfficiency","HLT vs Truth+Reco Efficiency; Number of tracks; Efficiency",10,0,10));
            addProfile(new TProfile("TProfTruthRecoHLTNVtxEfficiency","HLT vs Truth+Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
            addProfile(new TProfile("TProfTruthRecoHLTMuEfficiency","HLT vs Truth+Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
            
            //--------------------
            //Efficiency Histograms for Combined Triggers
            //--------------------
            
            addMonGroup( new MonGroup(this, "HLT/TauMon/Expert/"+trigItem+"/TurnOnCurves/TauComboEfficiency",run, ATTRIB_MANAGED, "") );
            addHistogram(new TH1F("hCombTauPtDenom",";True Tau p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombL1TauPtNum",";True Tau p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombHLTTauPtNum",";True Tau p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombL1TauPtEfficiency","L1 vs tau+tau Efficiency; True Tau p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombHLTTauPtEfficiency","HLT vs tau+tau Efficiency; True Tau p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            
            addHistogram(new TH1F("hCombelPtDenom",";True el p_{T} [GeV];",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombL1elPtNum",";True el p_{T} [GeV];",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombHLTelPtNum",";True el p_{T} [GeV];",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombL1elPtEfficiency","L1 vs tau+el Efficiency; True el p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombHLTelPtEfficiency","HLT vs tau+el Efficiency; True el p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            
            addHistogram(new TH1F("hCombmuPtDenom",";True mu p_{T} [GeV];",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombL1muPtNum",";True mu p_{T} [GeV];",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombHLTmuPtNum",";True mu p_{T} [GeV];",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombL1muPtEfficiency","L1 vs tau+mu Efficiency; True mu p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            addHistogram(new TH1F("hCombHLTmuPtEfficiency","HLT vs tau+mu Efficiency; True mu p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            
            addHistogram(new TH1F("hCombOffjetPtDenom",";Offline jet mu p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombL1OffjetPtNum",";Offline jet mu p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombHLTOffjetPtNum",";Offline jet mu p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombL1OffjetPtEfficiency","L1 vs tau+OffJet Efficiency; Offline jet p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hCombHLTOffjetPtEfficiency","HLT vs tau+OffJet Efficiency; Offline jet p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            
            addHistogram(new TH1F("hCombMETDenom",";MET;",nbin_met-1,bins_met));
            addHistogram(new TH1F("hCombL1METNum",";MET;",nbin_met-1,bins_met));
            addHistogram(new TH1F("hCombHLTMETNum",";MET;",nbin_met-1,bins_met));
            addHistogram(new TH1F("hCombL1METEfficiency","L1 vs tau+met Efficiency; MET; Efficiency",nbin_met-1,bins_met));
            addHistogram(new TH1F("hCombHLTMETEfficiency","HLT vs tau+met Efficiency; MET; Efficiency",nbin_met-1,bins_met));
            
            addHistogram(new TH1F("hCombdRDenom","; dR;",nbin_dr-1,bins_dr));
            addHistogram(new TH1F("hCombL1dRNum","; dR;",nbin_dr-1,bins_dr));
            addHistogram(new TH1F("hCombHLTdRNum","; dR;",nbin_dr-1,bins_dr));
            addHistogram(new TH1F("hCombL1dREfficiency","L1 vs dR Efficiency; dR; Efficiency",nbin_dr-1,bins_dr));
            addHistogram(new TH1F("hCombHLTdREfficiency","HLT vs dR Efficiency; dR; Efficiency",nbin_dr-1,bins_dr));
            
            addHistogram(new TH1F("hCombdEtaDenom","; d#eta;",40,0,4.0));
            addHistogram(new TH1F("hCombL1dEtaNum","; d#eta;",40,0,4.0));
            addHistogram(new TH1F("hCombHLTdEtaNum","; d#eta;",40,0,4.0));
            addHistogram(new TH1F("hCombL1dEtaEfficiency","L1 vs dEta Efficiency; d#eta; Efficiency",40,0,4.0));
            addHistogram(new TH1F("hCombHLTdEtaEfficiency","HLT vs dEta Efficiency; d#eta; Efficiency",40,0,4.0));
            
            addHistogram(new TH1F("hCombdPhiDenom","; d#phi;",16,0,3.2));
            addHistogram(new TH1F("hCombL1dPhiNum","; d#phi;",16,0,3.2));
            addHistogram(new TH1F("hCombHLTdPhiNum","; d#phi;",16,0,3.2));
            addHistogram(new TH1F("hCombL1dPhiEfficiency","L1 vs dPhi Efficiency; d#phi; Efficiency",16,0,3.2));
            addHistogram(new TH1F("hCombHLTdPhiEfficiency","HLT vs dPhi Efficiency; d#phi; Efficiency",16,0,3.2));
            
            addProfile(new TProfile("TProfCombL1TauPtEfficiency", "L1 vs tau+tau Efficiency; True Tau p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfCombHLTTauPtEfficiency", "HLT vs tau+tau Efficiency; True Tau p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfCombL1elPtEfficiency","L1 vs tau+el Efficiency; True el p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            addProfile(new TProfile("TProfCombHLTelPtEfficiency","HLT vs tau+el Efficiency; True el p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            addProfile(new TProfile("TProfCombL1muPtEfficiency","L1 vs tau+mu Efficiency; True mu p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            addProfile(new TProfile("TProfCombHLTmuPtEfficiency","HLT vs tau+mu Efficiency; True mu p_{T} [GeV]; Efficiency",nbin_leppt-1,bins_leppt));
            addProfile(new TProfile("TProfCombL1OffjetPtEfficiency","L1 vs tau+OffJet Efficiency; Offline jet p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfCombHLTOffjetPtEfficiency","HLT vs tau+OffJet Efficiency; Offline jet p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addProfile(new TProfile("TProfCombL1METEfficiency","L1 vs tau+met Efficiency; MET; Efficiency",nbin_met-1,bins_met));
            addProfile(new TProfile("TProfCombHLTMETEfficiency","HLT vs tau+met Efficiency; MET; Efficiency",nbin_met-1,bins_met));
            addProfile(new TProfile("TProfCombL1dREfficiency","L1 vs dR Efficiency; dR; Efficiency",nbin_dr-1,bins_dr));
            addProfile(new TProfile("TProfCombHLTdREfficiency","HLT vs dR Efficiency; dR; Efficiency",nbin_dr-1,bins_dr));
            addProfile(new TProfile("TProfCombL1dEtaEfficiency","L1 vs dEta Efficiency; d#eta; Efficiency",40,0,4.0));
            addProfile(new TProfile("TProfCombHLTdEtaEfficiency","HLT vs dEta Efficiency; d#eta; Efficiency",40,0,4.0));
            addProfile(new TProfile("TProfCombL1dPhiEfficiency","L1 vs dPhi Efficiency; d#phi; Efficiency",16,0,3.2));
            addProfile(new TProfile("TProfCombHLTdPhiEfficiency","HLT vs dPhi Efficiency; d#phi; Efficiency",16,0,3.2));
            
        }
        
        //Reco
        addMonGroup( new MonGroup(this, "HLT/TauMon/Expert/"+trigItem+"/TurnOnCurves/RecoEfficiency",run, ATTRIB_MANAGED, "") );
        
        addHistogram(new TH1F("hRecoTauPtDenom",";Reco p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoTauPt1PDenom",";Reco 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoTauPt3PDenom",";Reco 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoTauEtaDenom","; #eta;",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRecoTauPhiDenom","; #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRecoTauNTrackDenom","; Number of tracks;",10,0,10));
        addHistogram(new TH1F("hRecoTauNVtxDenom","; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
        addHistogram(new TH1F("hRecoTauMuDenom","; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
        addHistogram(new TH2F("hRecoTauEtaVsPhiDenom","; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
        
        addHistogram(new TH1F("hRecoL1PtNum","L1 vs Reco; Reco p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoL1Pt1PNum","L1 vs Reco; Reco 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoL1Pt3PNum","L1 vs Reco; Reco 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoL1EtaNum","L1 vs Reco; #eta;",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRecoL1PhiNum","L1 vs Reco; #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRecoL1NTrackNum","L1 vs Reco Number of tracks;",10,0,10));
        addHistogram(new TH1F("hRecoL1NVtxNum","L1 vs Reco; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
        addHistogram(new TH1F("hRecoL1MuNum","L1 vs Reco; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
        addHistogram(new TH2F("hRecoL1EtaVsPhiNum","L1 vs Reco; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
        
        addHistogram(new TH1F("hRecoHLTPtNum","HLT vs Reco; Reco p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoHLTPt1PNum","HLT vs Reco; Reco 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoHLTPt3PNum","HLT vs Reco; Reco 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoHLTEtaNum","HLT vs Reco; #eta;",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRecoHLTPhiNum","HLT vs Reco; #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRecoHLTNTrackNum","HLT vs Reco; Number of tracks;",10,0,10));
        addHistogram(new TH1F("hRecoHLTNVtxNum","HLT vs Reco; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
        addHistogram(new TH1F("hRecoHLTMuNum","HLT vs Reco; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
        addHistogram(new TH2F("hRecoHLTEtaVsPhiNum","HLT vs Reco; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
        
        addHistogram(new TH1F("hRecoL1PtEfficiency","L1 vs Reco Efficiency; Reco p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoL1Pt1PEfficiency","L1 vs Reco Efficiency; Reco 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoL1Pt3PEfficiency","L1 vs Reco Efficiency; Reco 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoL1EtaEfficiency","L1 vs Reco Efficiency; Reco #eta; Efficiency",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRecoL1PhiEfficiency","L1 vs Reco Efficiency; Reco #phi; Efficiency",16,-3.2,3.2));
        addHistogram(new TH1F("hRecoL1NTrackEfficiency","L1 vs Reco Efficiency; Number of tracks; Efficiency",10,0,10));
        addHistogram(new TH1F("hRecoL1NVtxEfficiency","L1 vs Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
        addHistogram(new TH1F("hRecoL1MuEfficiency","L1 vs Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
        addHistogram(new TH2F("hRecoL1EtaVsPhiEfficiency","L1 vs Reco in Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
        
        addHistogram(new TH1F("hRecoHLTPtEfficiency","HLT vs Reco Efficiency; Reco p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoHLTPt1PEfficiency","HLT vs Reco Efficiency; Reco 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoHLTPt3PEfficiency","HLT vs Reco Efficiency; Reco 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRecoHLTEtaEfficiency","HLT vs Reco Efficiency; Reco #eta; Efficiency",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRecoHLTPhiEfficiency","HLT vs Reco Efficiency; Reco #phi; Efficiency",16,-3.2,3.2));
        addHistogram(new TH1F("hRecoHLTNTrackEfficiency","HLT vs Reco Efficiency; Number of tracks; Efficiency",10,0,10));
        addHistogram(new TH1F("hRecoHLTNVtxEfficiency","HLT vs Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
        addHistogram(new TH1F("hRecoHLTMuEfficiency","HLT vs Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
        addHistogram(new TH2F("hRecoHLTEtaVsPhiEfficiency","HLT vs Reco in  Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
        
        addProfile(new TProfile("TProfRecoL1PtEfficiency", "L1 Vs Reco Efficiency; Reco p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addProfile(new TProfile("TProfRecoL1Pt1PEfficiency", "L1 Vs Reco Efficiency; Reco 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addProfile(new TProfile("TProfRecoL1Pt3PEfficiency", "L1 Vs Reco Efficiency; Reco 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addProfile(new TProfile("TProfRecoL1EtaEfficiency", "L1 Vs Reco Efficiency; Reco #eta; Efficiency",nbin_eta-1,bins_eta));
        addProfile(new TProfile("TProfRecoL1PhiEfficiency", "L1 Vs Reco Efficiency; Reco #phi; Efficiency",16,-3.2,3.2));
        addProfile(new TProfile("TProfRecoL1NTrackEfficiency", "L1 Vs Reco Efficiency; Number of tracks; Efficiency",10,0,10));
        addProfile(new TProfile("TProfRecoL1NVtxEfficiency", "L1 Vs Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
        addProfile(new TProfile("TProfRecoL1MuEfficiency", "L1 Vs Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
        
        addProfile(new TProfile("TProfRecoHLTPtEfficiency", "HLT Vs Reco Efficiency; Reco p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addProfile(new TProfile("TProfRecoHLTPt1PEfficiency", "HLT Vs Reco Efficiency; Reco 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addProfile(new TProfile("TProfRecoHLTPt3PEfficiency", "HLT Vs Reco Efficiency; Reco 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        addProfile(new TProfile("TProfRecoHLTEtaEfficiency", "HLT Vs Reco Efficiency; Reco #eta; Efficiency",nbin_eta-1,bins_eta));
        addProfile(new TProfile("TProfRecoHLTPhiEfficiency", "HLT Vs Reco Efficiency; Reco #phi; Efficiency",16,-3.2,3.2));
        addProfile(new TProfile("TProfRecoHLTNTrackEfficiency", "HLT Vs Reco Efficiency; Number of tracks; Efficiency",10,0,10));
        addProfile(new TProfile("TProfRecoHLTNVtxEfficiency", "HLT Vs Reco Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
        addProfile(new TProfile("TProfRecoHLTMuEfficiency", "HLT Vs Reco Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
        
        //Reco
        
    }
    
}

void HLTTauMonTool::bookHistogramsAllItem(){
    
    if(m_RealZtautauEff)
    {
        const int nbin_pt = 13;
        double bins_pt[nbin_pt] = {10.,20.,25.,30.,35.,40.,45.,50.,60.,70.,100.,150.,200.};
        const int nbin_eta = 9;
        double bins_eta[nbin_eta] = {-2.47,-1.52,-1.37,-0.69,0.,0.69,1.37,1.52,2.47};
        
        addMonGroup( new MonGroup(this, "HLT/TauMon/Expert/RealZtautauEff",run, ATTRIB_MANAGED, "") );
        
        addHistogram(new TH1F("hRealBSTauPt",";Before selection Offline #tau p_{T} [GeV];",100,0,200));
        addHistogram(new TH1F("hRealBSTauEta",";Before selection Offline #tau #eta;",12,-2.5,2.5));
        addHistogram(new TH1F("hRealBSTauPhi",";Before selection Offline #tau #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealBSTauNTrack",";Before selection Number of #tau tracks;",10,0,10));
        addHistogram(new TH1F("hRealCandTau",";Before selection Total Number of Offline #taus;",10,0,10));
        addHistogram(new TH1F("hRealASTauPt",";After selection Offline #tau p_{T} [GeV];",100,0,200));
        addHistogram(new TH1F("hRealASTauEta"," ;After selection Offline #tau #eta;",12,-2.5,2.5));
        addHistogram(new TH1F("hRealASTauPhi",";After selection Offline #tau #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealASTauNTrack",";After selection Number of #tau tracks;",10,0,10));
        addHistogram(new TH1F("hRealSelectedTau",";After selection Number of Offline #taus;",10,0,10));
        
        addHistogram(new TH1F("hRealTauMu",";Number of Selected taus for selected muon;",10,0,10));
        addHistogram(new TH1F("hRealMuTau",";Number of Selected muons for selected tau;",10,0,10));
        
        addHistogram(new TH1F("hRealBSMuPt",";Before selection Offline #mu p_{T} [GeV];",100,0,200));
        addHistogram(new TH1F("hRealBSMuEta",";Before selection Offline #mu #eta;",12,-2.5,2.5));
        addHistogram(new TH1F("hRealBSMuPhi",";Before selection Offline #mu #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealCandMu",";Before selection Total Number of Offline #mu;",10,0,10));
        addHistogram(new TH1F("hRealASMuPt",";After selection Offline #mu p_{T} [GeV];",100,0,200));
        addHistogram(new TH1F("hRealASMuEta",";After selection Offline #mu #eta;",12,-2.5,2.5));
        addHistogram(new TH1F("hRealASMuPhi",";After selection Offline #mu #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealSelectedMu",";After selection Number of Offline #mu;",10,0,10));
        
        //MET
        addHistogram(new TH1F("hRealMET",";E^{Miss}_{T} [GeV];",50,0,120));
        
        //tau >=1, mu == 1 Vars
        addHistogram(new TH1F("hRealTauPt",";Offline #tau p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRealTauEta",";Offline #tau #eta;",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRealTauPhi",";Offline #tau #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealTauNTrack",";Number of #tau tracks;",10,0,10));
        addHistogram(new TH1F("hRealTauCharge",";Offline #tau Charge;",11,-5.5,5.5));
        addHistogram(new TH1F("hRealMuPt",";Offline #mu p_{T} [GeV];",nbin_pt-1,bins_pt));
        addHistogram(new TH1F("hRealMuEta",";Offline #mu #eta;",nbin_eta-1,bins_eta));
        addHistogram(new TH1F("hRealMuPhi",";Offline #mu #phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealMuCharge",";Offline #mu Charge;",11,-5.5,5.5));
        addHistogram(new TH1F("hRealTauMuCosdPhi",";Cos#Delta#phi;",16,-2,2));
        addHistogram(new TH1F("hRealMETMuTransMass",";m_{T}(#mu, E^{miss}_{T}) [GeV];",100,0,140));
        addHistogram(new TH1F("hRealTauMuVisMass",";m_{vis}(#mu, #tau_{h}) [GeV];",100,0,140));
        addHistogram(new TH1F("hRealTauMuDPhi",";#mu #tau #Delta#phi;",16,-3.2,3.2));
        addHistogram(new TH1F("hRealTauMuCharge",";Offline #tau+#mu Charge;",11,-5.5,5.5));
        
        CutItems.clear();
        TauCutFlow.clear();
        MuCutFlow.clear();
        
        TauCutFlow.push_back("No Cut");
        TauCutFlow.push_back("Tau Pt");
        TauCutFlow.push_back("Tau Eta");
        TauCutFlow.push_back("Tau Ntrack");
        TauCutFlow.push_back("Tau Charge");
        TauCutFlow.push_back("Tau JetBDTSigMedium");
        
        MuCutFlow.push_back("No Cut");
        MuCutFlow.push_back("Mu Pt");
        MuCutFlow.push_back("Mu Eta");
        MuCutFlow.push_back("Author MuIdCo");
        MuCutFlow.push_back("Quality cuts on CB #mu tracks");
        MuCutFlow.push_back("EtCone20Rel");
        MuCutFlow.push_back("PtCone20Rel");
        
        CutItems.push_back("No Cut");
        CutItems.push_back("Selected Tau");
        CutItems.push_back("Selected Mu");
        CutItems.push_back("Selected #tau+#mu");
        CutItems.push_back("#tau+#mu Charge");
        CutItems.push_back("#tau+#mu Vis Mass");
        CutItems.push_back("#tau #mu #Delta#phi");
        
        addHistogram(new TH1F("hCutFlow","; ;Events",CutItems.size(),0,CutItems.size()));
        addHistogram(new TH1F("hTauCutFlow","; ;Events",TauCutFlow.size(),0,TauCutFlow.size()));
        addHistogram(new TH1F("hMuCutFlow","; ;Events",MuCutFlow.size(),0,MuCutFlow.size()));
        
        for(unsigned int i=0;i<CutItems.size(); ++i)
        {
            hist("hCutFlow")->GetXaxis()->SetBinLabel(i+1,CutItems.at(i).c_str());
        }
        for(unsigned int i=0;i<TauCutFlow.size(); ++i)
        {
            hist("hTauCutFlow")->GetXaxis()->SetBinLabel(i+1,TauCutFlow.at(i).c_str());
        }
        for(unsigned int i=0;i<MuCutFlow.size(); ++i)
        {
            hist("hMuCutFlow")->GetXaxis()->SetBinLabel(i+1,MuCutFlow.at(i).c_str());
        }
        
        for(unsigned int i=0;i<m_trigItems.size();++i){
            addMonGroup( new MonGroup(this, "HLT/TauMon/Expert/RealZtautauEff/"+m_trigItems[i],run, ATTRIB_MANAGED, "") );
            addHistogram(new TH1F("hRealZttPtDenom","Offline Real Tau;Offline Tau p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hRealZttL1PtNum","L1 vs Offline Real Tau; Offline Tau p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hRealZttHLTPtNum","HLT vs Offline Real Tau; Offline Tau p_{T} [GeV];",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hRealZttL1PtEfficiency","L1 vs Offline Real Tau Efficiency; Offline Tau p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
            addHistogram(new TH1F("hRealZttHLTPtEfficiency","HLT vs Offline Real Tau Efficiency; Offline Tau p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
        }
    }
    
    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert",run));
    addHistogram(new TH1F("hL1TBPCounts","L1 Before Prescale counts; Chains;Nevents",m_trigItems.size(),0,m_trigItems.size()));
    addHistogram(new TH1F("hL1Counts","L1 counts; Chains;Nevents",m_trigItems.size(),0,m_trigItems.size()));
    addHistogram(new TH1F("hL1CountsDebug","L1 counts; Chains;Nevents",m_trigItems.size(),0,m_trigItems.size()));
    addHistogram(new TH1F("hHLTCounts","HLT counts; Chains;Nevents",m_trigItems.size(),0,m_trigItems.size()));
    addHistogram(new TH1F("hHLTCountsDebug","HLT counts; Chains;Nevents",m_trigItems.size(),0,m_trigItems.size()));
    for(unsigned int i=0;i<m_trigItems.size(); ++i){
        hist("hL1TBPCounts")->GetXaxis()->SetBinLabel(i+1,m_trigItems.at(i).c_str());
        hist("hL1Counts")->GetXaxis()->SetBinLabel(i+1,m_trigItems.at(i).c_str());
        hist("hL1CountsDebug")->GetXaxis()->SetBinLabel(i+1,m_trigItems.at(i).c_str());
        hist("hHLTCounts")->GetXaxis()->SetBinLabel(i+1,m_trigItems.at(i).c_str());
        hist("hHLTCountsDebug")->GetXaxis()->SetBinLabel(i+1,m_trigItems.at(i).c_str());
    }
    
    if(m_doTestTracking){
        addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/FTF_track_comparison",run));
        addHistogram(new TH1F("hFTFnTrack_1step","FTF number of tracks;number of tracks;Nevents",10,0,10));
        addHistogram(new TH1F("hFTFnTrack_2steps","FTF number of tracks;number of tracks;Nevents",10,0,10));
        addHistogram(new TH1F("hFTFnWideTrack_1step","FTF number of tracks;number of tracks;Nevents",10,0,10));
        addHistogram(new TH1F("hFTFnWideTrack_2steps","FTF number of tracks;number of tracks;Nevents",10,0,10));
    }
    
    std::vector<string> lowest_names;
    lowest_names.push_back("lowest_singletau");
    //    lowest_names.push_back("lowest_ditau");
    //    lowest_names.push_back("lowest_etau");
    //    lowest_names.push_back("lowest_mutau");
    //    lowest_names.push_back("lowest_mettau");
    //    lowest_names.push_back("cosmic_chain");
    
    for(unsigned int i=0;i<lowest_names.size();++i){
        
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/L1RoI",run));
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/L1VsOffline",run));
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/PreselectionTau",run));
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/PreselectionVsOffline",run));
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFTau",run));
        //      addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFTau/BDT/1p_nonCorrected",run));
        //      addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFTau/BDT/mp_nonCorrected",run));
        //      addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFTau/BDT/1p_Corrected",run));
        //      addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFTau/BDT/mp_Corrected",run));
        //      if(m_turnOnCurves) addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/TurnOnCurves/RecoEfficiency",run));
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFVsOffline",run));
        //      addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFVsOffline/BDT/1p_nonCorrected",run));
        //      addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/EFVsOffline/BDT/mp_nonCorrected",run));
        addMonGroup(new MonGroup(this,"HLT/TauMon/Shifter/"+lowest_names.at(i)+"/TurnOnCurves",run));
    }
    
    if(m_emulation){
        addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/Emulation",run));
        addHistogram(new TH1F("hL1EmulationPassTDT","; TDT passed events;",m_emulation_l1_tau.size(),-0.5,m_emulation_l1_tau.size()-0.5));
        addHistogram(new TH1F("hHLTEmulationPassTDT"," TDT passed events;",m_emulation_hlt_tau.size(),-0.5,m_emulation_hlt_tau.size()-0.5));
        addHistogram(new TH1F("hL1EmulationPassEmul"," Emualtion passed events;",m_emulation_l1_tau.size(),-0.5,m_emulation_l1_tau.size()-0.5));
        addHistogram(new TH1F("hHLTEmulationPassEmul"," Emulation passed events;",m_emulation_hlt_tau.size(),-0.5,m_emulation_hlt_tau.size()-0.5));
        addHistogram(new TH1F("hL1Emulation"," Mismatched events;",m_emulation_l1_tau.size(),-0.5,m_emulation_l1_tau.size()-0.5));
        addHistogram(new TH1F("hHLTEmulation"," Mismatched events;",m_emulation_hlt_tau.size(),-0.5,m_emulation_hlt_tau.size()-0.5));
        for(unsigned int i=0;i<m_emulation_l1_tau.size(); ++i){
            hist("hL1Emulation")->GetXaxis()->SetBinLabel(i+1,m_emulation_l1_tau.at(i).c_str());
            hist("hL1EmulationPassTDT")->GetXaxis()->SetBinLabel(i+1,m_emulation_l1_tau.at(i).c_str());
            hist("hL1EmulationPassEmul")->GetXaxis()->SetBinLabel(i+1,m_emulation_l1_tau.at(i).c_str());
        }
        for(unsigned int i=0;i<m_emulation_hlt_tau.size(); ++i){
            hist("hHLTEmulation")->GetXaxis()->SetBinLabel(i+1,m_emulation_hlt_tau.at(i).c_str());
            hist("hHLTEmulationPassTDT")->GetXaxis()->SetBinLabel(i+1,m_emulation_hlt_tau.at(i).c_str());
            hist("hHLTEmulationPassEmul")->GetXaxis()->SetBinLabel(i+1,m_emulation_hlt_tau.at(i).c_str());
        }
    }
    const int nbin_pt = 13;
    double bins_pt[nbin_pt] = {10.,20.,25.,30.,35.,40.,45.,50.,60.,70.,100.,150.,200.};
    const int nbin_eta = 9;
    double bins_eta[nbin_eta] = {-2.47,-1.52,-1.37,-0.69,0.,0.69,1.37,1.52,2.47};
    const int nbin_nvtx = 6;
    double bins_nvtx[nbin_nvtx] = {0.,5.,10.,15.,20.,25.};
    const int nbin_mu = 21;
    float bins_mu[nbin_mu] = {0.,2.,4.,6.,8.,10.,12.,14.,16.,18.,20.,22.,24.,26.,28.,30.,32.,34.,36.,38.,40.};

    addMonGroup(new MonGroup(this,"HLT/TauMon/Expert/HLTefficiency",run));
    addProfile(new TProfile("TProfRecoHLT25PtEfficiency", "idperf_tracktwo Vs perf_tracktwo;  p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addProfile(new TProfile("TProfRecoHLT25Pt1PEfficiency", "idperf_tracktwo Vs perf_tracktwo;  1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addProfile(new TProfile("TProfRecoHLT25Pt3PEfficiency", "idperf_tracktwo Vs perf_tracktwo; 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addProfile(new TProfile("TProfRecoHLT25EtaEfficiency", "idperf_tracktwo Vs perf_tracktwo;  #eta; Efficiency",nbin_eta-1,bins_eta));
    addProfile(new TProfile("TProfRecoHLT25PhiEfficiency", "idperf_tracktwo Vs perf_tracktwo;  #phi; Efficiency",16,-3.2,3.2));
    addProfile(new TProfile("TProfRecoHLT25NTrackEfficiency", "idperf_tracktwo Vs perf_tracktwo; Number of tracks; Efficiency",10,0,10));
    addProfile(new TProfile("TProfRecoHLT25NVtxEfficiency", "idperf_tracktwo Vs perf_tracktwo; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
    addProfile(new TProfile("TProfRecoHLT25MuEfficiency", "idperf_tracktwo Vs perf_tracktwo; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
    
    addProfile(new TProfile("TProfRecoHLT25PtEfficiency_2", "perf_tracktwo Vs medium1_tracktwo; p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addProfile(new TProfile("TProfRecoHLT25Pt1PEfficiency_2", "perf_tracktwo Vs medium1_tracktwo; 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addProfile(new TProfile("TProfRecoHLT25Pt3PEfficiency_2", "perf_tracktwo Vs medium1_tracktwo;  3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addProfile(new TProfile("TProfRecoHLT25EtaEfficiency_2", "perf_tracktwo Vs medium1_tracktwo;  #eta; Efficiency",nbin_eta-1,bins_eta));
    addProfile(new TProfile("TProfRecoHLT25PhiEfficiency_2", "perf_tracktwo Vs medium1_tracktwo;  #phi; Efficiency",16,-3.2,3.2));
    addProfile(new TProfile("TProfRecoHLT25NTrackEfficiency_2", "perf_tracktwo Vs medium1_tracktwo; Number of tracks; Efficiency",10,0,10));
    addProfile(new TProfile("TProfRecoHLT25NVtxEfficiency_2", "perf_tracktwo Vs medium1_tracktwo; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
    addProfile(new TProfile("TProfRecoHLT25MuEfficiency_2", "perf_tracktwo Vs medium1_tracktwo; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
    
    
    addHistogram(new TH1F("hRecoHLT25PtNum","idperf_tracktwo Vs perf_tracktwo; p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt1PNum","idperf_tracktwo Vs perf_tracktwo;  1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt3PNum","idperf_tracktwo Vs perf_tracktwo;  3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25EtaNum","idperf_tracktwo Vs perf_tracktwo; #eta;",nbin_eta-1,bins_eta));
    addHistogram(new TH1F("hRecoHLT25PhiNum","idperf_tracktwo Vs perf_tracktwo; #phi;",16,-3.2,3.2));
    addHistogram(new TH1F("hRecoHLT25NTrackNum","idperf_tracktwo Vs perf_tracktwo; Number of tracks;",10,0,10));
    addHistogram(new TH1F("hRecoHLT25NVtxNum","idperf_tracktwo Vs perf_tracktwo; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
    addHistogram(new TH1F("hRecoHLT25MuNum","idperf_tracktwo Vs perf_tracktwo; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addHistogram(new TH2F("hRecoHLT25EtaVsPhiNum","idperf_tracktwo Vs perf_tracktwo; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
    addHistogram(new TH1F("hRecoTau25PtDenom","; p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoTau25Pt1PDenom","; 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoTau25Pt3PDenom","; 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoTau25EtaDenom","; #eta;",nbin_eta-1,bins_eta));
    addHistogram(new TH1F("hRecoTau25PhiDenom","; #phi;",16,-3.2,3.2));
    addHistogram(new TH1F("hRecoTau25NTrackDenom","; Number of tracks;",10,0,10));
    addHistogram(new TH1F("hRecoTau25NVtxDenom","; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
    addHistogram(new TH1F("hRecoTau25MuDenom","; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addHistogram(new TH2F("hRecoTau25EtaVsPhiDenom","; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
    addHistogram(new TH1F("hRecoHLT25PtEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency;  p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt1PEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency; 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt3PEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency; 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25EtaEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency;  #eta; Efficiency",nbin_eta-1,bins_eta));
    addHistogram(new TH1F("hRecoHLT25PhiEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency;  #phi; Efficiency",16,-3.2,3.2));
    addHistogram(new TH1F("hRecoHLT25NTrackEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency; Number of tracks; Efficiency",10,0,10));
    addHistogram(new TH1F("hRecoHLT25NVtxEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
    addHistogram(new TH1F("hRecoHLT25MuEfficiency","idperf_tracktwo Vs perf_tracktwo Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
    addHistogram(new TH2F("hRecoHLT25EtaVsPhiEfficiency","idperf_tracktwo Vs perf_tracktwo in  Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));




    addHistogram(new TH1F("hRecoHLT25PtNum_2","perf_tracktwo Vs medium1_tracktwo; p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt1PNum_2","perf_tracktwo Vs medium1_tracktwo;  1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt3PNum_2","perf_tracktwo Vs medium1_tracktwo;  3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25EtaNum_2","perf_tracktwo Vs medium1_tracktwo; #eta;",nbin_eta-1,bins_eta));
    addHistogram(new TH1F("hRecoHLT25PhiNum_2","perf_tracktwo Vs medium1_tracktwo; #phi;",16,-3.2,3.2));
    addHistogram(new TH1F("hRecoHLT25NTrackNum_2","perf_tracktwo Vs medium1_tracktwo; Number of tracks;",10,0,10));
    addHistogram(new TH1F("hRecoHLT25NVtxNum_2","perf_tracktwo Vs medium1_tracktwo; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
    addHistogram(new TH1F("hRecoHLT25MuNum_2","perf_tracktwo Vs medium1_tracktwo; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addHistogram(new TH2F("hRecoHLT25EtaVsPhiNum_2","perf_tracktwo Vs medium1_tracktwo; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
    addHistogram(new TH1F("hRecoTau25PtDenom_2","; p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoTau25Pt1PDenom_2","; 1 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoTau25Pt3PDenom_2","; 3 prong p_{T} [GeV];",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoTau25EtaDenom_2","; #eta;",nbin_eta-1,bins_eta));
    addHistogram(new TH1F("hRecoTau25PhiDenom_2","; #phi;",16,-3.2,3.2));
    addHistogram(new TH1F("hRecoTau25NTrackDenom_2","; Number of tracks;",10,0,10));
    addHistogram(new TH1F("hRecoTau25NVtxDenom_2","; Number of primary vertices;",nbin_nvtx-1,bins_nvtx));
    addHistogram(new TH1F("hRecoTau25MuDenom_2","; Average interactions per bunch crossing;",nbin_mu-1,bins_mu));
    addHistogram(new TH2F("hRecoTau25EtaVsPhiDenom_2","; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));
    addHistogram(new TH1F("hRecoHLT25PtEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency;  p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt1PEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency; 1 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25Pt3PEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency; 3 prong p_{T} [GeV]; Efficiency",nbin_pt-1,bins_pt));
    addHistogram(new TH1F("hRecoHLT25EtaEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency;  #eta; Efficiency",nbin_eta-1,bins_eta));
    addHistogram(new TH1F("hRecoHLT25PhiEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency;  #phi; Efficiency",16,-3.2,3.2));
    addHistogram(new TH1F("hRecoHLT25NTrackEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency; Number of tracks; Efficiency",10,0,10));
    addHistogram(new TH1F("hRecoHLT25NVtxEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency; Number of primary vertices; Efficiency",nbin_nvtx-1,bins_nvtx));
    addHistogram(new TH1F("hRecoHLT25MuEfficiency_2","perf_tracktwo Vs medium1_tracktwo Efficiency; Average interactions per bunch crossing; Efficiency",nbin_mu-1,bins_mu));
    addHistogram(new TH2F("hRecoHLT25EtaVsPhiEfficiency_2","perf_tracktwo Vs medium1_tracktwo in  Eta-Phi; #eta; #phi",nbin_eta-1,bins_eta,16,-3.2,3.2));

}
