/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFIT_H
#define TRKVKALVRTCORE_CFIT_H

#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
namespace Trk {

int CFit(VKalVrtControl *FitCONTROL, int ifCovV0, int NTRK,
         long int *ich, double xyz0[3], double (*par0)[3],
         double (*inp_Trk5)[5], double (*inp_CovTrk5)[15],
         double xyzfit[3], double (*parfs)[3], double ptot[4],
         double covf[21], double &chi2, double *chi2tr);

}  // namespace Trk

#endif
