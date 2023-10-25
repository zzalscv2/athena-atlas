/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_ROBTEST_H
#define TRKVKALVRTCORE_ROBTEST_H


#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

void robtest(VKVertex * vk, int ifl, int nIteration=10);
}  // namespace Trk

#endif

