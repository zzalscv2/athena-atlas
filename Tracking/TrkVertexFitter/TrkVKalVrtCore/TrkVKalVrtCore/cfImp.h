/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFIMP_H
#define TRKVKALVRTCORE_CFIMP_H

#include <array>

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

void cfimp(long int TrkID, long int ich, int IFL, double *par,
           const double *err, double *vrt, double *vcov, double *rimp,
           double *rcov, double *sign, VKalVrtControlBase *FitCONTROL);

void cfimpc(long int TrkID, long int ich, int IFL, double *par,
            const double *err, double *vrt, double *vcov, double *rimp,
            double *rcov, double *sign, VKalVrtControlBase *FitCONTROL);

}  // namespace Trk

#endif

