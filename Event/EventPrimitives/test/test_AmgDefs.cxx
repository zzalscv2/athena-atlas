/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file test_AmgDefs.cxx
 * @brief test for the Amg typedefs
 */

#include <iostream>

#include "EventPrimitives/EventPrimitives.h"

void dynTest() {
  using namespace Amg;
  std::cout << "----------------- " << '\n';
  MatrixX matrix(3, 3);
  matrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  std::cout << "rows/cols " << '\n';
  std::cout << matrix.rows() << '\n';
  std::cout << matrix.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << matrix << '\n';

  std::cout << "----------------- " << '\n';
  SymMatrixX symMatrix(3, 3);
  symMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  std::cout << "rows/cols " << '\n';
  std::cout << symMatrix.rows() << '\n';
  std::cout << symMatrix.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << symMatrix << '\n';

  std::cout << "----------------- " << '\n';
  VectorX vector(3);
  vector << 1, 2, 3;
  std::cout << "rows/cols " << '\n';
  std::cout << vector.rows() << '\n';
  std::cout << vector.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << vector << '\n';
}

void maxTest() {

  using namespace Amg;
  // These have capacity 5x5, or 5 but
  // size 3x3 or 3
  std::cout << "----------------- " << '\n';
  MatrixMaxX<5, 5> matrix(3, 3);
  matrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  std::cout << "rows/cols " << '\n';
  std::cout << matrix.rows() << '\n';
  std::cout << matrix.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << matrix << '\n';

  std::cout << "----------------- " << '\n';
  SymMatrixMaxX<5> symMatrix(3, 3);
  std::cout << "rows/cols " << '\n';
  symMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  std::cout << symMatrix.rows() << '\n';
  std::cout << symMatrix.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << symMatrix << '\n';

  std::cout << "----------------- " << '\n';
  VectorMaxX<5> vector(3);
  vector << 1, 2, 3;
  std::cout << "rows/cols " << '\n';
  std::cout << vector.rows() << '\n';
  std::cout << vector.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << vector << '\n';
}

void fixedTest() {

  std::cout << "----------------- " << '\n';
  AmgMatrix(3, 3) matrix;
  matrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  std::cout << "rows/cols " << '\n';
  std::cout << matrix.rows() << '\n';
  std::cout << matrix.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << matrix << '\n';

  std::cout << "----------------- " << '\n';
  AmgSymMatrix(3) symMatrix;
  symMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  std::cout << "rows/cols " << '\n';
  std::cout << symMatrix.rows() << '\n';
  std::cout << symMatrix.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << symMatrix << '\n';

  std::cout << "----------------- " << '\n';
  AmgVector(3) vector;
  vector << 1, 2, 3;
  std::cout << "rows/cols " << '\n';
  std::cout << vector.rows() << '\n';
  std::cout << vector.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << vector << '\n';

  std::cout << "----------------- " << '\n';
  AmgRowVector(3) rowVector;
  rowVector << 1, 2, 3;
  std::cout << "rows/cols " << '\n';
  std::cout << rowVector.rows() << '\n';
  std::cout << rowVector.cols() << '\n';
  std::cout << "elements: " << '\n';
  std::cout << rowVector << '\n';
}

int main() {
  dynTest();
  maxTest();
  fixedTest();
  return 0;
}
