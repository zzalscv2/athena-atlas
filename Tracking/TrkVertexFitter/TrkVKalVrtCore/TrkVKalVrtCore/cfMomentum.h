/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFMOMENTUM_H
#define TRKVKALVRTCORE_CFMOMENTUM_H

#include <array>

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {
void vkPerigeeToP(const double *perig3, double *pp, double BMAG);
std::array<double, 4> getFitParticleMom(const VKTrack *trk, const VKVertex *vk);
std::array<double, 4> getFitParticleMom(const VKTrack *trk, double BMAG);
std::array<double, 4> getIniParticleMom(const VKTrack *trk, const VKVertex *vk);
std::array<double, 4> getIniParticleMom(const VKTrack *trk, double BMAG);
std::array<double, 4> getCnstParticleMom(const VKTrack *trk,
                                         const VKVertex *vk);
std::array<double, 4> getCnstParticleMom(const VKTrack *trk, double BMAG);

}  // namespace Trk

#endif

