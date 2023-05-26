// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
/*
2D-function to describe the d0 resolution versus pT and eta, for Nominal geometry
obtained by fitting curves with a post processing of the indetphysvalmonitoring outputs
(original code from A.Cerri and P. Calfayan, used for the L1Track task-force, 2020)
https://twiki.cern.ch/twiki/bin/viewauth/Atlas/HTTPerfWithEmulation
*/
#include <cmath>
#include <stdio.h>
double getD0ResParam_N(float abstrketa, float trkpt, bool debug=0) {

  double d0Res = -1;
  if (trkpt>=1.0 && trkpt<1.5) {
    if (abstrketa<4.0) {
      d0Res += 62.28*pow(abstrketa, 0);
      d0Res += 7.69*pow(abstrketa, 1);
      d0Res += -0.38*pow(abstrketa, 2);
      d0Res += 1.76*pow(abstrketa, 3);
      d0Res += 24.13*exp(-0.5*pow((abstrketa-1.82)/0.49,2));
      d0Res += 31.98*exp(-0.5*pow((abstrketa-2.33)/0.26,2));
    }
  }

  if (trkpt>=1.5 && trkpt<2.5) {
    if (abstrketa<4.0) {
      d0Res += 41.81*pow(abstrketa, 0);
      d0Res += 2.80*pow(abstrketa, 1);
      d0Res += 1.17*pow(abstrketa, 2);
      d0Res += 0.97*pow(abstrketa, 3);
      d0Res += 16.61*exp(-0.5*pow((abstrketa-1.93)/0.58,2));
      d0Res += 15.48*exp(-0.5*pow((abstrketa-2.35)/0.26,2));
    }
  }

  if (trkpt>=2.5 && trkpt<5.0) {
    if (abstrketa<4.0) {
      d0Res += 25.53*pow(abstrketa, 0);
      d0Res += 1.28*pow(abstrketa, 1);
      d0Res += -2.25*pow(abstrketa, 2);
      d0Res += 10.19*pow(abstrketa, 3);
      d0Res += -4.72*pow(abstrketa, 4);
      d0Res += 0.64*pow(abstrketa, 5);
      d0Res += 5.26*exp(-0.5*pow((abstrketa-2.30)/0.26,2));
    }
  }

  if (trkpt>=5.0 && trkpt<10.0) {
    if (abstrketa<2.9) {
      d0Res += 15.68*pow(abstrketa, 0);
      d0Res += 5.02*exp(-0.5*pow((abstrketa-1.63)/0.50,2));
      d0Res += 15.63*exp(-0.5*pow((abstrketa-2.60)/0.49,2));
    }
    if (abstrketa>=2.9 && abstrketa<4.0) {
      d0Res += -129.25*pow(abstrketa, 0);
      d0Res += 46.90*pow(abstrketa, 1);
      d0Res += 20.96*exp(-0.5*pow((abstrketa-2.96)/0.28,2));
    }
  }

  if (trkpt>=10.0 && trkpt<20.0) {
    if (abstrketa<4.0) {
      d0Res += 10.54*pow(abstrketa, 0);
      d0Res += -3.03*pow(abstrketa, 1);
      d0Res += 3.63*pow(abstrketa, 2);
      d0Res += -5.05*pow(abstrketa, 3);
      d0Res += 1.18*pow(abstrketa, 4);
      d0Res += 27.84*exp(-0.5*pow((abstrketa-2.81)/0.95,2));
    }
  }

  if (trkpt>=20.0 && trkpt<100.0) {
    if (abstrketa<4.0) {
      d0Res += 7.26*pow(abstrketa, 0);
      d0Res += -0.72*pow(abstrketa, 1);
      d0Res += -0.20*pow(abstrketa, 2);
      d0Res += 1.36*pow(abstrketa, 3);
      d0Res += -0.32*pow(abstrketa, 4);
      d0Res += -0.16*pow(abstrketa, 5);
      d0Res += 0.05*pow(abstrketa, 6);
      d0Res += 1.72*exp(-0.5*pow((abstrketa-2.39)/0.15,2));
    }
  }

  if (debug) printf("d0Res = %f\n", d0Res);
  return d0Res;
}
