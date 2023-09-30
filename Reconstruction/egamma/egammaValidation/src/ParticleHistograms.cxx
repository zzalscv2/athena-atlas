/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ParticleHistograms.h"

#include "GaudiKernel/ITHistSvc.h"
#include "AsgMessaging/Check.h"
#include "xAODBase/IParticle.h"
#include "TH1D.h"

namespace egammaMonitoring {

  StatusCode ParticleHistograms::initializePlots() {

    histoMap["pT"]   = new TH1D(Form("%s_%s",m_name.c_str(),"pT")  , ";p_{T} [GeV];Events",  40,            0,         200);
    histoMap["eta"]  = new TH1D(Form("%s_%s",m_name.c_str(),"eta") , ";#eta;Events"       ,  60,         -4.5,         4.5);
    histoMap["phi"]  = new TH1D(Form("%s_%s",m_name.c_str(),"phi") , ";#phi;Events"       ,  20, -TMath::Pi(), TMath::Pi());

    histoMap["eta_15GeV"]  = new TH1D(Form("%s_%s",m_name.c_str(),"eta_15GeV") , ";#eta;Events"       ,  60,         -4.5,         4.5);
    histoMap["phi_15GeV"]  = new TH1D(Form("%s_%s",m_name.c_str(),"phi_15GeV") , ";#phi;Events"       ,  20, -TMath::Pi(), TMath::Pi());


    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"pT", histoMap["pT"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eta", histoMap["eta"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"phi", histoMap["phi"]));

    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eta_15GeV", histoMap["eta_15GeV"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"phi_15GeV", histoMap["phi_15GeV"]));

    if (m_hasFwd) {
      histoMap["pT_eta4"]   = new TH1D(Form("%s_%s",m_name.c_str(),"pT_eta4")  ,";p_{T} [GeV];Events",  40,            0,         200);
      histoMap["phi_eta4"]  = new TH1D(Form("%s_%s",m_name.c_str(),"phi_eta4") , ";#phi;Events"       ,  20, -TMath::Pi(), TMath::Pi());
      ATH_CHECK(m_rootHistSvc->regHist(m_folder+"pT_eta4", histoMap["pT_eta4"]));
      ATH_CHECK(m_rootHistSvc->regHist(m_folder+"phi_eta4", histoMap["phi_eta4"]));
    }

    return StatusCode::SUCCESS;
  }

  void ParticleHistograms::fill(const xAOD::IParticle& egamma) {
    ParticleHistograms::fill(egamma,0.);
  }


  void ParticleHistograms::fill(const xAOD::IParticle& egamma, float /*mu*/) {

    double pT = egamma.pt()*1e-3;
    if (pT > 0)
      histoMap["pT"]->Fill(pT);
    histoMap["eta"]->Fill(egamma.eta());
    histoMap["phi"]->Fill(egamma.phi());

    if (pT > 15) {
      histoMap["eta_15GeV"]->Fill(egamma.eta());
      histoMap["phi_15GeV"]->Fill(egamma.phi());
    }

    if (m_hasFwd && std::abs(egamma.eta()) < 4.) {
      histoMap["pT_eta4"]->Fill(pT);
      histoMap["phi_eta4"]->Fill(egamma.phi());
    }

  }

}
