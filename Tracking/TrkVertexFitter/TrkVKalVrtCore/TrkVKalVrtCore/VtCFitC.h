/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VTCFITC_H
#define TRKVKALVRTCORE_VTCFITC_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

int vtcfitc(VKVertex* vk);
double getCnstValues2(VKVertex* vk) noexcept;
}  // namespace Trk

#endif

