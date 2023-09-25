/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MaterialInteraction.h, ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_MATERIALINTERACTION_H
#define TRKGEOMETRY_MATERIALINTERACTION_H

#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/MaterialProperties.h"

namespace Trk {

/** @class MaterialInteraction
 Collection of parametrizations used in the Tracking realm
 @author sarka.todorova@cern.ch
 @author Christos Anastopoulos (Athena MT)
*/

struct MaterialInteraction
{
  /** dE/dl ionization energy loss per path unit */
  static double dEdl_ionization(
    double p,
    const Material& mat,
    ParticleHypothesis particle,
    double& sigma,
    double& kazL);

  /** dE/dl ionization energy loss per path unit */
  static double dEdXBetheBloch(
    const Trk::MaterialProperties& mat,
    double beta,
    double gamma,
    Trk::ParticleHypothesis particle);

  /** Most Propable dE ionization energly loss */
  static double dE_MPV_ionization(
    double p,
    const Trk::Material& mat,
    Trk::ParticleHypothesis particle,
    double& sigma,
    double& kazL,
    double path);

  /** dE/dl radiation energy loss per path unit */
  static double dEdl_radiation(
    double p,
    const Material& mat,
    ParticleHypothesis particle,
    double& sigma);

  static double dEdXBetheHeitler(
    const Trk::MaterialProperties& mat,
    double initialE,
    Trk::ParticleHypothesis particle);

  /** multiple scattering as function of dInX0 */
  static double sigmaMS(
    double dInX0,
    double p,
    double beta);
};
} // namespace Trk

#endif
