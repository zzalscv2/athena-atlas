/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_Resolution.cxx
 * @author nora pettersson
 **/

#include "InDetPerfPlot_Resolution.h"
#include <map>
#include <utility>
#include <cmath>
#include "logLinearBinning.h"
#include "TruthUtils/MagicNumbers.h"

namespace{
  constexpr float undefinedValue{-9999.};
  constexpr float smallestAllowableSin(1e-8);
  constexpr float smallestAllowableTan(1e-8);
  constexpr float smallestAllowableQoverPt(1e-8);
}

InDetPerfPlot_Resolution::InDetPerfPlot_Resolution(InDetPlotBase* pParent, const std::string& sDir)  : InDetPlotBase(pParent, sDir),
  m_resolutionMethod(IDPVM::ResolutionHelper::iterRMS_convergence),
  m_primTrk(false),
  m_secdTrk(false),
  m_allTrk(false),

  m_trkP{},
  m_truetrkP{},
  m_trkErrP{},

  m_resP{},
  m_pullP{},
  m_sigP{},

  m_pull{},
  m_res{},
  m_sigma{},

  m_resHelpereta{},
  m_resHelperpt{},
  m_resHelperlowpt{},
  m_pullHelpereta{}, 
  m_pullHelperpt{}, 
  m_pullHelperlowpt{}, 

  m_reswidth_vs_eta{},
  m_resmean_vs_eta{},
  m_reswidth_vs_pt{},
  m_resmean_vs_pt{},
  m_reswidth_vs_lowpt{},
  m_resmean_vs_lowpt{},

  m_pullwidth_vs_eta{},
  m_pullmean_vs_eta{},
  m_pullwidth_vs_pt{},
  m_pullmean_vs_pt{},
  m_pullwidth_vs_lowpt{},
  m_pullmean_vs_lowpt{},

  m_resHelpereta_pos{},
  m_resHelpereta_neg{},
  m_resHelperpt_pos{}, 
  m_resHelperpt_neg{},
  m_resHelperlowpt_pos{}, 
  m_resHelperlowpt_neg{},

  m_reswidth_vs_eta_pos{},
  m_resmean_vs_eta_pos{},
  m_reswidth_vs_pt_pos{},
  m_resmean_vs_pt_pos{},
  m_reswidth_vs_lowpt_pos{},
  m_resmean_vs_lowpt_pos{},

  m_reswidth_vs_eta_neg{},
  m_resmean_vs_eta_neg{},
  m_reswidth_vs_pt_neg{},
  m_resmean_vs_pt_neg{},
  m_reswidth_vs_lowpt_neg{},
  m_resmean_vs_lowpt_neg{},

  m_pullProjections_vs_pt{},
  m_pullProjections_vs_lowpt{},
  m_pullProjections_vs_eta{},
  m_resProjections_vs_pt{},
  m_resProjections_vs_lowpt{},
  m_resProjections_vs_eta{},

  m_sigma_vs_eta{},
  m_sigma_vs_pt{},
  m_sigma_vs_lowpt{}
  {
    
  TString tsDir = (TString) sDir;

  if (tsDir.Contains("Primary")) {
    m_primTrk = true; // control if sec/prim from init
  } else if (tsDir.Contains("Secondary")) {
    m_secdTrk = true;
  } else {
    m_allTrk = true;
  }

  
   
  std::vector<double> ptBins = IDPVM::logLinearBinning(m_nPtBins, m_ptMin, m_ptMax, false);
  for (int ipt = 0; ipt <= m_nPtBins; ipt++) {
    m_PtBins[ipt] = (float) ptBins[ipt];
  }
  
  std::vector<double> lowPtBins = IDPVM::logLinearBinning(m_nLowPtBins, m_lowPtMin, m_lowPtMax, false);
  for (int ipt = 0; ipt <= m_nLowPtBins; ipt++) {
    m_LowPtBins[ipt] = (float) lowPtBins[ipt];
  }
  


}

void
InDetPerfPlot_Resolution::initializePlots() {
  
  for (unsigned int iparam = 0; iparam < NPARAMS; iparam++) {
    //
    //1D distributions
    //

    if(iparam == PT) continue;
    book(m_pull[iparam], "pull_" + m_paramProp[iparam]);
    book(m_res[iparam],  "res_" + m_paramProp[iparam]);

    book(m_sigma[iparam], "sigma_" + m_paramProp[iparam]); //New One 
    //
    //2D Distributions to evaluate resolutions vs eta and pT
    //
    book(m_resHelpereta[iparam], "resHelper_eta_" + m_paramProp[iparam]);
    book(m_resHelperpt[iparam], "resHelper_pt_" + m_paramProp[iparam]);
    book(m_resHelperlowpt[iparam], "resHelper_lowpt_" + m_paramProp[iparam]);
    book(m_pullHelpereta[iparam], "pullHelper_eta_" + m_paramProp[iparam]);
    book(m_pullHelperpt[iparam], "pullHelper_pt_" + m_paramProp[iparam]);
    book(m_pullHelperlowpt[iparam], "pullHelper_lowpt_" + m_paramProp[iparam]);
    m_resHelperpt[iparam]->GetXaxis()->Set(m_nPtBins,m_PtBins);
    m_pullHelperpt[iparam]->GetXaxis()->Set(m_nPtBins,m_PtBins);
    m_resHelperlowpt[iparam]->GetXaxis()->Set(m_nLowPtBins,m_LowPtBins);
    m_pullHelperlowpt[iparam]->GetXaxis()->Set(m_nLowPtBins,m_LowPtBins);
    //
    //1D Histograms for the final resolution and means
    //
    book(m_reswidth_vs_eta[iparam], "resolution_vs_eta_" + m_paramProp[iparam]);
    book(m_resmean_vs_eta[iparam], "resmean_vs_eta_" + m_paramProp[iparam]);
    book(m_reswidth_vs_pt[iparam], "resolution_vs_pt_" + m_paramProp[iparam]);
    book(m_resmean_vs_pt[iparam], "resmean_vs_pt_" + m_paramProp[iparam]);
    book(m_reswidth_vs_lowpt[iparam], "resolution_vs_lowpt_" + m_paramProp[iparam]);
    book(m_resmean_vs_lowpt[iparam], "resmean_vs_lowpt_" + m_paramProp[iparam]);

    book(m_pullwidth_vs_eta[iparam], "pullwidth_vs_eta_" + m_paramProp[iparam]);
    book(m_pullmean_vs_eta[iparam], "pullmean_vs_eta_" + m_paramProp[iparam]);
    book(m_pullwidth_vs_pt[iparam], "pullwidth_vs_pt_" + m_paramProp[iparam]);
    book(m_pullmean_vs_pt[iparam], "pullmean_vs_pt_" + m_paramProp[iparam]);
    book(m_pullwidth_vs_lowpt[iparam], "pullwidth_vs_lowpt_" + m_paramProp[iparam]);
    book(m_pullmean_vs_lowpt[iparam], "pullmean_vs_lowpt_" + m_paramProp[iparam]);

    m_reswidth_vs_pt[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
    m_resmean_vs_pt[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
    m_pullwidth_vs_pt[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
    m_pullmean_vs_pt[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
    m_reswidth_vs_lowpt[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);
    m_resmean_vs_lowpt[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);
    m_pullwidth_vs_lowpt[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);
    m_pullmean_vs_lowpt[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);

    //
    //TProfiles of the sqrt(covii) 
    //
    book(m_sigma_vs_eta[iparam], "sigma_vs_eta_" + m_paramProp[iparam]);
    book(m_sigma_vs_pt[iparam], "sigma_vs_pt_" + m_paramProp[iparam]);
    book(m_sigma_vs_lowpt[iparam], "sigma_vs_lowpt_" + m_paramProp[iparam]);

    //NORA
    //
    //Projections only done if high level of detail specified
    //To keep functionalies to produce projections for varied number of pt / eta bins
    //keep histogram to the old Book1D methods instead of xml definitions
    //
    //
    //Detailed histograms
    //
    if(m_iDetailLevel >= 200){
      book(m_resHelpereta_pos[iparam], "resHelper_eta_" + m_paramProp[iparam], "resHelper_eta_" + m_paramProp[iparam]+"_posQ"); 
      book(m_resHelpereta_neg[iparam], "resHelper_eta_" + m_paramProp[iparam], "resHelper_eta_" + m_paramProp[iparam]+"_negQ");
      book(m_resHelperpt_pos[iparam],  "resHelper_pt_" + m_paramProp[iparam], "resHelper_pt_" + m_paramProp[iparam]+"_posQ"); 
      book(m_resHelperpt_neg[iparam],  "resHelper_pt_" + m_paramProp[iparam], "resHelper_pt_" + m_paramProp[iparam]+"_negQ");
      book(m_resHelperlowpt_pos[iparam],  "resHelper_lowpt_" + m_paramProp[iparam], "resHelper_lowpt_" + m_paramProp[iparam]+"_posQ"); 
      book(m_resHelperlowpt_neg[iparam],  "resHelper_lowpt_" + m_paramProp[iparam], "resHelper_lowpt_" + m_paramProp[iparam]+"_negQ");

      //Add log binning
      m_resHelperpt_pos[iparam]->GetXaxis()->Set(m_nPtBins,m_PtBins);
      m_resHelperpt_neg[iparam]->GetXaxis()->Set(m_nPtBins,m_PtBins);
      m_resHelperlowpt_pos[iparam]->GetXaxis()->Set(m_nLowPtBins,m_LowPtBins);
      m_resHelperlowpt_neg[iparam]->GetXaxis()->Set(m_nLowPtBins,m_LowPtBins);

      //Resolution, Resolution Mean, Pull, Pull Mean
      book(m_reswidth_vs_eta_pos[iparam], "resolution_vs_eta_" + m_paramProp[iparam], "resolution_vs_eta_" + m_paramProp[iparam] + "_posQ");
      book(m_reswidth_vs_eta_neg[iparam], "resolution_vs_eta_" + m_paramProp[iparam], "resolution_vs_eta_" + m_paramProp[iparam] + "_negQ");
      book(m_resmean_vs_eta_pos[iparam], "resmean_vs_eta_" + m_paramProp[iparam], "resmean_vs_eta_" + m_paramProp[iparam] + "_posQ");
      book(m_resmean_vs_eta_neg[iparam], "resmean_vs_eta_" + m_paramProp[iparam], "resmean_vs_eta_" + m_paramProp[iparam] + "_negQ");

      book(m_reswidth_vs_pt_pos[iparam], "resolution_vs_pt_" + m_paramProp[iparam], "resolution_vs_pt_" + m_paramProp[iparam] + "_posQ");
      book(m_reswidth_vs_pt_neg[iparam], "resolution_vs_pt_" + m_paramProp[iparam], "resolution_vs_pt_" + m_paramProp[iparam] + "_negQ");
      book(m_resmean_vs_pt_pos[iparam], "resmean_vs_pt_" + m_paramProp[iparam], "resmean_vs_pt_" + m_paramProp[iparam] + "_posQ");
      book(m_resmean_vs_pt_neg[iparam], "resmean_vs_pt_" + m_paramProp[iparam], "resmean_vs_pt_" + m_paramProp[iparam] + "_negQ");

      book(m_reswidth_vs_lowpt_pos[iparam], "resolution_vs_lowpt_" + m_paramProp[iparam], "resolution_vs_lowpt_" + m_paramProp[iparam] + "_posQ");
      book(m_reswidth_vs_lowpt_neg[iparam], "resolution_vs_lowpt_" + m_paramProp[iparam], "resolution_vs_lowpt_" + m_paramProp[iparam] + "_negQ");
      book(m_resmean_vs_lowpt_pos[iparam], "resmean_vs_lowpt_" + m_paramProp[iparam], "resmean_vs_lowpt_" + m_paramProp[iparam] + "_posQ");
      book(m_resmean_vs_lowpt_neg[iparam], "resmean_vs_lowpt_" + m_paramProp[iparam], "resmean_vs_lowpt_" + m_paramProp[iparam] + "_negQ");
     
      m_reswidth_vs_pt_pos[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
      m_resmean_vs_pt_pos[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
      m_reswidth_vs_pt_neg[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
      m_resmean_vs_pt_neg[iparam]->GetXaxis()->Set(m_nPtBins, m_PtBins);
      m_reswidth_vs_lowpt_pos[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);
      m_resmean_vs_lowpt_pos[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);
      m_reswidth_vs_lowpt_neg[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);
      m_resmean_vs_lowpt_neg[iparam]->GetXaxis()->Set(m_nLowPtBins, m_LowPtBins);

      std::string tmpName, tmpTitle;

      int nPtBins = m_pullHelperpt[iparam]->GetNbinsX(); 
      int nLowPtBins = m_pullHelperlowpt[iparam]->GetNbinsX(); 
      int nEtaBins = m_pullHelpereta[iparam]->GetNbinsX(); 

      std::shared_ptr<TH1D> refHistEta { m_pullHelpereta[iparam]->ProjectionY("refEta")}; 
      std::shared_ptr<TH1D> refHistPt { m_pullHelperpt[iparam]->ProjectionY("refPt")}; 
      std::shared_ptr<TH1D> refHistLowPt { m_pullHelperlowpt[iparam]->ProjectionY("refLowPt")}; 

      //Projections - VERY expensive, hence only at higher detail levels above 200
      if(m_iDetailLevel > 200){
        for (int ibins = 0; ibins < nPtBins; ibins++) {
          tmpName = "pullProjection_pt_" + m_paramProp[iparam] +"_bin_"+ std::to_string(ibins + 1);
          tmpTitle = tmpName + "; (" + m_paramProp[iparam] + "^{reco}-" + m_paramProp[iparam] +
                    "^{true})/#sigma_{" + m_paramProp[iparam] + "}";
          m_pullProjections_vs_pt[iparam][ibins] = Book1D(tmpName, refHistPt.get(), tmpTitle , false);


          tmpName = "resProjection_pt_" + m_paramProp[iparam] +"_bin_"+ std::to_string(ibins + 1);
          tmpTitle = tmpName + "; " + m_paramProp[iparam] + "^{reco}-" + m_paramProp[iparam] + "^{true} ";
          m_resProjections_vs_pt[iparam][ibins] = Book1D(tmpName, refHistPt.get(), tmpTitle , false);

        }
        for (int ibins = 0; ibins < nLowPtBins; ibins++) {
          tmpName = "pullProjection_lowpt_" + m_paramProp[iparam] +"_bin_"+ std::to_string(ibins + 1);
          tmpTitle = tmpName + "; (" + m_paramProp[iparam] + "^{reco}-" + m_paramProp[iparam] +
                    "^{true})/#sigma_{" + m_paramProp[iparam] + "}";
          m_pullProjections_vs_lowpt[iparam][ibins] = Book1D(tmpName, refHistLowPt.get(), tmpTitle , false);


          tmpName = "resProjection_lowpt_" + m_paramProp[iparam] +"_bin_"+ std::to_string(ibins + 1);
          tmpTitle = tmpName + "; " + m_paramProp[iparam] + "^{reco}-" + m_paramProp[iparam] + "^{true} ";
          m_resProjections_vs_lowpt[iparam][ibins] = Book1D(tmpName, refHistLowPt.get(), tmpTitle , false);

        }
        for (int ibins = 0; ibins < nEtaBins; ibins++) {
          tmpName = "pullProjection_eta_" + m_paramProp[iparam] +"_bin_"+ std::to_string(ibins + 1);
          tmpTitle = tmpName + "; (" + m_paramProp[iparam] + "^{reco}-" + m_paramProp[iparam] +
                    "^{true})/#sigma_{" + m_paramProp[iparam] + "}";
          m_pullProjections_vs_eta[iparam][ibins] = Book1D(tmpName, refHistEta.get(), tmpTitle , false);

          
          tmpName = "resProjection_eta_" + m_paramProp[iparam] +"_bin_"+ std::to_string(ibins + 1);
          tmpTitle = tmpName + "; " + m_paramProp[iparam] + "^{reco}-" + m_paramProp[iparam] + "^{true} ";
          m_resProjections_vs_eta[iparam][ibins] = Book1D(tmpName, refHistEta.get(), tmpTitle , false);
        }  
      }   
   }
   //
   //End of saving resolution and pull residual binnings
   //

  }

}

void
InDetPerfPlot_Resolution::fill(const xAOD::TrackParticle& trkprt, const xAOD::TruthParticle& truthprt, float weight) {

  int isPrimTrk = 0;
  int isSecdTrk = 0;

  if (HepMC::is_simulation_particle(&truthprt)) {
    isSecdTrk = 1;
  } else {
    if (truthprt.barcode()>0) isPrimTrk = 1;
  } 

  // Move on to the next track incase the wrong track category
  if (!isPrimTrk && !isSecdTrk) {
    return;
  }
  if (!isPrimTrk && m_primTrk) {
    return;
  }
  if (!isSecdTrk && m_secdTrk) {
    return;
  }

  // Get track paramters for truth and reco tracks for easy access
  // store all in track parameter maps trkP/truetrkP
  getTrackParameters(trkprt);
  getTrackParameters(truthprt);
  getPlotParameters();
  getPlots(weight);

}

void
InDetPerfPlot_Resolution::getPlots(float weight) {
  const float tanHalfTheta = std::tan(m_truetrkP[THETA] * 0.5);
  const bool tanThetaIsSane = std::abs(tanHalfTheta) > smallestAllowableTan;
  float eta = undefinedValue;
  if (tanThetaIsSane) eta = -std::log(tanHalfTheta);
  for (unsigned int iparam = 0; iparam < NPARAMS; iparam++) {    
    if(iparam == PT) continue;
    m_pull[iparam]->Fill(m_pullP[iparam], weight);
    m_res[iparam]->Fill(m_resP[iparam], weight);
    m_sigma[iparam]->Fill(m_sigP[iparam], weight);
    m_sigma_vs_eta[iparam]->Fill(eta, m_sigP[iparam], weight);
    m_sigma_vs_pt[iparam]->Fill(m_truetrkP[PT], m_sigP[iparam], weight);
    m_sigma_vs_lowpt[iparam]->Fill(m_truetrkP[PT], m_sigP[iparam], weight);
    m_resHelpereta[iparam]->Fill(eta, m_resP[iparam], weight);
    m_resHelperpt[iparam]->Fill(m_truetrkP[PT], m_resP[iparam], weight);
    m_resHelperlowpt[iparam]->Fill(m_truetrkP[PT], m_resP[iparam], weight);
    m_pullHelperpt[iparam]->Fill(m_truetrkP[PT], m_pullP[iparam], weight);
    m_pullHelperlowpt[iparam]->Fill(m_truetrkP[PT], m_pullP[iparam], weight);
    m_pullHelpereta[iparam]->Fill(eta, m_pullP[iparam], weight);
    
    if(m_iDetailLevel >= 200){
      if (m_trkP[QOVERPT] >= 0.) {
        m_resHelpereta_pos[iparam]->Fill(eta, m_resP[iparam], weight);
        m_resHelperpt_pos[iparam]->Fill(m_truetrkP[PT], m_resP[iparam], weight);
        m_resHelperlowpt_pos[iparam]->Fill(m_truetrkP[PT], m_resP[iparam], weight);
      }
      if (m_trkP[QOVERPT] < 0.) {
        m_resHelpereta_neg[iparam]->Fill(eta, m_resP[iparam], weight);
        m_resHelperpt_neg[iparam]->Fill(m_truetrkP[PT], m_resP[iparam], weight);
        m_resHelperlowpt_neg[iparam]->Fill(m_truetrkP[PT], m_resP[iparam], weight);
      }
    }

  }

}

void
InDetPerfPlot_Resolution::getPlotParameters() {
  for (unsigned int iparam = 0; iparam < NPARAMS; iparam++) {
    m_resP[iparam] = m_trkP[iparam] - m_truetrkP[iparam];
    m_sigP[iparam] = m_trkErrP[iparam];
    (m_sigP[iparam] != 0) ? m_pullP[iparam] = m_resP[iparam] / m_sigP[iparam] : m_pullP[iparam] = undefinedValue;
  }
  bool saneQoverPt = std::abs(m_truetrkP[QOVERPT]) > smallestAllowableQoverPt;
  if (not saneQoverPt){
    m_resP[QOVERPT] = undefinedValue;
  } else {
      m_resP[QOVERPT] = (m_trkP[QOVERPT] - m_truetrkP[QOVERPT]) * (1 / m_truetrkP[QOVERPT]);  // relative q/pt resolution
  }
  bool saneQoverPtrec = std::abs(m_trkP[QOVERPT]) > smallestAllowableQoverPt;
  if (not saneQoverPtrec){
    m_sigP[QOVERPT] = undefinedValue;
  } else {
    m_sigP[QOVERPT] *= 1. / std::abs(m_trkP[QOVERPT]);  // relative q/pt error
  }
}

void
InDetPerfPlot_Resolution::getTrackParameters(const xAOD::TrackParticle& trkprt) {

  m_trkP[D0] = trkprt.d0();
  m_trkP[Z0] = trkprt.z0();
  m_trkP[QOVERP] = trkprt.qOverP() * Gaudi::Units::GeV;
  const float sinTheta{std::sin(trkprt.theta())};
  const bool saneSineValue = (std::abs(sinTheta) > smallestAllowableSin);
  const float inverseSinTheta = saneSineValue ? 1./sinTheta : undefinedValue;
  m_trkP[QOVERPT] = saneSineValue ? trkprt.qOverP()*inverseSinTheta : undefinedValue;
  m_trkP[THETA] = trkprt.theta();
  m_trkP[PHI] = trkprt.phi0();
  m_trkP[PT] = trkprt.pt() / Gaudi::Units::GeV;
  m_trkP[Z0SIN] = trkprt.z0() * sinTheta;

  // Track fit errors
  m_trkErrP[D0] = std::sqrt(trkprt.definingParametersCovMatrix()(0, 0));
  m_trkErrP[Z0] = std::sqrt(trkprt.definingParametersCovMatrix()(1, 1));
  m_trkErrP[PHI] = std::sqrt(trkprt.definingParametersCovMatrix()(2, 2));
  m_trkErrP[THETA] = std::sqrt(trkprt.definingParametersCovMatrix()(3, 3));
  m_trkErrP[QOVERP] = std::sqrt(trkprt.definingParametersCovMatrix()(4, 4)) * Gaudi::Units::GeV;
  m_trkErrP[QOVERPT] = std::sqrt(trkprt.definingParametersCovMatrix()(4, 4)) * inverseSinTheta;
  m_trkErrP[Z0SIN] = std::sqrt(std::pow(m_trkErrP[Z0] * std::sin(m_trkP[THETA]), 2) + 
                               std::pow(m_trkP[Z0] * m_trkErrP[THETA] * std::cos(m_trkP[THETA]), 2) + 
                               2 * m_trkP[Z0] * std::sin(m_trkP[THETA]) * std::cos(m_trkP[THETA]) * trkprt.definingParametersCovMatrix()(1, 3));  // Fixing incorrect formula for z0sin error


  // Get error on pT, taken from xAOD::TrackingHelpers.pTErr() but this function only works on a pointer input...
  if (trkprt.definingParametersCovMatrixVec().size() < 15) {
    throw std::runtime_error(
            "TrackParticle without covariance matrix for defining parameters or the covariance matrix is wrong dimensionality.");
  }
  if (std::abs(trkprt.qOverP()) < 0) {
    m_trkErrP[PT] = 0.0;
    throw std::runtime_error("q/p is zero");
  } else {
    double pt = trkprt.pt();
    double diff_qp = -pt / std::abs(trkprt.qOverP());
    double diff_theta = pt / std::tan(trkprt.theta());
    const std::vector<float>& cov = trkprt.definingParametersCovMatrixVec();
    double pt_err2 = diff_qp * (diff_qp * cov[14] + diff_theta * cov[13]) + diff_theta * diff_theta * cov[9];
    m_trkErrP[PT] = std::sqrt(pt_err2) / Gaudi::Units::GeV;
  }

}


  void
InDetPerfPlot_Resolution::getTrackParameters(const xAOD::TruthParticle& truthprt) {
  // "d0", "z0", "phi", "theta" , "qOverP"
  // Perigee for truth particles are in aux container
  for (int iParams = 0; iParams < NPARAMS; iParams++) {
    m_truetrkP[iParams] = undefinedValue;
    if (truthprt.isAvailable<float>(m_paramProp[iParams])) {
      m_truetrkP[iParams] = (truthprt.auxdata<float>(m_paramProp[iParams]));
    }
  }
  //rescale Pt 
  m_truetrkP[PT] = truthprt.pt() / Gaudi::Units::GeV;
  //special cases
  //need both theta and qOverP for qOverPT
  //we didnt check qOverP yet, since the closest named variable (strangely; see header) is ptqopt
  const float qOverP = truthprt.isAvailable<float>("qOverP") ? truthprt.auxdata<float>("qOverP") : undefinedValue;
  if ((qOverP != undefinedValue) and (m_truetrkP[THETA] != undefinedValue)){
    const float sinTheta =std::sin(m_truetrkP[THETA]);
    if (std::abs(sinTheta) > smallestAllowableSin){
      m_truetrkP[QOVERPT] = qOverP /sinTheta;
    } else {
      m_truetrkP[QOVERPT] = undefinedValue;
    }
  }
  m_truetrkP[QOVERP] = qOverP * Gaudi::Units::GeV;
  //need both z0 and theta for z0sinTheta
  if ((m_truetrkP[Z0] != undefinedValue) and (m_truetrkP[THETA] != undefinedValue)){
    const float sinTheta =std::sin(m_truetrkP[THETA]);
    m_truetrkP[Z0SIN] = m_truetrkP[Z0] * sinTheta;
  } else {
    m_truetrkP[Z0SIN] = undefinedValue;
  }
}


void
InDetPerfPlot_Resolution::finalizePlots() {
    
  bool saveProjections = (m_iDetailLevel > 200);
  for (unsigned int iparam = 0; iparam < NPARAMS; iparam++) {
    if(iparam == PT) continue;
    //
    //Only save vert detailed information if... high detail level
    //Reduces output for ART / PhysVal
    //
    if(m_iDetailLevel >= 200){
      m_resolutionHelper.makeResolutions(m_resHelpereta[iparam], m_reswidth_vs_eta[iparam], m_resmean_vs_eta[iparam],
                      (saveProjections ? m_resProjections_vs_eta[iparam] : nullptr), saveProjections, m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperpt[iparam], m_reswidth_vs_pt[iparam], m_resmean_vs_pt[iparam],
                      (saveProjections ? m_resProjections_vs_pt[iparam] : nullptr), saveProjections, m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperlowpt[iparam], m_reswidth_vs_lowpt[iparam], m_resmean_vs_lowpt[iparam],
                      (saveProjections ? m_resProjections_vs_lowpt[iparam] : nullptr), saveProjections, m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_pullHelperpt[iparam], m_pullwidth_vs_pt[iparam], m_pullmean_vs_pt[iparam],
                      (saveProjections ? m_pullProjections_vs_pt[iparam] : nullptr), saveProjections, m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_pullHelperlowpt[iparam], m_pullwidth_vs_lowpt[iparam], m_pullmean_vs_lowpt[iparam],
                      (saveProjections ? m_pullProjections_vs_lowpt[iparam] : nullptr), saveProjections, m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_pullHelpereta[iparam], m_pullwidth_vs_eta[iparam], m_pullmean_vs_eta[iparam],
                      (saveProjections ? m_pullProjections_vs_eta[iparam] : nullptr), saveProjections, m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperpt_pos[iparam], m_reswidth_vs_pt_pos[iparam], m_resmean_vs_pt_pos[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperpt_neg[iparam], m_reswidth_vs_pt_neg[iparam], m_resmean_vs_pt_neg[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperlowpt_pos[iparam], m_reswidth_vs_lowpt_pos[iparam], m_resmean_vs_lowpt_pos[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperlowpt_neg[iparam], m_reswidth_vs_lowpt_neg[iparam], m_resmean_vs_lowpt_neg[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelpereta_pos[iparam], m_reswidth_vs_eta_pos[iparam], m_resmean_vs_eta_pos[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelpereta_neg[iparam], m_reswidth_vs_eta_neg[iparam], m_resmean_vs_eta_neg[iparam], m_resolutionMethod);
    } else {
      m_resolutionHelper.makeResolutions(m_resHelpereta[iparam], m_reswidth_vs_eta[iparam], m_resmean_vs_eta[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperpt[iparam], m_reswidth_vs_pt[iparam], m_resmean_vs_pt[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_resHelperlowpt[iparam], m_reswidth_vs_lowpt[iparam], m_resmean_vs_lowpt[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_pullHelperpt[iparam], m_pullwidth_vs_pt[iparam], m_pullmean_vs_pt[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_pullHelperlowpt[iparam], m_pullwidth_vs_lowpt[iparam], m_pullmean_vs_lowpt[iparam], m_resolutionMethod);
      m_resolutionHelper.makeResolutions(m_pullHelpereta[iparam], m_pullwidth_vs_eta[iparam], m_pullmean_vs_eta[iparam], m_resolutionMethod);
    }

  }

}
