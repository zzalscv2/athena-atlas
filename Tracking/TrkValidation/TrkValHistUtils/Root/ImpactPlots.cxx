/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkValHistUtils/ImpactPlots.h"

namespace Trk {
  void
  ImpactPlots::init() {
    z0 = nullptr;
    z0sig = nullptr;
    d0 = nullptr;
    d0sig = nullptr;
    d0_small = nullptr;
  }

  void
  ImpactPlots::initializePlots() {
    z0 = Book1D("z0", "z0;Signed Impact Parameter z0 (mm);Entries / 5 mm", 120, -300, 300);
//  z0sig  = Book1D("z0sig","z0sig,;Signed Impact Parameter Significance; Entries / 0.1", 100, 0, 10);
    d0 = Book1D("d0", "d0;Signed Impact Parameter d0 (mm);Entries / 0.025 mm", 24000, -300, 300);
//  d0sig  = Book1D("d0sig","d0sig,;Signed Impact Parameter Significance; Entries / 0.1", 100, 0, 10);
    d0_small = Book1D("d0_small", "d0;Signed Impact Parameter d0 (mm);Entries / 0.0025 mm", 80, -0.1, 0.1);
  }

  void
  ImpactPlots::fill(const xAOD::TrackParticle &trkprt, float weight) {
    d0->Fill(trkprt.d0(),weight);
    d0_small->Fill(trkprt.d0(),weight);
    z0->Fill(trkprt.z0(),weight);
  }
}
