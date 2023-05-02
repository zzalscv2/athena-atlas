/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_TRUTHPHOTONHISTOGRAMS_H
#define EGAMMAVALIDATION_TRUTHPHOTONHISTOGRAMS_H

#include "ParticleHistograms.h"
#include <map>

namespace xAOD{
  class IParticle;
}
class TH2D;

namespace egammaMonitoring {

  class TruthPhotonHistograms : public ParticleHistograms {
  public:

    using ParticleHistograms::ParticleHistograms;
    using ParticleHistograms::initializePlots;

    std::map<std::string, TH2D* > histo2DMap;

    StatusCode initializePlots();

    using ParticleHistograms::fill;

    void fill(const xAOD::IParticle&);
    void fill(const xAOD::IParticle&, float mu);

  private:

    float m_cR_bins[15] = {0, 50, 89, 123, 170, 210, 250, 299, 335, 371, 443, 514, 554, 800, 1085};

  };

}

#endif
