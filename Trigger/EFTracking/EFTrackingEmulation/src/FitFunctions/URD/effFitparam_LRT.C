// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
// 2D-function to describe the tracking efficiency versus pT and eta. See README
//pt is in GeV

#include <cmath>
#include <iostream>
double getEffParam_LRT(float abstrketa, float trkpt, double d0, bool debug=false) {

  double eff = 1;
  if (d0 > 3) eff = 0.1;
  if (abstrketa<0.5) {
    if (trkpt<2.)
      eff = 0.89;
    else if (trkpt<3.)
      eff = 0.91;
    else 
      eff = 0.92;
  } else if (abstrketa<1.5) {
    if (trkpt<2.)
      eff = 0.87;
    else if (trkpt<3.)
      eff = 0.90;
    else
      eff = 0.91;
  } else {
    if (trkpt<2.)
      eff = 0.77;
    else if (trkpt<3.)
      eff = 0.80;
    else
      eff = 0.80;
  }

  if (debug) printf("eff = %f\n", eff);
  return eff;
}
