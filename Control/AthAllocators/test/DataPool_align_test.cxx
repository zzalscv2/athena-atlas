/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include <cassert>
#include <iostream>

#include "AthAllocators/ArenaBlockAlignDetail.h"
#include "AthAllocators/DataPool.h"

using namespace std;

// Basic tests
void test() {

  std::cout << "test\n";

  // we should always be fine for this alignment
  using testStruct = SG::ArenaBlockAlignDetail::padForAlign;
  size_t struct_alignment = alignof(testStruct);

  auto g = std::make_unique<testStruct>();
  std::uintptr_t stdPtr = (uintptr_t)(g.get());
  std::cout << "Std ptr properly aligned " << (stdPtr % struct_alignment == 0)
            << '\n';
  assert(stdPtr % struct_alignment == 0);

  auto df = std::make_unique<DataPool<testStruct>>(10);
  testStruct* f = df->nextElementPtr();
  std::uintptr_t poolPtr = (uintptr_t)f;
  std::cout << "Pool ptr properly aligned " << (poolPtr % struct_alignment == 0)
            << '\n';
  assert(poolPtr % struct_alignment == 0);
}

int main() {
  test();
  return 0;
}
