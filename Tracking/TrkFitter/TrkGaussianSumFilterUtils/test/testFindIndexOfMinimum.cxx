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

// create global data for the test
constexpr size_t N = 4096;
struct InitArray {
 public:
  InitArray() : distances(N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.001, 5.0);
    for (size_t i = 0; i < N; ++i) {
      distances[i] = dis(gen);
      // At the end of the "vectorized loops"
      //  36 will the 1st element of the second SIMD vec
      //  1024 will the 1st element of the first SIMD vec
      //  17 will the 2nd eleament of the fourth SIMD vec
      //  40 will the 1st eleament of the third SIMD vec
      if (i == 17 || i == 40 || i == 36 || i == 1024) {
        distances[i] = 0.0;
      }
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
  int32_t minIndex = findMinimumIndex::impl<findMinimumIndex::VecAlwaysTrackIdx>(
      initArray.distances.buffer(), N);
  std::cout << "VecAlwaysTrackIdx Minimum index : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

static void findMinimumIndexVecUnordered() {
  int32_t minIndex = findMinimumIndex::impl<findMinimumIndex::VecUpdateIdxOnNewMin>(
      initArray.distances.buffer(), N);
  std::cout << "VecUpdateIdxOnNewMin Minimum index : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

int main() {
  findMinimumIndexC();
  findMinimumIndexSTL();
  findMinimumIndexVecBlend();
  findMinimumIndexVecUnordered();
  return 0;
}
