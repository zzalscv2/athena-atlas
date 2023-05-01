/*
Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_PARTICLEHISTOGRAMS_H
#define EGAMMAVALIDATION_PARTICLEHISTOGRAMS_H

#include "IHistograms.h"

namespace xAOD {
  class IParticle;
}

namespace egammaMonitoring {
  
  class ParticleHistograms: public IHistograms {
  public:

    using IHistograms::IHistograms;
    
    StatusCode initializePlots();

    void fill(const xAOD::IParticle& egamma);
    void fill(const xAOD::IParticle& egamma, float mu);

  };
 
}

#endif

