// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
/*
2D-function to describe the efficiency versus pT and eta
obtained from XXX
(original code from A.Cerri and P. Calfayan, used for the L1Track task-force, 2020)
https://twiki.cern.ch/twiki/bin/viewauth/Atlas/HTTPerfWithEmulation
*/
#include <cmath>
#include <stdio.h>
double getEffParam_N(float abstrketa, float trkpt, bool debug=0) {

  double eff = 1;

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
