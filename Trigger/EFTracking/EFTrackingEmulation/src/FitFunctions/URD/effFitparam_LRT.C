// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
// 2D-function to describe the tracking efficiency versus pT and eta. See README
//pt is in GeV

#include <cmath>
#include <iostream>
double getEffParam_LRT( double absd0, double lowd0_cut, double highd0_cut, bool debug=false) {

  double eff = 1.;

  // 100%, 80% and 20%
  
  // step function example (commented out for now)
  /*
  if (absd0 > lowd0_cut && absd0 <= highd0_cut) eff = 0.8;
  else if (absd0 > highd0_cut) eff = 0.2;
  */
  
  // can be also: double inverted Fermi can do the job?
  
  eff= (0.2/(1. + std::exp(0.5*(absd0-lowd0_cut))) + 0.6/(1. + std::exp(0.05*(absd0-highd0_cut))) + 0.2);

  if (debug) printf("eff = %f\n", eff);
  return eff;
}
