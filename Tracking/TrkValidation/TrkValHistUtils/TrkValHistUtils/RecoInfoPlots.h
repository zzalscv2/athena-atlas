/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVALHISTUTILS_RECOINFOPLOTS_H
#define TRKVALHISTUTILS_RECOINFOPLOTS_H

#include "PlotBase.h"
#include "xAODTracking/TrackParticle.h"

namespace Trk{

class RecoInfoPlots: public PlotBase {
  public:
 RecoInfoPlots(PlotBase *pParent, const std::string& sDir, const std::string& sType=""):PlotBase(pParent, sDir),m_sType(sType){ init();}
  void fill(const xAOD::TrackParticle& trkprt, float weight=1.0);
 		
    TH1* trackfitchi2;
    TH1* trackfitndof;
    TH1* trackcon;
    TH1* trackchi2prob;
    
  private:
    std::string m_sType;
    void init();
    void initializePlots();
			
};

}

#endif

