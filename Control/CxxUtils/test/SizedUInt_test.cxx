/*
  Copyright (C) 200223 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/SizedUInt.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Regression tests for SizedUInt
 */


#undef NDEBUG

#include "CxxUtils/SizedUInt.h"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <type_traits>


void test1()
{
  std::cout << "test1\n";
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<1>::type, uint8_t>));
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<2>::type, uint16_t>));
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<4>::type, uint32_t>));
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<8>::type, uint64_t>));
}


int main()
{
  std::cout << "CxxUtils/SizedUint_test\n";
  test1();
  return 0;
}
