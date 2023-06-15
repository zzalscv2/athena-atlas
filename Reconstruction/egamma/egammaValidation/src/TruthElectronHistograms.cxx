/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthElectronHistograms.h"

#include "AsgTools/AnaToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "xAODTracking/TrackParticle.h"

#include "TH1D.h"
#include "TH2D.h"

using namespace egammaMonitoring;

StatusCode TruthElectronHistograms::initializePlots() {
  return initializePlots (false);
}

StatusCode TruthElectronHistograms::initializePlots(bool reducedHistSet) {

  if (!reducedHistSet) {
    histoMap["deltaPhi2"] = new TH1D(Form("%s_%s",m_name.c_str(),"deltaPhi2"), ";deltaPhi2; Events", 40, -0.06, 0.06);
    histoMap["deltaEta2"] = new TH1D(Form("%s_%s",m_name.c_str(),"deltaEta2"), ";deltaEta2; Events", 40, -0.04, 0.04);
    histoMap["deltaPhiRescaled2"] = new TH1D(Form("%s_%s",m_name.c_str(),"deltaPhiRescaled2"), ";deltaPhiRescaled2; Events", 40, -0.04, 0.04);

    histoMap["d0Oversigmad0"] = new TH1D(Form("%s_%s",m_name.c_str(),"d0Oversigmad0"), "; d0Oversigmad0; Events", 40, -10, 10);
    histoMap["qOverp_resolution"] = new TH1D(Form("%s_%s",m_name.c_str(),"qOverp_resolution"), ";(q/P reco - q/P truth)/ q/p truth; Events", 60, -1, 1.5);

    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"deltaPhi2", histoMap["deltaPhi2"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"deltaEta2", histoMap["deltaEta2"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"deltaPhiRescaled2", histoMap["deltaPhiRescaled2"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"d0Oversigmad0", histoMap["d0Oversigmad0"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"qOverp_resolution", histoMap["qOverp_resolution"]));

    // 2D only for truthPromptElectronWithRecoTrack (temporary)
    if (m_name == "truthPromptElectronWithRecoTrack") {
      histoMap2D["eta_deltaPhi2"] =
	new TH2D(Form("%s_%s",m_name.c_str(),"eta_deltaPhi2"),
		 ";#eta;#Delta#phi_{2}; Events", 60, -4.5, 4.5, 40, -0.06, 0.06);
      histoMap2D["eta_deltaEta2"] =
	new TH2D(Form("%s_%s",m_name.c_str(),"eta_deltaEta2"),
		 ";#eta;#Delta#eta_{2}; Events", 60, -4.5, 4.5, 40, -0.06, 0.06);
      histoMap2D["eta_deltaPhiRescaled2"] =
	new TH2D(Form("%s_%s",m_name.c_str(),"eta_deltaPhiRescaled2"),
		 ";#eta;#Delta#phi_{2}^{Rescaled}; Events", 60, -4.5, 4.5, 40, -0.06, 0.06);
      histoMap2D["eta_d0Oversigmad0"] =
	new TH2D(Form("%s_%s",m_name.c_str(),"eta_d0Oversigmad0"),
		 ";#eta;d_{0}/#sigma_{d_{0}}; Events", 60, -4.5, 4.5, 40, -10, 10);
      histoMap2D["eta_qOverp_resolution"] =
	new TH2D(Form("%s_%s",m_name.c_str(),"eta_qOverp_resolution"),
		 ";#eta;(q/P_{reco})/(q/P_{truth}) -1; Events", 60, -4.5, 4.5, 60, -1, 1.5);

      for (auto e : histoMap2D) {
	ATH_CHECK(m_rootHistSvc->regHist(m_folder+e.first, e.second));
      }
    }
  }
  
  ATH_CHECK(ParticleHistograms::initializePlots());

  m_reducedHistSet = reducedHistSet;

  return StatusCode::SUCCESS;

} 


void TruthElectronHistograms::fill(const xAOD::TruthParticle *truth, const xAOD::Electron* electron) {

  ParticleHistograms::fill(*truth);

  if (!electron || m_reducedHistSet) return;

  const xAOD::TrackParticle* track = electron->trackParticle();

  // This can happen if we use it for forwardElectron
  if (!track) return;

  bool has2DHis = histoMap2D.size() > 0;

  float dphires2(0.);
  float dphi2(0.);
  float deta2(0);

  if (electron->trackCaloMatchValue(dphires2, xAOD::EgammaParameters::deltaPhiRescaled2)) {
    histoMap["deltaPhiRescaled2"]->Fill(dphires2);
    if (has2DHis) histoMap2D["eta_deltaPhiRescaled2"]->Fill(electron->eta(),dphires2);
  }
  if (electron->trackCaloMatchValue(dphi2, xAOD::EgammaParameters::deltaPhi2)) {
    histoMap["deltaPhi2"]->Fill(dphi2);
    if (has2DHis) histoMap2D["eta_deltaPhi2"]->Fill(electron->eta(),dphi2);
  }
  if (electron->trackCaloMatchValue(deta2, xAOD::EgammaParameters::deltaEta2)) {
    histoMap["deltaEta2"]->Fill(deta2);
    if (has2DHis) histoMap2D["eta_deltaEta2"]->Fill(electron->eta(),deta2);
  }

  float d0 = track->d0(); 
  float reco_qp = track->qOverP();
  float truth_qp = truth->charge()/(truth->pt()*cosh(truth->eta()));
  float vard0 = track->definingParametersCovMatrix()(0, 0);

  if (vard0 > 0) {
    histoMap["d0Oversigmad0"]->Fill(d0/sqrtf(vard0));
    if (has2DHis) histoMap2D["eta_d0Oversigmad0"]->Fill(electron->eta(),d0/sqrtf(vard0));
  }
  
  if (truth_qp > 0) {
    histoMap["qOverp_resolution"]->Fill((reco_qp-truth_qp)/truth_qp);
    if (has2DHis) histoMap2D["eta_qOverp_resolution"]->Fill(electron->eta(),(reco_qp-truth_qp)/truth_qp);
  }
  

}
  
