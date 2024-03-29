/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVALHISTUTILS_RESOLUTIONPLOTS_H
#define TRKVALHISTUTILS_RESOLUTIONPLOTS_H

#include "PlotBase.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"

namespace Trk{

class ResolutionPlots: public PlotBase {
  public:
    ResolutionPlots(PlotBase *pParent, const std::string& sDir, const std::string& sType=""):PlotBase(pParent, sDir),m_sType(sType) { init();}
  void fill(const xAOD::TrackParticle& trkprt, const xAOD::TruthParticle& truthprt, float weight=1.0);

    TH1* Res_pT;
    TH1* Res_eta;
    TH1* Res_phi;
  private:
    std::string m_sType;
    void init();
    void initializePlots();


};

}

#endif

