/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFTOTCOV_H
#define TRKVKALVRTCORE_CFTOTCOV_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

int afterFit(VKVertex *vk, double *ader, double *dcv, double *ptot,
             double *VrtMomCov, const VKalVrtControlBase *CONTROL);
int afterFitWithIniPar(VKVertex *vk, double *ader, double *dcv, double *ptot,
                       double *VrtMomCov, const VKalVrtControlBase *CONTROL);

}  // namespace Trk

#endif

