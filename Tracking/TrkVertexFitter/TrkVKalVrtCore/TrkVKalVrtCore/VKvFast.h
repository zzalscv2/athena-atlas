/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VKVFAST_H
#define TRKVKALVRTCORE_VKVFAST_H

namespace Trk {

double vkvFastV(double *p1, double *p2, const double *vRef, double dbmag,
                double *out);
}  // namespace Trk

#endif

