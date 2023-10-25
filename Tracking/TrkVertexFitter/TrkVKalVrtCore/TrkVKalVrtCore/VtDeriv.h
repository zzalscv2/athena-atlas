/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VTDERIV_H
#define TRKVKALVRTCORE_VTDERIV_H
#include <array>
namespace Trk {
class VKalVrtControl;
void vpderiv(bool UseTrackErr, long int Charge, const double *pari0,
             double *covi, double *vrtref, double *covvrtref, double *drdpar,
             double *dwgt, double *rv0, VKalVrtControl *FitCONTROL);

}  // namespace Trk

#endif
