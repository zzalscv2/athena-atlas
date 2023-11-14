/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHYSVALMONITORING_TRKANDVTXPLOTS_H
#define PHYSVALMONITORING_TRKANDVTXPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODEventInfo/EventInfo.h"

namespace PhysVal{
  
class TrkAndVtxPlots:public PlotBase {
    public:
      TrkAndVtxPlots(PlotBase* pParent, const std::string& sDir);
      void fill(const xAOD::TrackParticle* trk,const xAOD::EventInfo* evt);
      void fill(const xAOD::Vertex* vtx,const xAOD::EventInfo* evt);
      void fill(unsigned int ntrack, unsigned int nvertex, const xAOD::EventInfo* evt=NULL);
      
      // Reco only information
      TH1* ntrk = nullptr;

      TH1* nvtx = nullptr;
      TH1* vtx_x = nullptr;
      TH1* vtx_y = nullptr;
      TH1* vtx_z = nullptr;

    private:
      virtual void initializePlots();

      
};

}

#endif
