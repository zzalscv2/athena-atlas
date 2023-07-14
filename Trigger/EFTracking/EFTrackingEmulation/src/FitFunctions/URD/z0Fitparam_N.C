// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
// 2D-function to describe the z0 resolution versus pT and eta. See README

#include <iostream>
double getz0ResParam_N(float abstrketa, float trkpt, bool debug=false) {

  double z0Res = 0;
  if (trkpt>=1.0 && trkpt<1.5) {
    if (abstrketa<3.9) {
      z0Res += 71.65972485*std::pow(abstrketa, 0);
      z0Res += -26.19370915*std::pow(abstrketa, 1);
      z0Res += 40.37999526*std::pow(abstrketa, 2);
      z0Res += 17.50951981*std::pow(abstrketa, 3);
      z0Res += 1.23154903*std::pow(abstrketa, 4);
      z0Res += -1.44168366*std::pow(abstrketa, 5);
      z0Res += 0.92505577*std::pow(abstrketa, 6);
      z0Res += 42.92425474*exp(-0.5*std::pow((abstrketa-1.88701704)/0.04050632,2));
      z0Res += 162.68556467*exp(-0.5*std::pow((abstrketa-2.51822689)/0.2410260099805757727509103461,2));
    }
  }

  if (trkpt>=1.5 && trkpt<2.5) {
    if (abstrketa<3.9) {
      z0Res += 63.04626058*std::pow(abstrketa, 0);
      z0Res += -49.33952475*std::pow(abstrketa, 1);
      z0Res += 36.09831414*std::pow(abstrketa, 2);
      z0Res += 35.03400452*std::pow(abstrketa, 3);
      z0Res += -20.42236907*std::pow(abstrketa, 4);
      z0Res += 5.26113117*std::pow(abstrketa, 5);
      z0Res += 102.40355697*exp(-0.5*std::pow((abstrketa-2.53436441)/0.26300171,2));
    }
  }

  if (trkpt>=2.5 && trkpt<5.0) {
    if (abstrketa<3.9) {
      z0Res += 45.18314991*std::pow(abstrketa, 0);
      z0Res += -48.88127799*std::pow(abstrketa, 1);
      z0Res += 47.49106974*std::pow(abstrketa, 2);
      z0Res += 4.57155872*std::pow(abstrketa, 3);
      z0Res += -8.25716340*std::pow(abstrketa, 4);
      z0Res += 2.78210384*std::pow(abstrketa, 5);
      z0Res += 57.90881664*exp(-0.5*std::pow((abstrketa-2.53926139)/0.27623007,2));
    }
  }

  if (trkpt>=5.0 && trkpt<10.0) {
    if (abstrketa<3.9) {
      z0Res += 30.77411554*std::pow(abstrketa, 0);
      z0Res += -63.05190452*std::pow(abstrketa, 1);
      z0Res += 134.03156883*std::pow(abstrketa, 2);
      z0Res += -143.12115765*std::pow(abstrketa, 3);
      z0Res += 54.01517556*std::pow(abstrketa, 4);
      z0Res += -5.61428747*std::pow(abstrketa, 5);
      z0Res += 76.51135558*exp(-0.5*std::pow((abstrketa-1.94105642)/0.57056972,2));
      z0Res += 75.84135786*exp(-0.5*std::pow((abstrketa-2.58794738)/0.3647087643607762608155553608,2));
    }
  }

  if (trkpt>=10.0 && trkpt<20.0) {
    if (abstrketa<3.9) {
      z0Res += 19.39469788*std::pow(abstrketa, 0);
      z0Res += -22.51493172*std::pow(abstrketa, 1);
      z0Res += 31.92377013*std::pow(abstrketa, 2);
      z0Res += -11.17317474*std::pow(abstrketa, 3);
      z0Res += 0.66337736*std::pow(abstrketa, 4);
      z0Res += 0.58619897*std::pow(abstrketa, 5);
      z0Res += 14.43206857*exp(-0.5*std::pow((abstrketa-2.50442429)/0.32063788,2));
    }
  }

  if (trkpt>=20.0) {
    if (abstrketa<3.9) {
      z0Res += 2.92339716*std::pow(abstrketa, 0);
      z0Res += -34.55228105*std::pow(abstrketa, 1);
      z0Res += 12.87887345*std::pow(abstrketa, 2);
      z0Res += -2.20912261*std::pow(abstrketa, 3);
      z0Res += 1.05950066*std::pow(abstrketa, 4);
      z0Res += 29.46376030*exp(-0.5*std::pow((abstrketa-1.09960118)/0.71728841,2));
      z0Res += 28.39481084*exp(-0.5*std::pow((abstrketa-2.24335984)/0.6197033253331861724078066800,2));
    }
  }

  if (debug) std::cout<<"z0Res = " << z0Res << std::endl;
  return z0Res;
}
