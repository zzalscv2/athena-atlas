/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ComponentParameters.h
 * @date Sunday 8th May 2005
 * @author atkinson, Anthony Morley, Christos Anastopoulos
 * @brief  Definition of component parameters for use in a mixture
 * of many components. In this regime each track parameters
 * object comes with a weighting (double) attached
 */

#ifndef TrkComponentParameters
#define TrkComponentParameters

#include "TrkParameters/TrackParameters.h"
#include <utility>
#include <vector>
namespace Trk {

struct ComponentParameters {
  std::unique_ptr<Trk::TrackParameters> params{nullptr};
  double weight{0};
};

using MultiComponentState = std::vector<ComponentParameters>;

namespace MultiComponentStateHelpers {

/** Clone TrackParameters method */
MultiComponentState
clone(const MultiComponentState& in);

/** Scale the  covariance matrix components  by
   individual factors. */
MultiComponentState
WithScaledError(MultiComponentState&& in,
                double errorScaleLocX,
                double errorScaleLocY,
                double errorScalePhi,
                double errorScaleTheta,
                double errorScaleQoverP);

/** Check to see if all components
 * have covariance Matrix.*/
bool
allHaveCovariance(const MultiComponentState& in);

/** Performing renormalisation of total state weighting to one */
void
renormaliseState(MultiComponentState&, double norm = 1);

} // end of MultiComponentStateHelpers
} // end Trk namespace

#endif
