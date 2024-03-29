/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "InDetSVWithMuonTool/InDetSVWithMuonTool.h"
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "AthenaKernel/SlotSpecificObj.h" // for getNSlots
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ITHistSvc.h"
#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
//
//-------------------------------------------------
// Other stuff
//
#include<iostream>

namespace InDet {
//
//Constructor-------------------------------------------------------------- 
InDetSVWithMuonTool::InDetSVWithMuonTool(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_CutSctHits(4),
    m_CutPixelHits(1),
    m_CutSiHits(7),
    m_CutBLayHits(0),
    m_CutSharedHits(1),
    m_CutPt(700.),
    m_CutZVrt(25.),
    m_CutA0(5.),
    m_CutChi2(3.),
    m_SecTrkChi2Cut(10.),
    m_ConeForTag(0.4),
    m_Sel2VrtChi2Cut(4.5),
    m_Sel2VrtSigCut(3.0),
    m_TrkSigCut(2.5),
    m_A0TrkErrorCut(1.0),
    m_ZTrkErrorCut(5.0),
    m_FillHist(false),
    m_existIBL(true),
    m_RobustFit(5),
    m_Rbeampipe (0.),  //Correct values are filled     
    m_RlayerB   (0.),  // in initialize()
    m_Rlayer1   (0.),
    m_Rlayer2   (0.),
    m_Rlayer3   (0.),
    m_fitterSvc("Trk::TrkVKalVrtFitter/VertexFitterTool",this)
   {
    declareInterface<ISVWithMuonFinder>(this);
//
// Properties
//
//
    declareProperty("CutSctHits",    m_CutSctHits ,  "Remove track is it has less SCT hits" );
    declareProperty("CutPixelHits",  m_CutPixelHits, "Remove track is it has less Pixel hits");
    declareProperty("CutSiHits",     m_CutSiHits,    "Remove track is it has less Pixel+SCT hits"  );
    declareProperty("CutBLayHits",   m_CutBLayHits,  "Remove track is it has less B-layer hits"   );
    declareProperty("CutSharedHits", m_CutSharedHits,"Reject final 2tr vertices if tracks have shared hits" );

    declareProperty("CutPt",         m_CutPt,     "Track Pt selection cut"  );
    declareProperty("CutA0",         m_CutA0,     "Track A0 selection cut"  );
    declareProperty("CutZVrt",       m_CutZVrt,   "Track Z impact selection cut");
    declareProperty("ConeForTag",    m_ConeForTag,"Cone around jet direction for track selection");
    declareProperty("CutChi2",       m_CutChi2,   "Track Chi2 selection cut" );
    declareProperty("TrkSigCut",     m_TrkSigCut, "Track 3D impact significance w/r primary vertex" );
    declareProperty("SecTrkChi2Cut", m_SecTrkChi2Cut,"Track - common secondary vertex association cut. Single Vertex Finder only");

    declareProperty("A0TrkErrorCut",  m_A0TrkErrorCut, "Track A0 error cut" );
    declareProperty("ZTrkErrorCut",   m_ZTrkErrorCut,  "Track Z impact error cut" );

    declareProperty("Sel2VrtChi2Cut",    m_Sel2VrtChi2Cut, "Cut on Chi2 of 2-track vertex for initial selection"  );
    declareProperty("Sel2VrtSigCut",     m_Sel2VrtSigCut,  "Cut on significance of 3D distance between initial 2-track vertex and PV"  );

    declareProperty("FillHist",   m_FillHist, "Fill technical histograms"  );
    declareProperty("ExistIBL",   m_existIBL, "Inform whether 3-layer or 4-layer detector is used "  );

    declareProperty("RobustFit",  m_RobustFit, "Use vertex fit with RobustFit functional(VKalVrt) for common secondary vertex fit" );

    declareProperty("VertexFitterTool", m_fitterSvc);

   }

//Destructor---------------------------------------------------------------
    InDetSVWithMuonTool::~InDetSVWithMuonTool(){
     if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<< "InDetSVWithMuonTool destructor called" << endmsg;
   }

//Initialize---------------------------------------------------------------
   StatusCode InDetSVWithMuonTool::initialize(){
     if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<< "InDetSVWithMuonTool initialize() called" << endmsg;

     if (m_fitterSvc.retrieve().isFailure()) {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Could not find Trk::TrkVKalVrtFitter" << endmsg;
        return StatusCode::SUCCESS;
     } else {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "InDetSVWithMuonTool TrkVKalVrtFitter found" << endmsg;
     }
     m_fitSvc = dynamic_cast<Trk::TrkVKalVrtFitter*>(&(*m_fitterSvc));
     if(!m_fitSvc){
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<<" No implemented Trk::ITrkVKalVrtFitter interface" << endmsg;
        return StatusCode::SUCCESS;
     }

//------------------------------------------
// Chose whether IBL is installed
     if(m_existIBL){ // 4-layer pixel detector
       if( m_Rbeampipe==0.)  m_Rbeampipe=24.0;    
       if( m_RlayerB  ==0.)  m_RlayerB  =34.0;
       if( m_Rlayer1  ==0.)  m_Rlayer1  =51.6;
       if( m_Rlayer2  ==0.)  m_Rlayer2  =90.0;
       m_Rlayer3  =122.5;
     } else {   // 3-layer pixel detector
       if( m_Rbeampipe==0.)  m_Rbeampipe=29.4;    
       if( m_RlayerB  ==0.)  m_RlayerB  =51.5;
       if( m_Rlayer1  ==0.)  m_Rlayer1  =90.0;
       if( m_Rlayer2  ==0.)  m_Rlayer2  =122.5;
     }       
       
//
//
     if(m_FillHist){
       if (SG::getNSlots() > 1) {
         ATH_MSG_FATAL("Filling histograms not supported in MT jobs.");
         return StatusCode::FAILURE;
       }

       ServiceHandle<ITHistSvc> histSvc ("THistSvc", name());
       ATH_CHECK( histSvc.retrieve() );
       m_h = std::make_unique<Hists>();
       ATH_CHECK( m_h->book (*histSvc) );
     }


     return StatusCode::SUCCESS;

   }


   StatusCode InDetSVWithMuonTool::Hists::book (ITHistSvc& histSvc)
   {
     m_hb_massPiPi   = new TH1D("massPiPi"," massPiPi",160,200., 1000.);
     m_hb_muonPt     = new TH1D("muonPt",  " Pt of input muon",100,0., 20000.);
     m_hb_nvrt2      = new TH1D("nvrt2"," vertices2", 50,0., 50.);
     m_hb_nseltrk    = new TH1D("nseltrk"," Number of tracks cloase to muon", 50,0., 50.);
     m_hb_totmass    = new TH1D("totmass"," totmass", 250,0., 10000.);
     m_hb_impact     = new TH1D("impact", " impact", 100,0., 20.);
     m_hb_impactR    = new TH1D("impactR"," impactR", 100,-30., 70.);
     m_hb_impactZ    = new TH1D("impactZ"," impactZ", 100,-30., 70.);
     m_hb_r2d        = new TH1D("r2interact","Interaction radius 2tr", 150,0., 150.);
     m_hb_r2dc       = new TH1D("r2interactCommon","Interaction radius common", 150,0., 150.);
     m_hb_signif3D   = new TH1D("Signif3D","3D SV-PV significance for track+muon", 120,-5., 25.);
     std::string histDir="/file1/stat/MuonSV/";
#define H_CHECK(X) if((X).isFailure()) return StatusCode::FAILURE
     H_CHECK( histSvc.regHist(histDir+"muonPt",   m_hb_muonPt) );
     H_CHECK( histSvc.regHist(histDir+"massPiPi", m_hb_massPiPi) );
     H_CHECK( histSvc.regHist(histDir+"nvrt2",    m_hb_nvrt2) );
     H_CHECK( histSvc.regHist(histDir+"nseltrk",  m_hb_nseltrk) );
     H_CHECK( histSvc.regHist(histDir+"totmass",  m_hb_totmass) );
     H_CHECK( histSvc.regHist(histDir+"impact",   m_hb_impact) );
     H_CHECK( histSvc.regHist(histDir+"impactR",  m_hb_impactR) );
     H_CHECK( histSvc.regHist(histDir+"impactZ",  m_hb_impactZ) );
     H_CHECK( histSvc.regHist(histDir+"Signif3D", m_hb_signif3D) );
     H_CHECK( histSvc.regHist(histDir+"r2interact2tr",    m_hb_r2d) );
     H_CHECK( histSvc.regHist(histDir+"r2interactCommon", m_hb_r2dc) );
#undef H_CHECK
     return StatusCode::SUCCESS;
   }


  StatusCode InDetSVWithMuonTool::finalize()
  {
    if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) <<"InDetSVWithMuonTool finalize()" << endmsg;
    return StatusCode::SUCCESS; 
  }
  



   const xAOD::Vertex* InDetSVWithMuonTool::findSVwithMuon(const xAOD::Vertex & PrimVrt,
					               const xAOD::TrackParticle * Muon,
						       const std::vector<const xAOD::TrackParticle*> & InpTrk)
   const  {
    std::vector<const xAOD::TrackParticle*>            SelSecTrk;

    xAOD::Vertex* secVrt = MuonVrtSec( InpTrk, PrimVrt, Muon, SelSecTrk );

    return secVrt;
   }


   InDetSVWithMuonTool::Hists&
   InDetSVWithMuonTool::getHists() const
   {
     // We earlier checked that no more than one thread is being used.
     Hists* h ATLAS_THREAD_SAFE = m_h.get();
     return *h;
   }


}  // end InDet namespace
