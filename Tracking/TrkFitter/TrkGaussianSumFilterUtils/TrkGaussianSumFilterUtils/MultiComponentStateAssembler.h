/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   MultiComponentStateAssembler.h
 * @date   Monday 20th December 2004
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 *  Helper struct and methods to collect components of a
 *  multi-component state and put them all into a
 *  MultiComponentState. The addition of components can be
 *  one at a time through the addComponent method (taking a
 *  single ComponentParameters object) or many at a time,(taking a
 *  MultiComponentState object). It also included
 *  helpers to be used in weighting renormalisation
 *  calculations and the removal of components with
 *  insignificantly small weightings.
 */

#ifndef MultiComponentStateAssembler_H
#define MultiComponentStateAssembler_H

#include "TrkGaussianSumFilterUtils/GsfConstants.h"
#include "TrkParameters/ComponentParameters.h"
#include <vector>

namespace Trk {

/**
 * Helper struct representing a cache of the Multicomponent
 * state under assembly
 */
namespace MultiComponentStateAssembler {
struct Cache {
  Cache() {
    multiComponentState.reserve(GSFConstants::maxNumberofStateComponents);
  }
  Trk::MultiComponentState multiComponentState{};
  double validWeightSum = 0;
  double invalidWeightSum = 0;
  static constexpr double minimumValidFraction = 0.01;
  static constexpr double minimumFractionalWeight = 1e-09;
};

/** Method to add a single set of Trk::ComponentParameters to the cached
 * Trk::MultiComponentState object under construction */
void
addComponent(
  MultiComponentStateAssembler::Cache& cache,
  ComponentParameters&& multiComponentState);

/** Method to add a new Trk::MultiComponentState to the cached
 * Trk::MultiComponentState object under construction */
void
addMultiState(
  MultiComponentStateAssembler::Cache& cache,
  Trk::MultiComponentState&& multiComponentState);

/** Method to return the cached state object -
 * it performs a reweighting before returning the object based on the
 * valid and invaid weights */
MultiComponentState
assembledState(MultiComponentStateAssembler::Cache&& cache);

} // End MultiComponentStateAssembler namespace

} // End Trk namepace

// inline methods
inline void Trk::MultiComponentStateAssembler::addComponent(
    Cache& cache, ComponentParameters&& componentParameters) {

  cache.validWeightSum += componentParameters.weight;
  cache.multiComponentState.emplace_back(std::move(componentParameters.params),
                                         componentParameters.weight);
}

inline void Trk::MultiComponentStateAssembler::addMultiState(
    Cache& cache, Trk::MultiComponentState&& multiComponentState) {

  double sumW(0.);
  for (auto& component : multiComponentState) {
    sumW += component.weight;
    cache.multiComponentState.emplace_back(std::move(component.params),
                                           component.weight);
  }
  cache.validWeightSum += sumW;
}

#endif
