/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VKVFAST_H
#define TRKVKALVRTCORE_VKVFAST_H

namespace Trk {

void vkvfast_(double *p1, double *p2, const double *dbmag, double *out);
double vkvFastV(double *p1, double *p2, const double *vRef, double dbmag,
                double *out);
double vkvang_(double *crs, double *centr, const double *r__, const double *xd,
               const double *yd);
}  // namespace Trk

#endif

