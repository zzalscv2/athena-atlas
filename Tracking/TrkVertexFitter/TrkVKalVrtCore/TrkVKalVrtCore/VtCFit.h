/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VTCFIT_H
#define TRKVKALVRTCORE_VTCFIT_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

int vtcfit(VKVertex* vk);
double setLimitedFitVrt(VKVertex* vk, double alf, double bet, double dCoefNorm,
                        double newVrt[3]);
double calcChi2Addition(VKVertex* vk, const double wgtvrtd[6],
                        const double xyzf[3]);
void makeNoPostFit(VKVertex* vk, double wgtvrtd[], double& dCoefNorm);
bool makePostFit(VKVertex* vk, double wgtvrtd[], double& dCoefNorm);
}  // namespace Trk

#endif

