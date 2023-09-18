/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_RECOPHOTONHISTOGRAMS_H
#define EGAMMAVALIDATION_RECOPHOTONHISTOGRAMS_H

#include "xAODEgamma/Photon.h"
#include "ParticleHistograms.h"

class TH1D;

namespace egammaMonitoring {

  class RecoPhotonHistograms : public ParticleHistograms {

  public:
    using ParticleHistograms::ParticleHistograms;

    StatusCode initializePlots();

    using ParticleHistograms::fill;
    void fill(const xAOD::Photon& phrec);

  private:
    float m_cR_bins[15] =
      {0, 50, 89, 123, 170, 210, 250, 299, 335, 371, 443, 514, 554, 800, 1085};

  };

}

#endif
