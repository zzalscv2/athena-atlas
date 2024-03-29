/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKMONITORING_RECOMUONPLOTS_H
#define MUONTRACKMONITORING_RECOMUONPLOTS_H

#include "TrkValHistUtils/PlotBase.h" //base class
#include "TrkValHistUtils/ParamPlots.h" //member
#include "TrkValHistUtils/ImpactPlots.h" //member
#include "MuonHistUtils/RecoInfoPlots.h" //member
#include "MuonHistUtils/MomentumPullPlots.h" //member

#include "xAODMuon/Muon.h" //typedef xAOD::Muon
#include <cstdint> //for  uint8_t 
class TH2;

class RecoMuonPlots:public PlotBase {
    public:
      RecoMuonPlots(PlotBase* pParent, const std::string& sDir, bool detail=false);
      void fill(const xAOD::Muon& mu);
      void fill(const xAOD::Muon& mu, xAOD::Muon::Quality my_quality);
      
      // Reco only information
      Trk::ParamPlots                 m_oAllPlots;
      Muon::RecoInfoPlots             m_oMuRecoInfoPlots;
      Trk::ImpactPlots                m_oImpactPlots;
      Muon::MomentumPullPlots         m_oMomentumPullPlots;

      //extra monitoring efficiency plots; filled in post processing
      TH2* m_origin_eta_phi;
      TH2* m_eff_tight;
      TH2* m_eff_medium;
      TH2* m_tight_eta_phi;
      TH2* m_medium_eta_phi;
      TH1* m_pt_broad;
      TH2* m_eta_phi_broad;

      //extra monitoring efficiency plots, normalized in post processing
      TH2* m_eff_nPrec;
      TH2* m_eff_nPhi;
      TH2* m_eff_nTrigEta;
      TH2* m_eff_ndof;
      TH2* m_eff_chi2;
      TH2* m_ID_eff_ndof;
      TH2* m_ID_eff_chi2;
      TH2* m_MS_eff_ndof;
      TH2* m_MS_eff_chi2;

      TH2* m_avg_hits_precision_inner = nullptr;
      TH2* m_avg_hits_precision_middle = nullptr;
      TH2* m_avg_hits_precision_outer = nullptr;
      TH2* m_avg_hits_precision_extended = nullptr;

      TH2* m_avg_hits_trigger_layer1 = nullptr;
      TH2* m_avg_hits_trigger_layer2 = nullptr;
      TH2* m_avg_hits_trigger_layer3 = nullptr;
      TH2* m_avg_hits_trigger_layer4 = nullptr;

      TH2* m_avg_hits_ibl = nullptr;
      TH2* m_avg_hits_pix = nullptr;
      TH2* m_avg_hits_sct = nullptr;
      TH2* m_avg_hits_trt = nullptr;

      TH2* m_avg_ddpt_idme = nullptr;
      TH2* m_avg_dptsignif = nullptr;

      // overview layers
      uint8_t hitval_numberOfPrecisionLayers  = 0;
      uint8_t hitval_numberOfPhiLayers        = 0;
      uint8_t hitval_numberOfTriggerEtaLayers = 0;

      // precision hits
      uint8_t hitval_innerSmallHits    = 0;
      uint8_t hitval_innerLargeHits    = 0;
      uint8_t hitval_middleSmallHits   = 0;
      uint8_t hitval_middleLargeHits   = 0;
      uint8_t hitval_outerSmallHits    = 0;
      uint8_t hitval_outerLargeHits    = 0;
      uint8_t hitval_extendedSmallHits = 0;
      uint8_t hitval_extendedLargeHits = 0;

      // trigger hits
      uint8_t hitval_etaLayer1Hits = 0;
      uint8_t hitval_etaLayer2Hits = 0;
      uint8_t hitval_etaLayer3Hits = 0;
      uint8_t hitval_etaLayer4Hits = 0;

      // ID hits
      uint8_t hitval_numberOfBLayerHits = 0;
      uint8_t hitval_numberOfPixelHits  = 0;
      uint8_t hitval_numberOfSCTHits    = 0;
      uint8_t hitval_numberOfTRTHits    = 0;

      // momentum balance
      float ddpt_idme     = 0;
      float qoverp_diff   = 0;
      float qoverp_sigma  = 0;
      float qoverp_signif = 0;

      bool Detail;

    private:
      void initializePlots();
};

#endif
