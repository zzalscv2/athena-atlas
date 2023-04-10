/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/starts_with_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2023
 * @brief Unit test for starts_with.
 */

#undef NDEBUG

// gcc warns about some of the edge cases we test.
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wstring-compare"
#endif


#include "CxxUtils/starts_with.h"
#include <iostream>
#include <string>
#include <cassert>


void test1()
{
  std::cout << "test1\n";

  assert (CxxUtils::starts_with ("abcdef", "abc") );
  assert (!CxxUtils::starts_with ("abcdef", "abd") );
  assert (CxxUtils::starts_with ("abcdef", "abcdef") );
  assert (CxxUtils::starts_with ("abcdef", "") );

  assert (CxxUtils::ends_with ("abcdef", "def") );
  assert (!CxxUtils::ends_with ("abcdef", "deg") );
  assert (CxxUtils::ends_with ("abcdef", "abcdef") );
  assert (!CxxUtils::ends_with ("abcdef", "abcdefg") );
  assert (CxxUtils::ends_with ("abcdef", "") );
  assert (!CxxUtils::ends_with ("", "def") );
}


void test2()
{
  // separate out these edge cases because gcc warns about them.
  assert (!CxxUtils::starts_with ("abcdef", "abcdefg") );
  assert (!CxxUtils::starts_with ("", "abc") );
}


int main()
{
  std::cout << "CxxUtils/starts_with_test\n";
  test1();
  test2();
  return 0;
}
