/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/releasing_iterator_test.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Unit tests for releasing_iterator.
 */


#undef NDEBUG
#include "CxxUtils/releasing_iterator.h"
#include <memory>
#include <vector>
#include <iostream>
#include <cassert>


void test1()
{
  std::cout << "test1\n";

  std::vector<std::unique_ptr<int> > v1;
  for (int i = 0; i < 10; i++) {
    v1.emplace_back (std::make_unique<int>(i));
  }

  using CxxUtils::releasing_iterator;
  std::vector<int* > v2 (releasing_iterator(v1.begin()),
                         releasing_iterator(v1.end()));

  assert (v1.size() == 10);
  assert (v2.size() == 10);
  for (int i = 0; i < 10; i++) {
    assert (!v1[i]);
    assert (*v2[i] == i);
    delete v2[i];
  }
}


int main()
{
  std::cout << "CxxUtils/releasing_iterator\n";
  test1();
  return 0;
}

