/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "WidthPlot.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AsgTools/AnaToolHandle.h"

#include "TH1D.h"
#include "TH2D.h"

#include "TruthPhotonHistograms.h"
#include "widthestimators.h"

using namespace widthestimators;

namespace egammaMonitoring {

  WidthPlot::WidthPlot(std::string name, std::string folder, ITHistSvc * &rootHistSvc ) :
    m_name(std::move(name)),
    m_folder(std::move(folder)),
    m_rootHistSvc(rootHistSvc) { }

  StatusCode WidthPlot::fill(IHistograms *input) {

    TruthPhotonHistograms *histograms = dynamic_cast<TruthPhotonHistograms*>(input);

    for ( const auto &p : histograms->histo2DMap ) {
      TH1D* hist68 =
        new TH1D(Form("%s_%s", m_name.c_str(), p.first.c_str()),
                 Form("; %s ; resolution", p.second->GetXaxis()->GetTitle()),
                 p.second->GetXaxis()->GetNbins(),
                 p.second->GetXaxis()->GetXmin(),
                 p.second->GetXaxis()->GetXmax());
      hist68->SetStats(0);

      for (int bin = 1; bin <= p.second->GetXaxis()->GetNbins(); bin++) {
        TH1D *proj = p.second->ProjectionY(Form("%s_%d_projection", p.first.c_str(), bin), bin, bin+1);

        double s68 = binned::s68(proj);
        hist68->SetBinContent(bin, s68);

      }

      ATH_CHECK(m_rootHistSvc->regHist(Form("%sWidth_%s", m_folder.c_str(),  p.first.c_str()), hist68));

    }

    return StatusCode::SUCCESS;

  } //


} // namespace
