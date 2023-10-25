/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_DERCLCANG_H
#define TRKVKALVRTCORE_DERCLCANG_H
#include <array>

namespace Trk {
class VKPhiConstraint;
class VKThetaConstraint;
class VKPlaneConstraint;
void calcPhiConstraint( VKPhiConstraint * cnst);
void calcThetaConstraint( VKThetaConstraint * cnst);
void calcPlaneConstraint( VKPlaneConstraint * cnst);
}  // namespace Trk

#endif

