/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file test_eta.cxx
 * @brief test for vector3D eta calculation
 */

#include <iostream>
#include "EventPrimitives/EventPrimitives.h"

int main() {

  AmgVector(3) vec;
  vec << 0, 0, 0;
  std::cout << vec.eta() << '\n';

  vec << 0, 0, 1;
  std::cout << vec.eta() << '\n';

  vec << 0, 1, 1;
  std::cout << vec.eta() << '\n';

  vec << 1, 1, 1;
  std::cout << vec.eta() << '\n';

  vec << 1.02848e-05, -2.52655e-05, -7122.83;
  std::cout << vec.eta() << '\n';

  std::cout << "-----" << '\n';
  Eigen::Matrix<double, 3, 1> vec1;
  vec1 << 0, 0, 0;
  std::cout << vec1.eta() << '\n';

  vec1 << 2, 1, 3;
  std::cout << vec1.eta() << '\n';
}
