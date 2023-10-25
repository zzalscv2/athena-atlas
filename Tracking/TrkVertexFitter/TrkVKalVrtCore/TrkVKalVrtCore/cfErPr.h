/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFERPR_H
#define TRKVKALVRTCORE_CFERPR_H

namespace Trk {
void cferpr(const long int ich, double *par, double *ref, const double s0,
            double *errold, double *errnew);
}  // namespace Trk

#endif
