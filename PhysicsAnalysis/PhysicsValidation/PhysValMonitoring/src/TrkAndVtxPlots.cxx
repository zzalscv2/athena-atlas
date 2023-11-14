/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkAndVtxPlots.h"



using CLHEP::GeV;

namespace PhysVal{

TrkAndVtxPlots::TrkAndVtxPlots(PlotBase* pParent, const std::string& sDir):PlotBase(pParent, sDir)
{}	
  
void TrkAndVtxPlots::initializePlots(){
  ntrk  = Book1D("ntrk", "Number of tracks; n ;Events", 3000, 0., 3000);
  nvtx  = Book1D("nvtx", "Number of vertices; n ;Events", 150, 0., 150);

  vtx_x  = Book1D("x", "Vertex x; x ;Events", 200, -1., 1);
  vtx_y  = Book1D("y", "Vertex y; y ;Events", 300, -1.5, 1.5);
  vtx_z  = Book1D("z", "Vertex z; z ;Events", 200, -250., 250);
}

 void TrkAndVtxPlots::fill(const xAOD::Vertex* vtx,const xAOD::EventInfo* evt) {
    
   vtx_x->Fill(vtx->x(),evt->beamSpotWeight());
   vtx_y->Fill(vtx->y(),evt->beamSpotWeight());
   vtx_z->Fill(vtx->z(),evt->beamSpotWeight());

}

  void TrkAndVtxPlots::fill(const xAOD::TrackParticle* /*trk*/, const xAOD::EventInfo* evt){
   std::cout << "filling TrackAndVertex plots with BS weight: " << evt->beamSpotWeight();
}

  void TrkAndVtxPlots::fill(unsigned int ntrack, unsigned int nvertex, const xAOD::EventInfo* evt) {

  ntrk->Fill(ntrack,evt->beamSpotWeight());
  nvtx->Fill(nvertex,evt->beamSpotWeight());
}

}
