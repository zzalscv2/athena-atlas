/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CASCADEDEFINITION_H
#define TRKVKALVRTCORE_CASCADEDEFINITION_H

#include <memory>

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {
VKVertex *startCascade(std::unique_ptr<VKVertex> vk);
VKVertex *addCascadeEntry(std::unique_ptr<VKVertex> vk);
VKVertex *addCascadeEntry(std::unique_ptr<VKVertex> vk,
                          const std::vector<int> &index);
int makeCascade(VKalVrtControl &FitCONTROL, long int NTRK, const long int *ich,
                double *wm, double *inp_Trk5, double *inp_CovTrk5,
                const std::vector<std::vector<int> > &vertexDefinition,
                const std::vector<std::vector<int> > &cascadeDefinition,
                double definedCnstAccuracy = 1.e-4);
int initCascadeEngine(CascadeEvent &cascadeEvent_);
int setCascadeMassConstraint(CascadeEvent &cascadeEvent_, long int IV,
                             double Mass);
int setCascadeMassConstraint(CascadeEvent &cascadeEvent_, long int IV,
                             std::vector<int> &trkInVrt,
                             std::vector<int> &pseudoInVrt, double Mass);

}  // namespace Trk

#endif

