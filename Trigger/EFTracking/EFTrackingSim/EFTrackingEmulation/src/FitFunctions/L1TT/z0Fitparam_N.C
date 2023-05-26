// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
/*
2D-function to describe the z0 resolution versus pT and eta, for Nominal geometry
obtained by fitting curves with a post processing of the indetphysvalmonitoring outputs
(original code from A.Cerri and P. Calfayan, used for the L1Track task-force, 2020)
https://twiki.cern.ch/twiki/bin/viewauth/Atlas/HTTPerfWithEmulation
*/
#include <cmath>
#include <stdio.h>
double getZ0ResParam_N(float abstrketa, float trkpt, bool debug=0) {

  double z0Res = -1;
  if (trkpt>=1.0 && trkpt<1.5) {
    if (abstrketa<4.0) {
      z0Res += 62.49*pow(abstrketa, 0);
      z0Res += 11.15*pow(abstrketa, 1);
      z0Res += -97.86*pow(abstrketa, 2);
      z0Res += 340.16*pow(abstrketa, 3);
      z0Res += -276.95*pow(abstrketa, 4);
      z0Res += 85.68*pow(abstrketa, 5);
      z0Res += -8.04*pow(abstrketa, 6);
      z0Res += 414.60*exp(-0.5*pow((abstrketa-2.39)/0.46,2));
    }
  }

  if (trkpt>=1.5 && trkpt<2.5) {
    if (abstrketa<4.0) {
      z0Res += 43.65*pow(abstrketa, 0);
      z0Res += -9.94*pow(abstrketa, 1);
      z0Res += -16.34*pow(abstrketa, 2);
      z0Res += 148.41*pow(abstrketa, 3);
      z0Res += -108.10*pow(abstrketa, 4);
      z0Res += -3.09*pow(abstrketa, 5);
      z0Res += 15.41*pow(abstrketa, 6);
      z0Res += -2.37*pow(abstrketa, 7);
      z0Res += 512.88*exp(-0.5*pow((abstrketa-2.55)/0.56,2));
    }
  }

  if (trkpt>=2.5 && trkpt<5.0) {
    if (abstrketa<4.0) {
      z0Res += 28.44*pow(abstrketa, 0);
      z0Res += -21.22*pow(abstrketa, 1);
      z0Res += 17.80*pow(abstrketa, 2);
      z0Res += 67.20*pow(abstrketa, 3);
      z0Res += -67.35*pow(abstrketa, 4);
      z0Res += -1.57*pow(abstrketa, 5);
      z0Res += 10.43*pow(abstrketa, 6);
      z0Res += -1.64*pow(abstrketa, 7);
      z0Res += 427.85*exp(-0.5*pow((abstrketa-2.64)/0.63,2));
    }
  }

  if (trkpt>=5.0 && trkpt<10.0) {
    if (abstrketa<2.0) {
      z0Res += 20.48*pow(abstrketa, 0);
      z0Res += -48.16*pow(abstrketa, 1);
      z0Res += 135.27*pow(abstrketa, 2);
      z0Res += -141.57*pow(abstrketa, 3);
      z0Res += 72.27*pow(abstrketa, 4);
      z0Res += -12.49*pow(abstrketa, 5);
    }
    if (abstrketa>=2.0 && abstrketa<4.0) {
      z0Res += 1159.63*pow(abstrketa, 0);
      z0Res += -1036.00*pow(abstrketa, 1);
      z0Res += 256.18*pow(abstrketa, 2);
      z0Res += -4.22*pow(abstrketa, 3);
      z0Res += 74.45*exp(-0.5*pow((abstrketa-2.46)/0.27,2));
    }
  }

  if (trkpt>=10.0 && trkpt<20.0) {
    if (abstrketa<2.0) {
      z0Res += 15.61*pow(abstrketa, 0);
      z0Res += -45.29*pow(abstrketa, 1);
      z0Res += 119.31*pow(abstrketa, 2);
      z0Res += -124.47*pow(abstrketa, 3);
      z0Res += 61.85*pow(abstrketa, 4);
      z0Res += -10.88*pow(abstrketa, 5);
    }
    if (abstrketa>=2.0 && abstrketa<4.0) {
      z0Res += 211.71*pow(abstrketa, 0);
      z0Res += -83.16*pow(abstrketa, 1);
      z0Res += -37.82*pow(abstrketa, 2);
      z0Res += 19.11*pow(abstrketa, 3);
      z0Res += 30.03*exp(-0.5*pow((abstrketa-2.48)/0.25,2));
    }
  }

  if (trkpt>=20.0 && trkpt<100.0) {
    if (abstrketa<4.0) {
      z0Res += 12.24*pow(abstrketa, 0);
      z0Res += -45.41*pow(abstrketa, 1);
      z0Res += 122.61*pow(abstrketa, 2);
      z0Res += -150.45*pow(abstrketa, 3);
      z0Res += 100.37*pow(abstrketa, 4);
      z0Res += -36.24*pow(abstrketa, 5);
      z0Res += 6.66*pow(abstrketa, 6);
      z0Res += -0.48*pow(abstrketa, 7);
      z0Res += 3.04*exp(-0.5*pow((abstrketa-2.46)/0.21,2));
    }
  }

  if (debug) printf("z0Res = %f\n", z0Res);
  return z0Res;
}
