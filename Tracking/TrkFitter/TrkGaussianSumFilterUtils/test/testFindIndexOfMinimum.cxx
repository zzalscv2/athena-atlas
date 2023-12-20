/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkGaussianSumFilterUtils/GSFFindIndexOfMimimum.h"
//
#include "TrkGaussianSumFilterUtils/AlignedDynArray.h"
//
#include <algorithm>
#include <iostream>
#include <random>

constexpr int alignment = 32;
// create global data for the test
constexpr size_t N = 4096;
struct InitArray {
 public:
  InitArray() : distances(N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.001, 10.0);
    for (size_t i = 0; i < N; ++i) {
      distances[i] = dis(gen);
    }
  }
  GSFUtils::AlignedDynArray<float, GSFConstants::alignment> distances;
};
static const InitArray initArray;

static void findMinimumIndexC() {
  int32_t minIndex = findMinimumIndex::impl<findMinimumIndex::C>(
      initArray.distances.buffer(), N);
  std::cout << "C Minimum index : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

static void findMinimumIndexSTL() {
  int32_t minIndex = findMinimumIndex::impl<findMinimumIndex::STL>(
      initArray.distances.buffer(), N);
  std::cout << "STL Minimum index : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

static void findMinimumIndexVecBlend() {
  int32_t minIndex = findMinimumIndex::impl<findMinimumIndex::VecBlend>(
      initArray.distances.buffer(), N);
  std::cout << "Vec Minimum index Blend : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

static void findMinimumIndexVecUnordered() {
  int32_t minIndex = findMinimumIndex::impl<findMinimumIndex::VecUnordered>(
      initArray.distances.buffer(), N);
  std::cout << "Vec Minimum index Unordered : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

int main() {
  findMinimumIndexC();
  findMinimumIndexSTL();
  findMinimumIndexVecBlend();
  findMinimumIndexVecUnordered();
  return 0;
}
