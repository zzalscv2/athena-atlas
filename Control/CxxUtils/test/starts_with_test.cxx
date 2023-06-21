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


void testChar1()
{
  std::cout << "testChar1\n";

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

void testChar2()
{
  // separate out these edge cases because gcc warns about them.
  assert (!CxxUtils::starts_with ("abcdef", "abcdefg") );
  assert (!CxxUtils::starts_with ("", "abc") );
}

void testString1()
{
  std::cout << "testString1\n";

  assert(CxxUtils::starts_with (std::string("abcdef"), std::string("abc") ) );
  assert(!CxxUtils::starts_with(std::string("abcdef"), std::string("abd") ) );
  assert(CxxUtils::starts_with (std::string("abcdef"), std::string("abcdef") ) );
  assert(CxxUtils::starts_with (std::string("abcdef"), std::string("") ) );

  assert(CxxUtils::ends_with (std::string("abcdef"), std::string("def") ) );
  assert(!CxxUtils::ends_with (std::string("abcdef"), std::string("deg") ) );
  assert (CxxUtils::ends_with (std::string("abcdef"), std::string("abcdef") ) );
  assert (!CxxUtils::ends_with (std::string("abcdef"), std::string("abcdefg") ) );
  assert (CxxUtils::ends_with (std::string("abcdef"), std::string("") ) );
  assert (!CxxUtils::ends_with (std::string(""), std::string("def") ) );
}

void testString2()
{
  // separate out these edge cases because gcc warns about them.
  assert (!CxxUtils::starts_with (std::string("abcdef"), std::string("abcdefg")) );
  assert (!CxxUtils::starts_with (std::string(""), std::string("abc")) );
}

void testMixed1()
{
  std::cout << "testMixed1\n";

  assert (CxxUtils::starts_with (std::string("abcdef"), "abc") );
  assert (!CxxUtils::starts_with (std::string("abcdef"), "abd") );
  assert (CxxUtils::starts_with (std::string("abcdef"), "abcdef") );
  assert (CxxUtils::starts_with (std::string("abcdef"), "") );

  assert(CxxUtils::ends_with (std::string("abcdef"), "def") );
  assert (!CxxUtils::ends_with (std::string("abcdef"), "deg") );
  assert (CxxUtils::ends_with (std::string("abcdef"), "abcdef") );
  assert (!CxxUtils::ends_with (std::string("abcdef"), "abcdefg") );
  assert (CxxUtils::ends_with (std::string("abcdef"), "") );
  assert (!CxxUtils::ends_with (std::string(""), "def") );
}

void testMixed2()
{
  // separate out these edge cases because gcc warns about them.
  assert (!CxxUtils::starts_with (std::string("abcdef"), "abcdefg") );
  assert (!CxxUtils::starts_with (std::string(""), "abc") );
}



int main()
{
  std::cout << "CxxUtils/starts_with_test for both const char* inputs \n";
  testChar1();
  testChar2();
  std::cout << "CxxUtils/starts_with_test for both std::string inputs \n";
  testString1();
  testString2();
  std::cout << "CxxUtils/starts_with_test for mixed std::string const char* inputs \n";
  testMixed1();
  testMixed2();
  return 0;
}
