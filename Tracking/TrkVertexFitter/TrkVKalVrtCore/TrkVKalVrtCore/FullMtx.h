/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_FULLMTX_H
#define TRKVKALVRTCORE_FULLMTX_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

void FullMTXfill(VKVertex * vk, double * ader);
int FullMCNSTfill(VKVertex * vk, double * ader, double * LSide);
}  // namespace Trk

#endif

