/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/normalizeFunctionName.cxx
 * @author scott snyder
 * @date Sep 2023
 * @brief Regression tests for normalizeFunctionName.
 */


#undef NDEBUG
#include "CxxUtils/normalizeFunctionName.h"
#include <mutex>
#include <iostream>
#include <cassert>


void test1()
{
  std::cout << "test1\n";
#define CHECK(a, b) assert(CxxUtils::normalizeFunctionName(a)==b)
  CHECK( "void FPGATrackSimPlaneMap::map(FPGATrackSimHit &)",
         "void FPGATrackSimPlaneMap::map(FPGATrackSimHit&)" );
  CHECK( "void test2(std::vector<int, std::allocator<int> >, const int*, int (*)(), int*)",
         "void test2(std::vector<int>, const int*, int (*)(), int*)" );
  CHECK( "void foo(std::basic_string<char, std:allocator<char> >)",
         "void foo(std::string)" );
  CHECK( "void foo(CLID)",
         "void foo(unsigned int)" );
}


int main()
{
  std::cout << "CxxUtils/normalizeFunctionName_test\n";
  test1();
  return 0;
}
