/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_XYZTRP_H
#define TRKVKALVRTCORE_XYZTRP_H

namespace Trk {
void xyztrp(const long int ich, double *vrt0, double *pv0, double *covi,
            double BMAG, double *paro, double *errt);
void combinedTrack(long int ICH, double *pv0, double *covi, double BMAG,
                   double *par, double *covo);
}  // namespace Trk

#endif
