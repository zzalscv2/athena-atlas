/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/SizedUInt.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Regression tests for UIntConv
 */


#undef NDEBUG

#include "CxxUtils/UIntConv.h"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <type_traits>


template <class T>
void test1a (T x)
{
  uintptr_t y = CxxUtils::detail::UIntConv<T>::valToUInt (x);
  assert (CxxUtils::detail::UIntConv<T>::uintToVal (y) == x);
}
void test1()
{
  std::cout << "test1\n";

  test1a<uintptr_t> (123456789);
  test1a<unsigned long> (123456789);
  test1a<int> (-23456789);
  test1a<char> ('y');
  test1a<float> (123.45);
  test1a<double> (123.45e40);

  int x = 0;
  test1a<int*> (&x);
}


int main()
{
  std::cout << "CxxUtils/UintConv_test\n";
  test1();
  return 0;
}
