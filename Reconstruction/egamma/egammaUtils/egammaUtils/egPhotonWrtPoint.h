/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGPHOTONWRTPOINT_H
#define EGPHOTONWRTPOINT_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "xAODEgamma/EgammaFwd.h"

namespace photonWrtPoint {
/**
 * @brief egamma clusters kinematics are always
 * wrt the ATLAS frame (0,0,0).
 * In certain cases we might want to express things
 * wrt to some other point.
 */

/* The cluster is assumed to be massless -- > Photon*/
struct PtEtaPhi {
  double pt = 0;
  double eta = 0;
  double phi = 0;
};

/**Function to get the kinematics of a photon cluster wrt (0,0,z0) */
PtEtaPhi PtEtaPhiWrtZ(const xAOD::Egamma& ph, double z);
/**Function to modify in place the kinematics of a photon
 * wrt (0,0,z0) */
void correctForZ(xAOD::Egamma& ph, double z);

}  // namespace photonWrtPoint

#endif
