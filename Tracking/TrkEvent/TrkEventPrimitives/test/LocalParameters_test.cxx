/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Christos Anastopoulos
 * @brief Some tests for LocalParameters
 */

#undef NDEBUG
#include "TrkEventPrimitives/LocalParameters.h"

#include <iostream>

void test1() {
  std::cout << "Size 1 Local Parameters" << '\n';
  std::cout << "------> d0" << '\n';
  Trk::LocalParameters d0(Trk::DefinedParameter{1, Trk::d0});
  std::cout << "Dimension " << d0.dimension() << '\n';
  std::cout << "ParameterKey " << d0.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << d0.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << d0.expansionMatrix() << '\n';
  std::cout << "AsVector: " << d0.asVector() << '\n';

  std::cout << "------> z0" << '\n';
  Trk::LocalParameters z0(Trk::DefinedParameter{2, Trk::z0});
  std::cout << "Dimension " << z0.dimension() << '\n';
  std::cout << "ParameterKey " << z0.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << z0.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << z0.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << z0.asVector() << '\n';

  std::cout << "------> phi0" << '\n';
  Trk::LocalParameters phi0(Trk::DefinedParameter{3, Trk::phi0});
  std::cout << "Dimension " << phi0.dimension() << '\n';
  std::cout << "ParameterKey " << phi0.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << phi0.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << phi0.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << phi0.asVector() << '\n';

  std::cout << "------> theta" << '\n';
  Trk::LocalParameters theta(Trk::DefinedParameter{4, Trk::theta});
  std::cout << "Dimension " << theta.dimension() << '\n';
  std::cout << "ParameterKey " << theta.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << theta.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << theta.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << theta.asVector() << '\n';

  std::cout << "------> qOverP" << '\n';
  Trk::LocalParameters qOverP(Trk::DefinedParameter{5, Trk::qOverP});
  std::cout << "Dimension " << qOverP.dimension() << '\n';
  std::cout << "ParameterKey " << qOverP.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << qOverP.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << qOverP.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << qOverP.asVector() << '\n';
}

void test2() {
  std::cout << "Size 2 Local Parameters" << '\n';
  std::cout << "------> position" << '\n';
  Trk::LocalParameters pos(Amg::Vector2D{1, 2});
  std::cout << "Dimension " << pos.dimension() << '\n';
  std::cout << "ParameterKey " << pos.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << pos.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << pos.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << pos.asVector() << '\n';

  std::cout << "------> phi0, theta" << '\n';
  Trk::LocalParameters phitheta(Trk::DefinedParameter{3, Trk::phi0},
                                Trk::DefinedParameter{4, Trk::theta});
  std::cout << "Dimension " << phitheta.dimension() << '\n';
  std::cout << "ParameterKey " << phitheta.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n'
            << phitheta.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n'
            << phitheta.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << phitheta.asVector() << '\n';
}

void test3() {
  std::cout << "From size 3 array" << '\n';
  std::array<Trk::DefinedParameter, 3> tmp{Trk::DefinedParameter{1, Trk::d0},
                                           Trk::DefinedParameter{2, Trk::z0},
                                           Trk::DefinedParameter{3, Trk::phi0}};
  std::cout << "------> 3D" << '\n';
  Trk::LocalParameters dim3(tmp);
  std::cout << "Dimension " << dim3.dimension() << '\n';
  std::cout << "ParameterKey " << dim3.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << dim3.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << dim3.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << dim3.asVector() << '\n';
}

void test4() {
  std::cout << "From size 4 array" << '\n';
  std::array<Trk::DefinedParameter, 4> tmp{
      Trk::DefinedParameter{1, Trk::d0}, Trk::DefinedParameter{2, Trk::z0},
      Trk::DefinedParameter{3, Trk::phi0},
      Trk::DefinedParameter{4, Trk::theta}};
  std::cout << "------> 4D" << '\n';
  Trk::LocalParameters dim4(tmp);
  std::cout << "Dimension " << dim4.dimension() << '\n';
  std::cout << "ParameterKey " << dim4.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << dim4.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << dim4.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << dim4.asVector() << '\n';
}

void test5() {
  std::cout << "------> 5D" << '\n';
  Trk::LocalParameters dim5(1, 2, 3, 4, 5);
  std::cout << "Dimension " << dim5.dimension() << '\n';
  std::cout << "ParameterKey " << dim5.parameterKey() << '\n';
  std::cout << "Reduction Matrix " << '\n' << dim5.reductionMatrix() << '\n';
  std::cout << "Expansion Matrix " << '\n' << dim5.expansionMatrix() << '\n';
  std::cout << "AsVector: " << '\n' << dim5.asVector() << '\n';
}

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
  return 0;
}

