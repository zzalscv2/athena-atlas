
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// since we are testing an "internal" /implementation method of the .cxx
#include "src/MultiComponentStateModeCalculator.cxx"
#include <iostream>

int
main()
{

  VecOfComponents vec0 = {
      {0.973657, -27.9079, 0.0538001},    {0.024202, -27.9023, 0.0580491},
      {0.00146762, -27.9063, 0.0594736},  {0.000452399, -27.8908, 0.0591268},
      {0.000152378, -27.8987, 0.0604233}, {5.65349e-05, -27.9603, 0.0517557},
      {5.81047e-06, -27.9495, 0.0553466}, {5.21863e-06, -27.9006, 0.0603241},
      {6.61547e-07, -27.9117, 0.0626188}, {1.19371e-07, -27.8984, 0.060867},
      {1.11035e-07, -27.9323, 0.0585395}};

  VecOfComponents vec1 = {
      {0.973657, 38.4958, 0.757618},    {0.024202, 38.4292, 0.801337},
      {0.00146762, 38.4686, 0.814726},  {0.000452399, 38.285, 0.811128},
      {0.000152378, 38.382, 0.826285},  {5.65349e-05, 39.137, 0.728419},
      {5.81047e-06, 39.0091, 0.763217}, {5.21863e-06, 38.3888, 0.824838},
      {6.61547e-07, 38.5323, 0.858115}, {1.19371e-07, 38.2937, 0.834079},
      {1.11035e-07, 38.7643, 0.802247}};

  VecOfComponents vec2 = {
      {0.973657, -1.0267, 0.000445822},    {0.024202, -1.02727, 0.000738283},
      {0.00146762, -1.03185, 0.000786109}, {0.000452399, -1.02815, 0.000729393},
      {0.000152378, -1.03015, 0.00159626}, {5.65349e-05, -1.03469, 0.000698825},
      {5.81047e-06, -1.04587, 0.00150398}, {5.21863e-06, -1.03862, 0.00246934},
      {6.61547e-07, -1.05081, 0.00301325}, {1.19371e-07, -1.08593, 0.0072159},
      {1.11035e-07, -1.07377, 0.0038744}};

  VecOfComponents vec3 = {
      {0.973657, 2.94981, 0.00113228},    {0.024202, 2.94985, 0.00120513},
      {0.00146762, 2.94994, 0.00120203},  {0.000452399, 2.95008, 0.00122586},
      {0.000152378, 2.94981, 0.00140487}, {5.65349e-05, 2.94921, 0.00108731},
      {5.81047e-06, 2.94948, 0.00113324}, {5.21863e-06, 2.95005, 0.00141219},
      {6.61547e-07, 2.9482, 0.00391511},  {1.19371e-07, 2.94952, 0.00241575},
      {1.11035e-07, 2.94963, 0.0014292}};

  VecOfComponents vec4 = {{0.973657, -7.99235e-05, 1.2352e-05},
                          {0.024202, -0.000165865, 6.85227e-05},
                          {0.00146762, -0.000731881, 4.43473e-05},
                          {0.000452399, -0.000297975, 2.98129e-05},
                          {0.000152378, -0.000534431, 0.00017548},
                          {5.65349e-05, -0.000996226, 5.45339e-05},
                          {5.81047e-06, -0.00246959, 0.00014759},
                          {5.21863e-06, -0.0015881, 0.000274361},
                          {6.61547e-07, -0.00321585, 0.000282572},
                          {1.19371e-07, -0.00750016, 0.000670315},
                          {1.11035e-07, -0.0059806, 0.000365448}};

  std::array<VecOfComponents, 5> mixture = {
    vec0, vec1, vec2, vec3, vec4
  };

  std::array<double, 10> result = evaluateMode(mixture);
  for (double i : result) {
    std::cout << i << '\n';
  }

  return 0;
}
