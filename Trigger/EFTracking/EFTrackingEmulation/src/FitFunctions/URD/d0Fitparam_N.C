// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
// 2D-function to describe the d0 resolution versus pT and eta. See README
// pt is in GeV

#include <iostream>
double getd0ResParam_N(float abstrketa, float trkpt, bool debug=false) {

  double d0Res = 0;
  if (trkpt>=1.0 && trkpt<1.5) {
    if (abstrketa<3.9) {
      d0Res += 48.72407280*std::pow(abstrketa, 0);
      d0Res += 11.70076933*std::pow(abstrketa, 1);
      d0Res += -35.38275844*std::pow(abstrketa, 2);
      d0Res += 70.21769320*std::pow(abstrketa, 3);
      d0Res += -43.85424708*std::pow(abstrketa, 4);
      d0Res += 11.73304174*std::pow(abstrketa, 5);
      d0Res += -1.13186654*std::pow(abstrketa, 6);
      d0Res += 21.84052455*exp(-0.5*std::pow((abstrketa-2.27356257)/0.22488828,2));
    }
  }

  if (trkpt>=1.5 && trkpt<2.5) {
    if (abstrketa<3.9) {
      d0Res += 32.55725363*std::pow(abstrketa, 0);
      d0Res += 0.09893172*std::pow(abstrketa, 1);
      d0Res += 6.75310046*std::pow(abstrketa, 2);
      d0Res += 1.34062131*std::pow(abstrketa, 3);
      d0Res += -0.40280069*std::pow(abstrketa, 4);
      d0Res += 7.64425495*exp(-0.5*std::pow((abstrketa-2.22125941)/0.15911675,2));
    }
  }

  if (trkpt>=2.5 && trkpt<5.0) {
    if (abstrketa<3.9) {
      d0Res += 19.10784492*std::pow(abstrketa, 0);
      d0Res += 0.63985912*std::pow(abstrketa, 1);
      d0Res += 3.49699665*std::pow(abstrketa, 2);
      d0Res += 7.64124327*exp(-0.5*std::pow((abstrketa-2.72410726)/0.55861409,2));
    }
  }

  if (trkpt>=5.0 && trkpt<10.0) {
    if (abstrketa<3.9) {
      d0Res += 11.38970015*std::pow(abstrketa, 0);
      d0Res += -2.13066894*std::pow(abstrketa, 1);
      d0Res += 10.72056708*std::pow(abstrketa, 2);
      d0Res += -13.07596283*std::pow(abstrketa, 3);
      d0Res += 8.31592265*std::pow(abstrketa, 4);
      d0Res += -2.25703331*std::pow(abstrketa, 5);
      d0Res += 0.21876101*std::pow(abstrketa, 6);
    }
  }

  if (trkpt>=10.0 && trkpt<20.0) {
    if (abstrketa<3.9) {
      d0Res += 7.27429507*std::pow(abstrketa, 0);
      d0Res += -0.24379687*std::pow(abstrketa, 1);
      d0Res += 3.60091383*std::pow(abstrketa, 2);
      d0Res += -5.43632881*std::pow(abstrketa, 3);
      d0Res += 3.91527620*std::pow(abstrketa, 4);
      d0Res += -1.13992206*std::pow(abstrketa, 5);
      d0Res += 0.11873396*std::pow(abstrketa, 6);
    }
  }

  if (trkpt>=20.0) {
    if (abstrketa<3.9) {
      d0Res += 4.36631419*std::pow(abstrketa, 0);
      d0Res += 1.27017785*std::pow(abstrketa, 1);
      d0Res += -2.15589494*std::pow(abstrketa, 2);
      d0Res += 0.79568803*std::pow(abstrketa, 3);
      d0Res += 0.43200853*std::pow(abstrketa, 4);
      d0Res += -0.28839985*std::pow(abstrketa, 5);
      d0Res += 0.04554335*std::pow(abstrketa, 6);
    }
  }

  if (debug) std::cout<<"d0Res = " << d0Res << std::endl;
  return d0Res;
}
