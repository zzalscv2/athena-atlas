/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   MultiComponentStateModeCalculator.h
 * @date   Thursday 6th July 2006
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 *  Calculate the mode of the gaussian mixture and the relevant
 *  uncertainties
 */

#ifndef Trk_MultiComponentStateModeCalculator_H
#define Trk_MultiComponentStateModeCalculator_H

#include "TrkParameters/ComponentParameters.h"
//
#include <array>
#include <vector>

namespace Trk {
namespace MultiComponentStateModeCalculator {

/** @brief  Method to calculate mode with MultiComponentState
 * state as input */
std::array<double, 10>
calculateMode(const MultiComponentState&);

} // namespace MultiComponentStateModeCalculator

} // namespace Trk

#endif
