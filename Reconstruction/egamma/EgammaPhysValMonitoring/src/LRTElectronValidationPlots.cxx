/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LRTElectronValidationPlots.h"
#include "xAODEgamma/EgammaDefs.h"

LRTElectronValidationPlots::LRTElectronValidationPlots(PlotBase* pParent, const std::string& sDir):PlotBase(pParent, sDir),
                      m_oCentralElecPlots(this,"Central/", "Central"),
                      author(nullptr),
                      mu_average(nullptr),
                      mu_actual(nullptr),
                      res_et(nullptr),
                      res_eta(nullptr),
                      res_et_cut(nullptr),
                      res_eta_cut(nullptr),
                      res_et_cut_pt_20(nullptr),
                      res_eta_cut_pt_20(nullptr),
                      matrix(nullptr)

{
  m_oCentralElecPlots.Set_d0_nBins(50);
  m_oCentralElecPlots.Set_d0sig_nBins(50);
  m_oCentralElecPlots.Set_z0_nBins(50);
  m_oCentralElecPlots.Set_d0_Bins(std::vector<double>{-300.0,300.0});
  m_oCentralElecPlots.Set_d0sig_Bins(std::vector<double>{-300.0,300.0});
  m_oCentralElecPlots.Set_z0_Bins(std::vector<double>{-300.0,300.0});
}	

void LRTElectronValidationPlots::initializePlots(){

  author       = Book1D("author", "Electron  Author ; author;Events", 20, -0.5, 19.5);
  mu_average  = Book1D("mu_average", "#mu average interactions per crossing ; #mu average;Events", 100, -0.5, 99.5);
  mu_actual    = Book1D("mu_actual", "#mu actual interactions per crossing ; #mu_actual;Events", 100, -0.5, 99.5);
  res_et       = BookTProfile("res_et"," IsoElectron;E_{T}^{Truth}, [GeV];(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",100, 0., 200.);
  res_eta      = BookTProfile("res_eta"," IsoElectron;#eta;(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",50, -2.5, 2.5);
  res_et_cut   = BookTProfile("res_et_cut"," IsoElectron;E_{T}^{Truth}, [GeV];(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",100, 0., 200.);
  res_eta_cut  = BookTProfile("res_eta_cut"," IsoElectron;#eta;(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",50, -2.5, 2.5);
  res_et_cut_pt_20   = BookTProfile("res_et_cut_pt_20"," Prompt Electron;E_{T}^{Truth}, [GeV];(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",100, 0., 200.);
  res_eta_cut_pt_20  = BookTProfile("res_eta_cut_pt_20"," Prompt Electron;#eta;(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",50, -2.5, 2.5);
  matrix       = Book2D("matrix","reco vs truth pt",200,0.,200.,200,0.,200.);

}
 
void LRTElectronValidationPlots::fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt, bool pass_LHVeryLooseNoPix, bool pass_LHLooseNoPix, bool pass_LHMediumNoPix, bool pass_LHTightNoPix) {

  float weight = eventInfo.beamSpotWeight();

  author->Fill(electron.author(),weight);
  
  if(electron.author()&xAOD::EgammaParameters::AuthorElectron||
     electron.author(xAOD::EgammaParameters::AuthorAmbiguous))  {
    m_oCentralElecPlots.fill(electron, eventInfo, isPrompt, pass_LHVeryLooseNoPix, pass_LHLooseNoPix, pass_LHMediumNoPix, pass_LHTightNoPix);
  } 
}
