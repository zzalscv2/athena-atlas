// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXCUDA_TRACKPARTICLECALIBRATE_H
#define ATHEXCUDA_TRACKPARTICLECALIBRATE_H

// Local include(s).
#include "TrackParticleContainer.h"

namespace AthCUDAExamples {

/// "Calibrate" one TrackParticleContainer, into another container.
void calibrate(const TrackParticleContainer::const_view& input,
               TrackParticleContainer::view output);

}  // namespace AthCUDAExamples

#endif  // ATHEXCUDA_TRACKPARTICLECALIBRATE_H
