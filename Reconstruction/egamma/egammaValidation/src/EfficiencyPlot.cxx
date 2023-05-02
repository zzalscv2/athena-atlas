/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "EfficiencyPlot.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AsgMessaging/Check.h"

#include "TH1D.h"

namespace egammaMonitoring {

  EfficiencyPlot::EfficiencyPlot(std::string name, std::string folder, ITHistSvc * &rootHistSvc ) :
    m_name(std::move(name)),
    m_folder(std::move(folder)),
    m_rootHistSvc(rootHistSvc) {
  }

  StatusCode EfficiencyPlot::divide(IHistograms *pass, IHistograms* total) {

    for (const auto &p : total->histoMap){
      TH1D *efficiency = (TH1D*) pass->histoMap[p.first]->Clone(Form("%s_%s",m_name.c_str(),p.first.c_str()));
      efficiency->Divide(pass->histoMap[p.first], p.second, 1, 1, "B");
      efficiency->GetYaxis()->SetTitle("Efficiency");
      efficiency->GetYaxis()->SetRangeUser(0,1.1);
      efficiency->SetStats(0);
      ATH_CHECK(m_rootHistSvc->regHist(Form("%sEff_%s", m_folder.c_str(),  p.first.c_str()), efficiency));
    }

    return StatusCode::SUCCESS;

  } //


} // namespace
