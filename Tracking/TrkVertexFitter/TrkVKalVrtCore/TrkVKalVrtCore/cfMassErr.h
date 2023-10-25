/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFMASSERR_H
#define TRKVKALVRTCORE_CFMASSERR_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

void cfmasserr(VKVertex *vk, const int *list, double BMAG, double *MASS,
               double *sigM);
void cfmasserrold_(const long int ntrk, long int *list, double *parfs,
                   double *ams, double *deriv, double BMAG, double *dm,
                   double *sigm);

}  // namespace Trk

#endif

