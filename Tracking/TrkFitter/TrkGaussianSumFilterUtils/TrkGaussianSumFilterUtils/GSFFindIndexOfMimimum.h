/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  GSFFindIndexOfMimimum
 * @author Christos Anastopoulos
 *
 *
 * @brief Finding the index of a minimum
 * is an importand operation for the
 * the KL mixture reduction.
 * We rely on having an as fast as
 * possible implementation
 *
 * The issues are described in ATLASRECTS-5244
 *
 * There is literature in the internet
 * namely in blogs by Wojciech Mula
 * and Sergey Slotin
 * They solve the problem for
 * integers using intrinsics and various
 * AVX levels
 *
 * We need to solve it for float.
 * Furthermore, after discussion with Scott Snyder
 * we opted for using the gnu vector types.
 * And we target x86_64-v2.
 *
 * For completeness and future comparisons
 * we collect
 *
 * - A "C" implementation
 * - A "STL" implementation
 * - A "Vec"  implementation always tracking the index
 * - A "Vec" implementation that updates the index when an new minimum is
 * foiund. This can be faster than the above when the inputs are not ordered.
 *
 *  We provide a convenient entry method
 *  to select in compile time an implementation
 *
 *  The original playground with benchmarking code
 *  can be found at:
 *  https://github.com/AnChristos/FindMinimumIndex
 */

#ifndef GSFFindIndexOfMimimum_H
#define GSFFindIndexOfMimimum_H
#include "CxxUtils/assume_aligned.h"
#include "CxxUtils/inline_hints.h"
#include "CxxUtils/restrict.h"
#include "CxxUtils/vec.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
//
#include <algorithm>
namespace findMinimumIndexDetail {

// index of minimum scalar
ATH_ALWAYS_INLINE
int32_t scalarC(const float* distancesIn, int n) {
  const float* array =
      CxxUtils::assume_aligned<GSFConstants::alignment>(distancesIn);
  float minvalue = array[0];
  int minIndex = 0;
  for (int i = 0; i < n; ++i) {
    const float value = array[i];
    if (value < minvalue) {
      minvalue = value;
      minIndex = i;
    }
  }
  return minIndex;
}

// index of minimum STL
ATH_ALWAYS_INLINE
int32_t scalarSTL(const float* distancesIn, int n) {
  const float* array =
      CxxUtils::assume_aligned<GSFConstants::alignment>(distancesIn);
  return std::distance(array, std::min_element(array, array + n));
}

// index of minimum vec with blend
ATH_ALWAYS_INLINE
int32_t vecAlwaysTrackIdx(const float* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      CxxUtils::assume_aligned<GSFConstants::alignment>(distancesIn);
  const vec<int, 4> increment = {16, 16, 16, 16};

  vec<int, 4> indices1 = {0, 1, 2, 3};
  vec<int, 4> indices2 = {4, 5, 6, 7};
  vec<int, 4> indices3 = {8, 9, 10, 11};
  vec<int, 4> indices4 = {12, 13, 14, 15};

  vec<int, 4> minIndices1 = indices1;
  vec<int, 4> minIndices2 = indices2;
  vec<int, 4> minIndices3 = indices3;
  vec<int, 4> minIndices4 = indices4;

  vec<float, 4> minValues1;
  vec<float, 4> minValues2;
  vec<float, 4> minValues3;
  vec<float, 4> minValues4;
  vload(minValues1, array);
  vload(minValues2, array + 4);
  vload(minValues3, array + 8);
  vload(minValues4, array + 12);

  vec<float, 4> values1;
  vec<float, 4> values2;
  vec<float, 4> values3;
  vec<float, 4> values4;
  for (int i = 16; i < n; i += 16) {
    // 1
    vload(values1, array + i);  // 0-3
    indices1 = indices1 + increment;
    vec<int, 4> lt1 = values1 < minValues1;
    vselect(minIndices1, indices1, minIndices1, lt1);
    vmin(minValues1, values1, minValues1);
    // 2
    vload(values2, array + i + 4);  // 4-7
    indices2 = indices2 + increment;
    vec<int, 4> lt2 = values2 < minValues2;
    vselect(minIndices2, indices2, minIndices2, lt2);
    vmin(minValues2, values2, minValues2);
    // 3
    vload(values3, array + i + 8);  // 8-11
    indices3 = indices3 + increment;
    vec<int, 4> lt3 = values3 < minValues3;
    vselect(minIndices3, indices3, minIndices3, lt3);
    vmin(minValues3, values3, minValues3);
    // 4
    vload(values4, array + i + 12);  // 12-15
    indices4 = indices4 + increment;
    vec<int, 4> lt4 = values4 < minValues4;
    vselect(minIndices4, indices4, minIndices4, lt4);
    vmin(minValues4, values4, minValues4);
  }

  float minValues[16];
  int minIndices[16];
  vstore(minValues, minValues1);
  vstore(minValues + 4, minValues2);
  vstore(minValues + 8, minValues3);
  vstore(minValues + 12, minValues4);
  vstore(minIndices, minIndices1);
  vstore(minIndices + 4, minIndices2);
  vstore(minIndices + 8, minIndices3);
  vstore(minIndices + 12, minIndices4);

  float minValue = minValues[0];
  int32_t minIndex = minIndices[0];
  for (size_t i = 1; i < 16; ++i) {
    const float value = minValues[i];
    const int32_t index = minIndices[i];
    if (value < minValue) {
      minValue = value;
      minIndex = index;
    } else if (value == minValue && index < minIndex) {
      // we want to return the smallest index
      // in case of 2 same values
      minIndex = index;
    }
  }
  return minIndex;
}

// index of minimum vec quite fast for the "unordered"/"random" case
ATH_ALWAYS_INLINE
int32_t vecUpdateIdxOnNewMin(const float* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      CxxUtils::assume_aligned<GSFConstants::alignment>(distancesIn);

  int32_t idx = 0;
  float min = distancesIn[0];
  vec<float, 4> minvalues;
  vbroadcast(minvalues, min);
  vec<float, 4> values1;
  vec<float, 4> values2;
  vec<float, 4> values3;
  vec<float, 4> values4;
  vec<float, 4> values5;
  vec<float, 4> values6;
  vec<float, 4> values7;
  vec<float, 4> values8;
  for (int i = 0; i < n; i += 32) {
    // 1
    vload(values1, array + i);  // 0-3
    // 2
    vload(values2, array + i + 4);  // 4-7
    // 3
    vload(values3, array + i + 8);  // 8-11
    // 4
    vload(values4, array + i + 12);  // 12-15
    // 5
    vload(values5, array + i + 16);  // 16-19
    // 6
    vload(values6, array + i + 20);  // 20-23
    // 7
    vload(values7, array + i + 24);  // 24-27
    // 8
    vload(values8, array + i + 28);  // 28-31

    // Compare //1 with //2
    vmin(values1, values1, values2);
    // compare //3 with //4
    vmin(values3, values3, values4);
    // Compare //5 with //6
    vmin(values5, values5, values6);
    // compare /7 with //8
    vmin(values7, values7, values8);

    // Compare //1 with //3
    vmin(values1, values1, values3);
    // Compare //5 with //7
    vmin(values5, values5, values7);
    // Compare //1 with //5
    vmin(values1, values1, values5);

    // see if the new minimum contain something less
    // than the existing.
    vec<int, 4> newMinimumMask = values1 < minvalues;
    if (vany(newMinimumMask)) {
      idx = i;
      float minCandidates[4];
      vstore(minCandidates, values1);
      for (int j = 0; j < 4; ++j) {
        if (minCandidates[j] < min) {
          min = minCandidates[j];
        }
      }
      vbroadcast(minvalues, min);
    }
  }

  // Do the final calculation scalar way
  for (int i = idx; i < idx + 32; ++i) {
    if (distancesIn[i] == min) {
      return i;
    }
  }
  return 0;
}
}  // namespace findMinimumIndexDetail

namespace findMinimumIndex {
enum Impl { VecUpdateIdxOnNewMin = 0, VecAlwaysTrackIdx = 1, C = 2, STL = 3 };

template <enum Impl I>
ATH_ALWAYS_INLINE int32_t impl(const float* distancesIn, int n) {

  static_assert(
      I == VecUpdateIdxOnNewMin || I == VecAlwaysTrackIdx || I == C || I == STL,
      "Not a valid implementation chosen");
  if constexpr (I == VecUpdateIdxOnNewMin) {
    return findMinimumIndexDetail::vecUpdateIdxOnNewMin(distancesIn, n);
  } else if constexpr (I == VecAlwaysTrackIdx) {
    return findMinimumIndexDetail::vecAlwaysTrackIdx(distancesIn, n);
  } else if constexpr (I == C) {
    return findMinimumIndexDetail::scalarC(distancesIn, n);
  } else if constexpr (I == STL) {
    return findMinimumIndexDetail::scalarSTL(distancesIn, n);
  }
}
}  // namespace findMinimumIndex

#endif
