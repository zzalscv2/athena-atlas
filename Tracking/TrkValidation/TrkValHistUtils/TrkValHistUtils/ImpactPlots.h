/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVALHISTUTILS_IMPACTPLOTS_H
#define TRKVALHISTUTILS_IMPACTPLOTS_H

#include "PlotBase.h"
#include "xAODTracking/TrackParticle.h"

namespace Trk{

class ImpactPlots: public PlotBase {
  public:
    ImpactPlots(PlotBase *pParent, const std::string& sDir):PlotBase(pParent, sDir){ init();}
  void fill(const xAOD::TrackParticle& trkprt, float weight=1.0);
    
    TH1* z0;
    TH1* z0sig;
    TH1* d0;
    TH1* d0sig;
    TH1* d0_small;
		
  private:
    void init();
    void initializePlots();
};

}

#endif
