/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VTCFITE_H
#define TRKVKALVRTCORE_VTCFITE_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

int getFullVrtCov(VKVertex *vk, double *ader, const double *dcv,
                  double verr[6][6]);

}  // namespace Trk

#endif

