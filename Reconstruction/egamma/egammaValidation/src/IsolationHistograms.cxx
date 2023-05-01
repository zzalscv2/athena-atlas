/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "IsolationHistograms.h"

#include "AsgMessaging/Check.h"

#include "GaudiKernel/ITHistSvc.h"
 
#include "TH1D.h"


using namespace egammaMonitoring;

StatusCode IsolationHistograms::initializePlots(bool do_var_histos) {

  histoMap["ptCone20"] = new TH1D(Form("%s_%s",m_name.c_str(),"ptCone20" ), ";p_{T}^{cone20}; Events / 0.25 GeV", 60, 0., 15.);
  histoMap["ptCone30"] = new TH1D(Form("%s_%s",m_name.c_str(),"ptCone30" ), ";p_{T}^{cone30}; Events / 0.25 GeV", 60, 0., 15.);

  if (do_var_histos) {
    histoMap["ptVarCone20"] = new TH1D(Form("%s_%s",m_name.c_str(),"ptVarCone20" ), ";p_{T}^{varCone20}; Events / 0.25 GeV", 60, 0., 15.);
    histoMap["ptVarCone30"] = new TH1D(Form("%s_%s",m_name.c_str(),"ptVarCone30" ), ";p_{T}^{varCone30}; Events / 0.25 GeV", 60, 0., 15.);
  }
  histoMap["topoEtCone20"] = new TH1D(Form("%s_%s",m_name.c_str(),"topoEtCone20" ), ";E_{T}^{topoCone20}; Events / 0.8 GeV", 60, -20., 30.);
  histoMap["topoEtCone30"] = new TH1D(Form("%s_%s",m_name.c_str(),"topoEtCone30" ), ";E_{T}^{topoCone30}; Events / 0.8 GeV", 60, -20., 30.);
  histoMap["topoEtCone40"] = new TH1D(Form("%s_%s",m_name.c_str(),"topoEtCone40" ), ";E_{T}^{topoCone40}; Events / 0.8 GeV", 60, -20., 30.);

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"ptCone20", histoMap["ptCone20"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"ptCone30", histoMap["ptCone30"]));
  if (do_var_histos) {
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"ptVarCone20", histoMap["ptVarCone20"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"ptVarCone30", histoMap["ptVarCone30"]));
  }
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"topoEtCone20", histoMap["topoEtCone20"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"topoEtCone30", histoMap["topoEtCone30"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"topoEtCone40", histoMap["topoEtCone40"]));

  return StatusCode::SUCCESS;

} // initializePlots

void IsolationHistograms::fill(const xAOD::Egamma& egamma) {

  static const std::map<std::string,xAOD::Iso::IsolationType> mmap = {
    { "ptCone20", xAOD::Iso::ptcone20 },
    { "ptCone30", xAOD::Iso::ptcone30 },
    { "ptVarCone20", xAOD::Iso::ptvarcone20 },
    { "ptVarCone30", xAOD::Iso::ptvarcone30 },
    { "topoEtCone20", xAOD::Iso::topoetcone20 },
    { "topoEtCone30", xAOD::Iso::topoetcone30 },
    { "topoEtCone40", xAOD::Iso::topoetcone40 } };
  for (auto e : mmap) {
    if (histoMap.find(e.first) == histoMap.end())
      continue;
    float x = -9e9;
    if (egamma.isolationValue(x, e.second)) {
      TH1D *h = histoMap[e.first];
      double xmax = h->GetBinCenter(h->GetNbinsX());
      h->Fill(std::min(x/1000., xmax));
    }
  }
}
