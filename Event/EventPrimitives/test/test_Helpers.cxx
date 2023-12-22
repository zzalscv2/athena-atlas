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

  std::cout<<"------------------"<<'\n';
  AmgSymMatrix(5) mat;
  mat << 36.0000, 0.0000, -0.0222, -0.0001, 0.0000,
         0.0000, 3600.0000, -1.5939, -0.0164, 0.0000,
        -0.0184, -1.2931, -6230751.9604, -290277.8826, -0.5211,
         0.0001, -0.0023, -290277.8826, -13523.4478, -0.0243,
         0.0000, 0.0000, -0.5211, -0.0243, 0.0000;
  std::cout <<Amg::hasPositiveOrZeroDiagElems(mat)<<'\n';
  std::cout <<Amg::hasPositiveDiagElems(mat)<<'\n';

  std::cout<<"------------------"<<'\n';
  Amg::MatrixX dynMat(5,5);
  dynMat << 36.0000, 0.0000, -0.0222, -0.0001, 0.0000,
            0.0000, 3600.0000, -1.5939, -0.0164, 0.0000,
            -0.0184, -1.2931, -6230751.9604, -290277.8826, -0.5211,
            0.0001, -0.0023, -290277.8826, -13523.4478, -0.0243,
            0.0000, 0.0000, -0.5211, -0.0243, 0.0000;
  std::cout <<Amg::hasPositiveOrZeroDiagElems(dynMat)<<'\n';
  std::cout <<Amg::hasPositiveDiagElems(dynMat)<<'\n';


  std::cout<<"------------------"<<'\n';
  AmgSymMatrix(5) mat1;
  mat1 << 50977.455023, -3154.699348, 191.699597, -3.127933, 0.000037,
          -3154.699348, 50043.753058, -10.483807, -29.006012, -0.001595,
          191.699597, -10.483807, 0.724398, -0.012684, 0.000000,
          -3.127933,  -29.006012, -0.012684, 0.021047, 0.000003,
          0.000037, -0.001595, 0.000000, 0.000003, 1e-12;
  std::cout<<Amg::hasPositiveOrZeroDiagElems(mat1)<<'\n';
  std::cout <<Amg::hasPositiveDiagElems(mat1)<<'\n';


  std::cout<<"------------------"<<'\n';
  AmgSymMatrix(5) mat0;
  mat0.setZero();
  std::cout <<Amg::hasPositiveOrZeroDiagElems(mat0)<<'\n';
  std::cout <<Amg::hasPositiveDiagElems(mat0)<<'\n';



}
