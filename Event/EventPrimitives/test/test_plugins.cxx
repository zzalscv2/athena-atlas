/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file test_plugins.cxx
 * @brief test for vector3D methods
 * from the relevant plugins
 */

#include <iostream>

#include "EventPrimitives/EventPrimitives.h"

int main() {
  AmgVector(3) vec;
  vec << 10., 15.5, 4.5;
  std::cout <<std::scientific;
  std::cout << " mag: " << vec.mag() << '\n';
  std::cout << " mag2: " << vec.mag2() << '\n';
  std::cout << " perp: " << vec.perp() << '\n';
  std::cout << " perp2: " << vec.perp2() << '\n';
  std::cout << " phi: " << vec.phi() << '\n';
  std::cout << " theta: " << vec.theta() << '\n';
  std::cout << " eta: " << vec.eta() << '\n';
  return 0;
}
