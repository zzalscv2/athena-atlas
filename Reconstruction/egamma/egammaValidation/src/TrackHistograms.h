/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_TRACKHISTOGRAMS_H
#define EGAMMAVALIDATION_TRACKHISTOGRAMS_H

#include <map>

#include "IHistograms.h"

class IParticle;
class TProfile;

namespace egammaMonitoring {

  class TrackHistograms : public IHistograms {
  public:

    using IHistograms::IHistograms;
    
    std::map<std::string, TProfile*> profileMap;

    StatusCode initializePlots();
    void fill(const xAOD::IParticle& track, float mu);
    void fill(const xAOD::IParticle& track);

  };

}

#endif
