/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Author: Vadim Kostyukhin (vadim.kostyukhin@cern.ch)

// Header include
#include "InDetVKalVxInJetTool/InDetVKalVxInJetTool.h"
#include "InDetVKalVxInJetTool/InDetMaterialVeto.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "VxSecVertex/VxSecVKalVertexInfo.h"
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TProfile.h"

#include "TMath.h"
//

namespace InDet {

const int InDetVKalVxInJetTool::DevTuple::maxNTrk;

//
//Constructor-------------------------------------------------------------- 
InDetVKalVxInJetTool::InDetVKalVxInJetTool(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_cutSctHits(4),
    m_cutPixelHits(1),
    m_cutSiHits(7),
    m_cutBLayHits(0),
    m_cutSharedHits(1000), // Dummy configurable cut
    m_cutPt(700.),
    m_cutZVrt(15.),
    m_cutA0(5.),
    m_cutChi2(5.),
    m_secTrkChi2Cut(10.),
    m_coneForTag(0.4),
    m_sel2VrtChi2Cut(10.0),
    m_sel2VrtSigCut(4.0),
    m_trkSigCut(2.0),
    m_a0TrkErrorCut(1.0),
    m_zTrkErrorCut(5.0),
    m_cutBVrtScore(0.015),
    m_vrt2TrMassLimit(4000.),
    m_useFrozenVersion(true),
    m_fillHist(false),
    m_existIBL(true),
    m_RobustFit(1),
    m_beampipeR (0.),  //Correct values are filled     
    m_rLayerB   (0.),  // in initialize()
    m_rLayer1   (0.),
    m_rLayer2   (0.),
    m_rLayer3   (0.),
    m_useVertexCleaningPix(false),
    m_useVertexCleaningFMP(false),
    m_rejectBadVertices(false),
    m_multiVertex(false),
    m_multiWithPrimary(false),
    m_getNegativeTail(false),
    m_getNegativeTag(false),
    m_multiWithOneTrkVrt(true),
    m_vertexMergeCut(3.),
    m_trackDetachCut(6.),
    m_fitterSvc("Trk::TrkVKalVrtFitter/VertexFitterTool",this),
    m_trackClassificator("InDet::InDetTrkInJetType",this),
    m_useITkMaterialRejection(false),
    m_beamPipeMgr(nullptr),
    m_pixelManager(nullptr)
   {
//
// Declare additional interface
//
    declareInterface< ISecVertexInJetFinder >(this);
// Properties
//
//
    declareProperty("CutSctHits",    m_cutSctHits ,  "Remove track is it has less SCT hits" );
    declareProperty("CutPixelHits",  m_cutPixelHits, "Remove track is it has less Pixel hits");
    declareProperty("CutSiHits",     m_cutSiHits,    "Remove track is it has less Pixel+SCT hits"  );
    declareProperty("CutBLayHits",   m_cutBLayHits,  "Remove track is it has less B-layer hits"   );
    declareProperty("CutSharedHits", m_cutSharedHits,"Reject final 2tr vertices if tracks have shared hits" );

    declareProperty("CutPt",         m_cutPt,     "Track Pt selection cut"  );
    declareProperty("CutA0",         m_cutA0,     "Track A0 selection cut"  );
    declareProperty("CutZVrt",       m_cutZVrt,   "Track Z impact selection cut");
    declareProperty("ConeForTag",    m_coneForTag,"Cone around jet direction for track selection");
    declareProperty("CutChi2",       m_cutChi2,   "Track Chi2 selection cut" );
    declareProperty("TrkSigCut",     m_trkSigCut, "Track 3D impact significance w/r primary vertex" );
    declareProperty("SecTrkChi2Cut", m_secTrkChi2Cut,"Track - common secondary vertex association cut. Single Vertex Finder only");

    declareProperty("A0TrkErrorCut",  m_a0TrkErrorCut, "Track A0 error cut" );
    declareProperty("ZTrkErrorCut",   m_zTrkErrorCut,  "Track Z impact error cut" );
    declareProperty("CutBVrtScore",   m_cutBVrtScore,  "B vertex selection cut on 2track vertex score (probability-like) based on track classification" );
    declareProperty("Vrt2TrMassLimit",m_vrt2TrMassLimit,  "Maximal allowed mass for 2-track vertices" );

    declareProperty("Sel2VrtChi2Cut",    m_sel2VrtChi2Cut, "Cut on Chi2 of 2-track vertex for initial selection"  );
    declareProperty("Sel2VrtSigCut",     m_sel2VrtSigCut,  "Cut on significance of 3D distance between initial 2-track vertex and PV"  );

    declareProperty("UseFrozenVersion",   m_useFrozenVersion," Switch from default frozen version to the development/improved one" );
    declareProperty("FillHist",   m_fillHist, "Fill technical histograms"  );
    declareProperty("ExistIBL",   m_existIBL, "Inform whether 3-layer or 4-layer detector is used "  );

    declareProperty("RobustFit",  m_RobustFit, "Use vertex fit with RobustFit functional(VKalVrt) for common secondary vertex fit" );

    declareProperty("useVertexCleaningPix", m_useVertexCleaningPix, "Clean vertices requiring track pixel hit patterns according to vertex position" );
    declareProperty("useVertexCleaningFMP", m_useVertexCleaningFMP, "Clean vertices requiring track F(irst) M(easured) P(oints) matching to vertex position" );
    declareProperty("rejectBadVertices", m_rejectBadVertices, "Reject V0s after checking 3D PV impact" );
    

    declareProperty("MultiVertex",        m_multiVertex,       "Run Multiple Secondary Vertices in jet finder"  );
    declareProperty("MultiWithPrimary",   m_multiWithPrimary,  "Find Multiple Secondary Vertices + primary vertex in jet. MultiVertex Finder only!"  );
    declareProperty("MultiWithOneTrkVrt", m_multiWithOneTrkVrt,"Allow one-track-vertex addition to already found secondary vertices. MultiVertex Finder only! ");
    declareProperty("getNegativeTail", m_getNegativeTail, "Allow secondary vertex behind the primary one (negative) w/r jet direction (not for multivertex!)" );
    declareProperty("getNegativeTag",  m_getNegativeTag,  "Return ONLY negative secondary vertices (not for multivertex!)"   );

    declareProperty("VertexMergeCut",	  m_vertexMergeCut, "To allow vertex merging for MultiVertex Finder" );
    declareProperty("TrackDetachCut",	  m_trackDetachCut, "To allow track from vertex detachment for MultiVertex Finder" );

    declareProperty("VertexFitterTool",  m_fitterSvc);
    declareProperty("TrackClassTool",  m_trackClassificator);

    declareProperty("useITkMaterialRejection", m_useITkMaterialRejection, "Reject vertices from hadronic interactions in detector material using ITk layout");

    m_instanceName=name;

   }

//Destructor---------------------------------------------------------------
    InDetVKalVxInJetTool::~InDetVKalVxInJetTool(){
     ATH_MSG_DEBUG("InDetVKalVxInJetTool destructor called");
   }

//Initialize---------------------------------------------------------------
   StatusCode InDetVKalVxInJetTool::initialize(){
     ATH_MSG_DEBUG("InDetVKalVxInJetTool initialize() called");
 
     m_useTrackClassificator = !(m_trackClassificator.empty());
     ATH_CHECK(m_trackClassificator.retrieve( DisableTool{!m_useTrackClassificator} ));
     if(!m_useTrackClassificator) ATH_MSG_INFO("TrackClassificator disabled");

     m_useEtaDependentCuts = !(m_etaDependentCutsSvc.name().empty());
     if (m_useEtaDependentCuts){
       ATH_CHECK(m_etaDependentCutsSvc.retrieve());
       ATH_MSG_INFO("Using InDetEtaDependentCutsSvc. Individual inclusive track selections from config not used");
     }
     else{
       ATH_MSG_INFO("Using individual inclusive track selections from config");
     }

     if (m_fitterSvc.retrieve().isFailure()) {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Could not find Trk::TrkVKalVrtFitter" << endmsg;
        return StatusCode::SUCCESS;
     } else {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "InDetVKalVxInJetTool TrkVKalVrtFitter found" << endmsg;
     }
     m_fitSvc = dynamic_cast<Trk::TrkVKalVrtFitter*>(&(*m_fitterSvc));
     if(!m_fitSvc){
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<<" No implemented Trk::ITrkVKalVrtFitter interface" << endmsg;
        return StatusCode::SUCCESS;
     }

     ATH_CHECK( m_eventInfoKey.initialize() );
     //------------------------------------------
     if(msgLvl(MSG::DEBUG)) ATH_CHECK(service("ChronoStatSvc", m_timingProfile));
//------------------------------------------
// Chose whether IBL is installed
     if(m_existIBL){ // 4-layer pixel detector
       if( m_beampipeR==0.)  m_beampipeR=24.0;    
       if( m_rLayerB  ==0.)  m_rLayerB  =34.0;
       if( m_rLayer1  ==0.)  m_rLayer1  =51.6;
       if( m_rLayer2  ==0.)  m_rLayer2  =90.0;
       m_rLayer3  =122.5;
     } else {   // 3-layer pixel detector
       if( m_beampipeR==0.)  m_beampipeR=29.4;    
       if( m_rLayerB  ==0.)  m_rLayerB  =51.5;
       if( m_rLayer1  ==0.)  m_rLayer1  =90.0;
       if( m_rLayer2  ==0.)  m_rLayer2  =122.5;
     }       

     if(m_fillHist){
       if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
         ATH_MSG_FATAL("Filling histograms not supported in MT jobs.");
         return StatusCode::FAILURE;
       }

       ITHistSvc*     hist_root=nullptr;
       ATH_CHECK( service( "THistSvc", hist_root) );
       ATH_MSG_DEBUG( "InDetVKalVxInJetTool Histograms found" );

       std::string histDir;
       if(m_multiVertex) histDir="/file1/stat/MSVrtInJet"+m_instanceName+"/";
       else              histDir="/file1/stat/SVrtInJet"+m_instanceName+"/";
       m_h = std::make_unique<Hists>();
       ATH_CHECK( m_h->book (*hist_root, histDir) );

//-------------------------------------------------------
     }

     if(!m_multiVertex)m_multiWithPrimary = false; 

     if(m_getNegativeTag){
        if(msgLvl(MSG::INFO))msg(MSG::INFO) << " Negative TAG is requested! " << endmsg;
        if(msgLvl(MSG::INFO))msg(MSG::INFO) << "Not compatible with negativeTAIL option, so getNegativeTail is set to FALSE." << endmsg;
        m_getNegativeTail=false;
     }

     for(int ntv=2; ntv<=10; ntv++) m_chiScale[ntv]=TMath::ChisquareQuantile(0.9,2.*ntv-3.)/ntv;
     m_chiScale[0]=m_chiScale[2];
     for(int ntv=2; ntv<=10; ntv++) m_chiScale[ntv]/=m_chiScale[0];

     if(m_RobustFit>7)m_RobustFit=7;
     if(m_RobustFit<0)m_RobustFit=0;

     if(m_useITkMaterialRejection){

       ATH_CHECK(detStore()->retrieve(m_beamPipeMgr, "BeamPipe"));
       ATH_CHECK(detStore()->retrieve(m_pixelManager, "ITkPixel"));

       InDetMaterialVeto matVeto(m_beamPipeMgr, m_pixelManager);

       m_ITkPixMaterialMap = matVeto.ITkPixMaterialMap();

     }

     return StatusCode::SUCCESS;

   }

   StatusCode InDetVKalVxInJetTool::Hists::book (ITHistSvc& histSvc,
                                                 const std::string& histDir)
   {
     m_hb_massPiPi   = new TH1D("massPiPi"," mass PiPi",160,200., 1000.);
     m_hb_massPiPi1  = new TH1D("massPiPi1"," mass PiPi",100,200., 2000.);
     m_hb_massPPi    = new TH1D("massPPi"," massPPi", 100,1000., 1250.);
     m_hb_massEE     = new TH1D("massEE"," massEE", 100,0., 200.);
     m_hb_nvrt2      = new TH1D("nvrt2"," vertices2", 50,0., 50.);
     m_hb_ratio      = new TH1D("ratio"," ratio", 51,0., 1.02);
     m_hb_totmass    = new TH1D("totmass"," totmass", 250,0., 10000.);
     m_hb_totmassEE  = new TH1D("massEEcomvrt"," totmass EE common vertex", 100,0., 1000.);
     m_hb_totmass2T0 = new TH1D("mass2trcomvrt0"," totmass 2tr common vertex", 800,0., 4000.);
     m_hb_totmass2T1 = new TH1D("mass2trcomvrt1"," totmass 2tr common vertex", 200,0., 10000.);
     m_hb_totmass2T2 = new TH1D("mass2trcomvrt2"," totmass 2tr common vertex", 200,0., 10000.);
     m_hb_impact     = new TH1D("impact", " impact", 100,0., 20.);
     m_hb_impactR    = new TH1D("impactR"," impactR", 400,-30., 70.);
     m_hb_impactZ    = new TH1D("impactZ"," impactZ", 100,-30., 70.);
     m_hb_impactRZ   = new TH2D("impactRZ"," impactRZ", 40,-10., 10., 60, -30.,30. );
     m_hb_trkD0      = new TH1D("trkD0"," d0 of tracks", 100,-20., 20.);
     m_hb_r2d        = new TH1D("r2interact","Interaction radius 2tr selected", 150,0., 150.);
     m_hb_r1dc       = new TH1D("r1interactCommon","Interaction 1tr radius common", 150,0., 150.);
     m_hb_r2dc       = new TH1D("r2interactCommon","Interaction 2tr radius common", 150,0., 150.);
     m_hb_r3dc       = new TH1D("r3interactCommon","Interaction 3tr radius common", 150,0., 150.);
     m_hb_rNdc       = new TH1D("rNinteractCommon","Interaction Ntr radius common", 150,0., 150.);
     m_hb_dstToMat   = new TH1D("dstToMatL","Distance to material layer", 80,0., 40.);
     m_hb_impV0      = new TH1D("impactV0"," V0 impact", 100,0., 50.);
     m_hb_ntrkjet    = new TH1D("ntrkjet"," NTrk in jet", 50,0., 50.);
     m_hb_jmom       = new TH1D("jetmom"," Jet mom", 200,0., 2000000.);
     m_hb_mom        = new TH1D("jetmomvrt"," Jet mom with sec. vertex", 200,0., 2000000.);
     m_hb_signif3D   = new TH1D("signif3D"," Signif3D for initial 2tr vertex", 140,-20., 50.);
     m_hb_sig3DTot   = new TH1D("sig3dcommon"," Signif3D for common vertex", 140,-20., 50.);
     m_hb_sig3D1tr   = new TH1D("sig3D1tr","Signif3D for 1tr  vertices", 140,-20., 50.);
     m_hb_sig3D2tr   = new TH1D("sig3D2tr","Signif3D for 2tr single vertex", 140,-20., 50.);
     m_hb_sig3DNtr   = new TH1D("sig3DNtr","Signif3D for many-tr single vertex", 140,-20., 50.);
     m_hb_goodvrtN   = new TH1F("goodvrtN","Number of good vertices", 20,0., 20.);
     m_hb_distVV     = new TH1D("distvv","Vertex-Vertex dist", 100,0., 20.);
     m_hb_diffPS     = new TH1D("diffPS","Primary-Secondary assoc", 200,-20., 20.);
     m_hb_trkPtMax   = new TH1D("trkPtMax","Maximal track Pt to jet", 100, 0., 5000.);
     m_hb_rawVrtN    = new TH1F("rawVrtN","Number of raw vertices multivertex case", 20, 0., 20.);
     m_hb_lifetime   = new TH1F("lifetime","Distance/momentum", 100, 0., 5.);
     m_hb_trkPErr    = new TH1F("trkPErr","Track momentum error for P>10 GeV", 100, 0., 0.5);
     m_hb_deltaRSVPV = new TH1F("deltaRSVPV","SV-PV vs jet dR ", 200, 0., 1.);
     m_pr_NSelTrkMean = new TProfile("NSelTrkMean"," NTracks selected vs jet pt", 80, 0., 1600000.);
     m_pr_effVrt2tr   = new TProfile("effVrt2tr"," 2tr vertex efficiency vs Ntrack", 50, 0., 50.);
     m_pr_effVrt2trEta= new TProfile("effVrt2trEta"," 2tr vertex efficiency vs eta", 50, -3., 3.);
     m_pr_effVrt      = new TProfile("effVrt","Full vertex efficiency vs Ntrack", 50, 0., 50.);
     m_pr_effVrtEta   = new TProfile("effVrtEta","Full vertex efficiency vs eta", 50, -3., 3.);

     ATH_CHECK( histSvc.regHist(histDir+"massPiPi", m_hb_massPiPi) );
     ATH_CHECK( histSvc.regHist(histDir+"massPiPi1", m_hb_massPiPi1) );
     ATH_CHECK( histSvc.regHist(histDir+"massPPi", m_hb_massPPi) );
     ATH_CHECK( histSvc.regHist(histDir+"massEE", m_hb_massEE ) );
     ATH_CHECK( histSvc.regHist(histDir+"nvrt2", m_hb_nvrt2) );
     ATH_CHECK( histSvc.regHist(histDir+"ratio", m_hb_ratio) );
     ATH_CHECK( histSvc.regHist(histDir+"totmass", m_hb_totmass) );
     ATH_CHECK( histSvc.regHist(histDir+"massEEcomvrt", m_hb_totmassEE) );
     ATH_CHECK( histSvc.regHist(histDir+"mass2trcomvrt0", m_hb_totmass2T0) );
     ATH_CHECK( histSvc.regHist(histDir+"mass2trcomvrt1", m_hb_totmass2T1) );
     ATH_CHECK( histSvc.regHist(histDir+"mass2trcomvrt2", m_hb_totmass2T2) );
     ATH_CHECK( histSvc.regHist(histDir+"impact",    m_hb_impact) );
     ATH_CHECK( histSvc.regHist(histDir+"impactR",   m_hb_impactR) );
     ATH_CHECK( histSvc.regHist(histDir+"impactZ",   m_hb_impactZ) );
     ATH_CHECK( histSvc.regHist(histDir+"impactRZ",  m_hb_impactRZ) );
     ATH_CHECK( histSvc.regHist(histDir+"trkD0",     m_hb_trkD0) );
     ATH_CHECK( histSvc.regHist(histDir+"r2interact",       m_hb_r2d) );
     ATH_CHECK( histSvc.regHist(histDir+"r1interactCommon", m_hb_r1dc) );
     ATH_CHECK( histSvc.regHist(histDir+"r2interactCommon", m_hb_r2dc) );
     ATH_CHECK( histSvc.regHist(histDir+"r3interactCommon", m_hb_r3dc) );
     ATH_CHECK( histSvc.regHist(histDir+"rNinteractCommon", m_hb_rNdc) );
     ATH_CHECK( histSvc.regHist(histDir+"dstToMatL", m_hb_dstToMat) );
     ATH_CHECK( histSvc.regHist(histDir+"impactV0",  m_hb_impV0) );
     ATH_CHECK( histSvc.regHist(histDir+"ntrkjet",   m_hb_ntrkjet) );
     ATH_CHECK( histSvc.regHist(histDir+"jetmom",    m_hb_jmom) );
     ATH_CHECK( histSvc.regHist(histDir+"jetmomvrt", m_hb_mom) );
     ATH_CHECK( histSvc.regHist(histDir+"signif3D",  m_hb_signif3D) );
     ATH_CHECK( histSvc.regHist(histDir+"sig3dcommon", m_hb_sig3DTot) );
     ATH_CHECK( histSvc.regHist(histDir+"sig3D1tr",  m_hb_sig3D1tr) );
     ATH_CHECK( histSvc.regHist(histDir+"sig3D2tr",  m_hb_sig3D2tr) );
     ATH_CHECK( histSvc.regHist(histDir+"sig3DNtr",  m_hb_sig3DNtr) );
     ATH_CHECK( histSvc.regHist(histDir+"goodvrtN",  m_hb_goodvrtN) );
     ATH_CHECK( histSvc.regHist(histDir+"distVV",    m_hb_distVV) );
     ATH_CHECK( histSvc.regHist(histDir+"diffPS",    m_hb_diffPS) );
     ATH_CHECK( histSvc.regHist(histDir+"trkPtMax",  m_hb_trkPtMax) );
     ATH_CHECK( histSvc.regHist(histDir+"rawVrtN",   m_hb_rawVrtN) );
     ATH_CHECK( histSvc.regHist(histDir+"lifetime",  m_hb_lifetime) );
     ATH_CHECK( histSvc.regHist(histDir+"trkPErr",   m_hb_trkPErr) );
     ATH_CHECK( histSvc.regHist(histDir+"deltaRSVPV",   m_hb_deltaRSVPV) );
     ATH_CHECK( histSvc.regHist(histDir+"NSelTrkMean",  m_pr_NSelTrkMean) );
     ATH_CHECK( histSvc.regHist(histDir+"effVrt2tr",    m_pr_effVrt2tr) );
     ATH_CHECK( histSvc.regHist(histDir+"effVrt2trEta", m_pr_effVrt2trEta) );
     ATH_CHECK( histSvc.regHist(histDir+"effVrt",       m_pr_effVrt) );
     ATH_CHECK( histSvc.regHist(histDir+"effVrtEta",    m_pr_effVrtEta) );

     m_tuple = new TTree("Tracks","Tracks");
     ATH_CHECK( histSvc.regTree(histDir,m_tuple) );
     m_curTup = new DevTuple();

     m_tuple->Branch("ewgt",        &m_curTup->ewgt,      "ewgt/F");
     m_tuple->Branch("ptjet",       &m_curTup->ptjet,     "ptjet/F");
     m_tuple->Branch("etajet",      &m_curTup->etajet,    "etajet/F");
     m_tuple->Branch("phijet",      &m_curTup->phijet,    "phijet/F");
     m_tuple->Branch("ntrk",        &m_curTup->nTrkInJet, "ntrk/I");
     m_tuple->Branch("etatrk",      &m_curTup->etatrk,    "etatrk[ntrk]/F");
     m_tuple->Branch("prbS",        &m_curTup->s_prob,    "prbS[ntrk]/F");
     m_tuple->Branch("prbP",        &m_curTup->p_prob,    "prbP[ntrk]/F");
     m_tuple->Branch("wgtB",        &m_curTup->wgtB,      "wgtB[ntrk]/F");
     m_tuple->Branch("wgtL",        &m_curTup->wgtL,      "wgtL[ntrk]/F");
     m_tuple->Branch("wgtG",        &m_curTup->wgtG,      "wgtG[ntrk]/F");
     m_tuple->Branch("sig3D",       &m_curTup->sig3D,     "sig3D[ntrk]/F");
     m_tuple->Branch("idMC",        &m_curTup->idMC,      "idMC[ntrk]/I");
     m_tuple->Branch("ibl",         &m_curTup->ibl,       "ibl[ntrk]/I");
     m_tuple->Branch("bl",          &m_curTup->bl,        "bl[ntrk]/I");
     m_tuple->Branch("fhitR",       &m_curTup->fhitR,     "fhitRR[ntrk]/F");
     m_tuple->Branch("SigR",        &m_curTup->SigR,      "SigR[ntrk]/F");
     m_tuple->Branch("SigZ",        &m_curTup->SigZ,      "SigZ[ntrk]/F");
     m_tuple->Branch("d0",          &m_curTup->d0,        "d0[ntrk]/F");
     m_tuple->Branch("Z0",          &m_curTup->Z0,        "Z0[ntrk]/F");
     m_tuple->Branch("pTvsJet",     &m_curTup->pTvsJet,   "pTvsJet[ntrk]/F");
     m_tuple->Branch("prodTJ",      &m_curTup->prodTJ,    "prodTJ[ntrk]/F");
     m_tuple->Branch("nVrtT",       &m_curTup->nVrtT,     "nVrtT[ntrk]/I");
     m_tuple->Branch("chg",         &m_curTup->chg,       "chg[ntrk]/I");
     //-----
     m_tuple->Branch("TotM",        &m_curTup->TotM,      "TotM/F");
     //-----
     m_tuple->Branch("nvrt",        &m_curTup->nVrt,      "nvrt/I");
     m_tuple->Branch("VrtDist2D",   &m_curTup->VrtDist2D, "VrtDist2D[nvrt]/F");
     m_tuple->Branch("VrtSig3D",    &m_curTup->VrtSig3D,  "VrtSig3D[nvrt]/F");
     m_tuple->Branch("VrtSig2D",    &m_curTup->VrtSig2D,  "VrtSig2D[nvrt]/F");
     m_tuple->Branch("VrtDR",       &m_curTup->VrtDR,     "VrtDR[nvrt]/F");
     m_tuple->Branch("VrtdRtt",     &m_curTup->VrtdRtt,   "VrtdRtt[nvrt]/F");
     m_tuple->Branch("VrtErrR",     &m_curTup->VrtErrR,   "VrtErrR[nvrt]/F");
     m_tuple->Branch("itrk",        &m_curTup->itrk,      "itrk[nvrt]/I");
     m_tuple->Branch("jtrk",        &m_curTup->jtrk,      "jtrk[nvrt]/I");
     m_tuple->Branch("badV",        &m_curTup->badVrt,    "badV[nvrt]/I");
     m_tuple->Branch("mass",        &m_curTup->mass,      "mass[nvrt]/F");
     m_tuple->Branch("Chi2",        &m_curTup->Chi2,      "Chi2[nvrt]/F");
     //-----
     m_tuple->Branch("ntHF",        &m_curTup->NTHF,      "ntHF/I");
     m_tuple->Branch("itHF",        &m_curTup->itHF,      "itHF[ntHF]/I");
     //-----
     m_tuple->Branch("nNVrt",       &m_curTup->nNVrt,      "nNVrt/I");
     m_tuple->Branch("NVrtDist2D",  &m_curTup->NVrtDist2D, "NVrtDist2D[nNVrt]/F");
     m_tuple->Branch("NVrtSig3D",   &m_curTup->NVrtSig3D,  "NVrtSig3D[nNVrt]/F");
     m_tuple->Branch("NVrtNT",      &m_curTup->NVrtNT,     "NVrtNT[nNVrt]/I");
     m_tuple->Branch("NVrtTrkI",    &m_curTup->NVrtTrkI,   "NVrttrkI[nNVrt]/I");
     m_tuple->Branch("NVrtM",       &m_curTup->NVrtM,      "NVrtM[nNVrt]/F");
     m_tuple->Branch("NVrtChi2",    &m_curTup->NVrtChi2,   "NVrtChi2[nNVrt]/F");
     m_tuple->Branch("NVrtMaxW",    &m_curTup->NVrtMaxW,   "NVrtMaxW[nNVrt]/F");
     m_tuple->Branch("NVrtAveW",    &m_curTup->NVrtAveW,   "NVrtAveW[nNVrt]/F");
     m_tuple->Branch("NVrtDR",      &m_curTup->NVrtDR,     "NVrtDR[nNVrt]/F");

     return StatusCode::SUCCESS;
   }


  StatusCode InDetVKalVxInJetTool::finalize()
  {
    if(m_timingProfile)m_timingProfile->chronoPrint("InDetVKalVxInJetTool");
    ATH_MSG_DEBUG("InDetVKalVxInJetTool finalize()");
    return StatusCode::SUCCESS; 
  }
  



   Trk::VxSecVertexInfo* InDetVKalVxInJetTool::findSecVertex(const xAOD::Vertex & primVrt,
							           const TLorentzVector & jetDir,
						 	           const std::vector<const xAOD::IParticle*> & IInpTrk)
    const  {
    if(m_timingProfile)m_timingProfile->chronoStart("InDetVKalVxInJetTool");
    std::vector<double>     Results;
    std::vector<const xAOD::TrackParticle*>            InpTrk;
    std::vector<const xAOD::TrackParticle*>            SelSecTrk;
    std::vector< std::vector<const xAOD::TrackParticle*> >  SelSecTrkPerVrt;
    std::vector<const xAOD::TrackParticle*>        xaodTrkFromV0;
    std::vector<xAOD::Vertex*> listVrtSec(0);
    double SecVtxMass =   0.;
    double RatioE     =   0.;
    double EnergyJet  =   0.;
    int N2trVertices  =   0 ;
    int NBigImpTrk    =   0 ;

    if(m_fillHist){
      Hists& h = getHists();
      if (h.m_curTup) {
        h.m_curTup->nVrt=0;
        h.m_curTup->nTrkInJet=0;
        h.m_curTup->NTHF=0;
        h.m_curTup->nNVrt=0;
        h.m_curTup->TotM=0.; h.m_curTup->ewgt=1.;
      }
    }

    int pseudoVrt = 0;

    compatibilityGraph_t compatibilityGraph;

    InpTrk.clear(); InpTrk.reserve(IInpTrk.size());
    std::vector<const xAOD::IParticle*>::const_iterator   i_itrk;
    for (i_itrk = IInpTrk.begin(); i_itrk < IInpTrk.end(); ++i_itrk) {
          const xAOD::TrackParticle * tmp=dynamic_cast<const xAOD::TrackParticle *> ((*i_itrk));
          if(tmp)InpTrk.push_back(tmp);
    }

    if(m_multiVertex){
       std::unique_ptr<workVectorArrxAOD> tmpVectxAOD= std::make_unique<workVectorArrxAOD>();
       tmpVectxAOD->InpTrk.resize(InpTrk.size());
       std::copy(InpTrk.begin(),InpTrk.end(), tmpVectxAOD->InpTrk.begin());
       listVrtSec = getVrtSecMulti(tmpVectxAOD.get(),primVrt,jetDir,Results,compatibilityGraph);
       SelSecTrkPerVrt.swap(tmpVectxAOD->FoundSecondTracks);
       xaodTrkFromV0.swap(tmpVectxAOD->TrkFromV0);
    }else{
       int nRefPVTrk=0;
       xAOD::Vertex* secVrt = getVrtSec( InpTrk,primVrt,jetDir,Results,SelSecTrk,xaodTrkFromV0, nRefPVTrk, compatibilityGraph);
       if(secVrt != nullptr) listVrtSec.push_back(secVrt);
       else if(m_fillHist) {
         Hists& h = getHists();
         h.m_pr_effVrt->Fill((float)nRefPVTrk,0.);
         h.m_pr_effVrtEta->Fill( jetDir.Eta(),0.);
       }
    }
    if(Results.size()<7) {
       listVrtSec.clear();
    }else{
       SecVtxMass =      Results[0];
       RatioE     =      Results[1];
       N2trVertices  = (int)Results[2];
       NBigImpTrk    = (int)Results[3];
       EnergyJet     =      Results[6];
       if( Results[2]==0 && Results[4]==0 ) pseudoVrt=1;
    }

    std::vector<const xAOD::IParticle*>  iparTrkFromV0(0); 
    for(auto & i : xaodTrkFromV0)iparTrkFromV0.push_back(i);

    Trk::VxSecVKalVertexInfo* res=nullptr;
    try{
      if(pseudoVrt){
        res =  new Trk::VxSecVKalVertexInfo(listVrtSec[0], SecVtxMass, RatioE, NBigImpTrk, iparTrkFromV0 );
      }else{
        res =  new Trk::VxSecVKalVertexInfo(listVrtSec, SecVtxMass, RatioE, N2trVertices, EnergyJet, iparTrkFromV0 );
        if(Results.size()>8)res->setDstToMatLay(Results[7]);
    } }
    catch (std::bad_alloc& ba){
      ATH_MSG_DEBUG("Trk::VxSecVKalVertexInfo allocation failure! "<< ba.what());
    }

    if(m_fillHist){
      Hists& h = getHists();
      h.m_tuple->Fill();
    };
    if(m_timingProfile)m_timingProfile->chronoStop("InDetVKalVxInJetTool");
    return res;
   }


   InDetVKalVxInJetTool::Hists&
   InDetVKalVxInJetTool::getHists() const
   {
     // We earlier checked that no more than one thread is being used.
     Hists* h ATLAS_THREAD_SAFE = m_h.get();
     return *h;
   }


}  // end InDet namespace
