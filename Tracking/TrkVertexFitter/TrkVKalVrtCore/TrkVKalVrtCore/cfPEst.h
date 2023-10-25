/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFPEST_H
#define TRKVKALVRTCORE_CFPEST_H

namespace Trk {
void cfpest(int ntrk, double *xyz, long int *ich, double (*parst)[5],
            double (*parf)[3]);
}  // namespace Trk

#endif
