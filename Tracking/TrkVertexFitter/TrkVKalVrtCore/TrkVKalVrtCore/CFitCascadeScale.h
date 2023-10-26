/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFITCASCADESCALE_H
#define TRKVKALVRTCORE_CFITCASCADESCALE_H

#include <vector>

#include "TrkVKalVrtCore/TrkVKalUtils.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
namespace Trk {
void rescaleVrtErrForPointing(double Div, CascadeEvent& cascadeEvent_);
int fitVertexCascadeScale(VKVertex* vk, double& distToVertex);
int processCascadeScale(CascadeEvent& cascadeEvent_);
}  // namespace Trk

#endif

