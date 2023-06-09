/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file test_eta.cxx
 * @brief test for EventPrimitivesHelpers
 */

#include <iostream>
#include "EventPrimitives/EventPrimitivesHelpers.h"

int main() {

  AmgSymMatrix(5) mat;
  mat << 36.0000, 0.0000, -0.0222, -0.0001, 0.0000,
         0.0000, 3600.0000, -1.5939, -0.0164, 0.0000,
        -0.0184, -1.2931, -6230751.9604, -290277.8826, -0.5211,
         0.0001, -0.0023, -290277.8826, -13523.4478, -0.0243,
         0.0000, 0.0000, -0.5211, -0.0243, 0.0000;

  std::cout <<Amg::saneCovarianceDiagonal(mat)<<'\n';


  Amg::MatrixX dynMat(5,5);
  dynMat << 36.0000, 0.0000, -0.0222, -0.0001, 0.0000,
            0.0000, 3600.0000, -1.5939, -0.0164, 0.0000,
           -0.0184, -1.2931, -6230751.9604, -290277.8826, -0.5211,
            0.0001, -0.0023, -290277.8826, -13523.4478, -0.0243,
            0.0000, 0.0000, -0.5211, -0.0243, 0.0000;

  std::cout <<Amg::saneCovarianceDiagonal(dynMat)<<'\n';

}
