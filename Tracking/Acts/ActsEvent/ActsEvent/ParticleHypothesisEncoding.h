/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_ParticleHypothesisEncoding_h
#define ActsEvent_ParticleHypothesisEncoding_h
#include "xAODTracking/TrackingPrimitives.h"
#include "Acts/EventData/ParticleHypothesis.hpp"


namespace ActsTrk {
namespace ParticleHypothesis {
    xAOD::ParticleHypothesis convert(Acts::ParticleHypothesis h);
    Acts::ParticleHypothesis convert(xAOD::ParticleHypothesis h);
}  // namespace ParticleHypothesis
}  // namespace ActsTrk

#endif