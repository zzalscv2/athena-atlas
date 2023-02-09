/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAPHYSVALMONITORING_TRACKPLOTS_H
#define EGAMMAPHYSVALMONITORING_TRACKPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "EgammaPhysValHistUtilities.h"
#include "xAODEgamma/Electron.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

namespace Egamma{
  
class TrackPlots:public PlotBase {
    public:
      TrackPlots(PlotBase* pParent, const std::string& sDir, std::string sParticleType);
      void fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo);
     
      std::string m_sParticleType;
        
      TH1* deta;
      TH1* dphi;
      TH1* d0;
      TH1* z0;
      TH1* d0significance;
      TH1* blayer; 
      TH1* pixel;
      TH1* sct;
      TH1* si;
      TH1* trt;
      TH1* trt_xe;
      TH1* trt_total;
      TH1* trt_ht;
      TH1* trt_ht_total;
      TH1* dphirescaled;
      TH1* eProbHT;
      TH1* deltaPoverP;
      TH1* EoverP;
      TH2* trtratio;
      TH2* trtvseta;
      TH2* trthtvseta;

      unsigned m_d0_nBins = 200;
      unsigned m_d0sig_nBins = 50;
      unsigned m_z0_nBins = 200;
      std::vector<double> m_d0Range = {-100.0,100.0};
      std::vector<double> m_d0sigRange = {-25.0,25.0};
      std::vector<double> m_z0Range = {-100.0,100.0};
      
      void Set_d0_nBins(unsigned d0_nBins);
      void Set_d0sig_nBins(unsigned d0sig_nBins);
      void Set_z0_nBins(unsigned z0_nBins);
      void Set_d0_Bins(const std::vector<double> &d0Range);
      void Set_d0sig_Bins(const std::vector<double> &d0sigRange);
      void Set_z0_Bins(const std::vector<double> &z0Range);

      unsigned Get_d0_nBins(){ return m_d0_nBins; };
      unsigned Get_d0sig_nBins(){ return m_d0sig_nBins; };
      unsigned Get_z0_nBins(){ return m_z0_nBins; };
      const std::vector<double>& Get_d0_Bins(){ return m_d0Range; };
      const std::vector<double>& Get_d0sig_Bins(){ return m_d0sigRange; };
      const std::vector<double>& Get_z0_Bins(){ return m_z0Range; };


    private:
      virtual void initializePlots();
      
};

}

#endif
