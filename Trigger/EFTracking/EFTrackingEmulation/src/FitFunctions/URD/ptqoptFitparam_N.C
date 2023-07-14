// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
// 2D-function to describe the ptqopt resolution versus pT and eta. See README

#include <iostream>
double getptqoptResParam_N(float abstrketa, float trkpt, bool debug=false) {

  double ptqoptRes = 0;
  if (trkpt>=1.0 && trkpt<1.5) {
    if (abstrketa<3.9) {
      ptqoptRes += 0.01093407*std::pow(abstrketa, 0);
      ptqoptRes += -0.00431876*std::pow(abstrketa, 1);
      ptqoptRes += 0.01111538*std::pow(abstrketa, 2);
      ptqoptRes += -0.00442307*std::pow(abstrketa, 3);
      ptqoptRes += 0.00173122*std::pow(abstrketa, 4);
      ptqoptRes += 0.01098564*exp(-0.5*std::pow((abstrketa-2.77940862)/0.15000000,2));
    }
  }

  if (trkpt>=1.5 && trkpt<2.5) {
    if (abstrketa<3.9) {
      ptqoptRes += 0.01101489*std::pow(abstrketa, 0);
      ptqoptRes += -0.00448591*std::pow(abstrketa, 1);
      ptqoptRes += 0.01165447*std::pow(abstrketa, 2);
      ptqoptRes += -0.00493889*std::pow(abstrketa, 3);
      ptqoptRes += 0.00188055*std::pow(abstrketa, 4);
      ptqoptRes += 0.01124036*exp(-0.5*std::pow((abstrketa-2.79870700)/0.15000000,2));
    }
  }

  if (trkpt>=2.5 && trkpt<5.0) {
    if (abstrketa<3.9) {
      ptqoptRes += 0.01020762*std::pow(abstrketa, 0);
      ptqoptRes += 0.00001977*std::pow(abstrketa, 1);
      ptqoptRes += -0.00163216*std::pow(abstrketa, 2);
      ptqoptRes += -0.00010910*std::pow(abstrketa, 3);
      ptqoptRes += 0.00033767*std::pow(abstrketa, 4);
      ptqoptRes += 0.00030012*std::pow(abstrketa, 5);
      ptqoptRes += 0.00513584*exp(-0.5*std::pow((abstrketa-1.19486674)/0.07530451,2));
      ptqoptRes += 0.03498376*exp(-0.5*std::pow((abstrketa-2.92659416)/0.9870140614351902152989737260,2));
    }
  }

  if (trkpt>=5.0 && trkpt<10.0) {
    if (abstrketa<3.9) {
      ptqoptRes += 0.01081659*std::pow(abstrketa, 0);
      ptqoptRes += 0.00122081*std::pow(abstrketa, 1);
      ptqoptRes += 0.00217843*std::pow(abstrketa, 2);
      ptqoptRes += -0.00092976*std::pow(abstrketa, 3);
      ptqoptRes += 0.00159232*std::pow(abstrketa, 4);
      ptqoptRes += 0.00390577*exp(-0.5*std::pow((abstrketa-1.20254029)/0.08392621,2));
      ptqoptRes += 0.00089119*exp(-0.5*std::pow((abstrketa-2.92315389)/0.9530567193602196285340255599,2));
    }
  }

  if (trkpt>=10.0 && trkpt<20.0) {
    if (abstrketa<3.9) {
      ptqoptRes += 0.01168864*std::pow(abstrketa, 0);
      ptqoptRes += 0.00362780*std::pow(abstrketa, 1);
      ptqoptRes += -0.01770760*std::pow(abstrketa, 2);
      ptqoptRes += 0.03796665*std::pow(abstrketa, 3);
      ptqoptRes += -0.02638396*std::pow(abstrketa, 4);
      ptqoptRes += 0.00819620*std::pow(abstrketa, 5);
      ptqoptRes += -0.00080399*std::pow(abstrketa, 6);
      ptqoptRes += 0.04035099*exp(-0.5*std::pow((abstrketa-3.00000000)/0.15138388,2));
    }
  }

  if (trkpt>=20.0) {
    if (abstrketa<3.9) {
      ptqoptRes += -0.01411704*std::pow(abstrketa, 0);
      ptqoptRes += -0.05067335*std::pow(abstrketa, 1);
      ptqoptRes += -0.09047611*std::pow(abstrketa, 2);
      ptqoptRes += 0.04366047*std::pow(abstrketa, 3);
      ptqoptRes += 0.17898237*exp(-0.5*std::pow((abstrketa-1.67732024)/0.88188119,2));
      ptqoptRes += 0.01168753*exp(-0.5*std::pow((abstrketa-2.13599594)/0.2136018417597442820010655851,2));
    }
  }

  if (debug) std::cout<<"ptqoptRes = " << ptqoptRes << std::endl;
  return ptqoptRes;
}
