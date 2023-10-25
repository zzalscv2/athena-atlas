/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFITCASCADE_H
#define TRKVKALVRTCORE_CFITCASCADE_H

#include <vector>

#include "TrkVKalVrtCore/TrkVKalUtils.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
namespace Trk {

int setVTrackMass(VKVertex *vk);
long int getVertexCharge(VKVertex *vk);
double cascadeCnstRemnants(CascadeEvent &cascadeEvent_);
int fitVertexCascade(VKVertex *vk, int Pointing);
int processCascade(CascadeEvent &cascadeEvent_);
int processCascadePV(CascadeEvent &cascadeEvent_, const double *primVrt,
                     const double *primVrtCov);
int processCascade(CascadeEvent &cascadeEvent_, const double *primVrt,
                   const double *primVrtCov);
int processCascade(CascadeEvent &cascadeEvent_, double *primVrt);
int translateToFittedPos(CascadeEvent &cascadeEvent_, double Step = 1.0);
int restorePreviousPos(CascadeEvent &cascadeEvent_, std::vector<VKVertex> &SV);
void getFittedCascade(CascadeEvent &cascadeEvent_,
                      std::vector<Vect3DF> &cVertices,
                      std::vector<std::vector<double> > &covVertices,
                      std::vector<std::vector<VectMOM> > &fittedParticles,
                      std::vector<std::vector<double> > &cascadeCovar,
                      std::vector<double> &particleChi2,
                      std::vector<double> &fullCovar);

}  // namespace Trk

#endif

