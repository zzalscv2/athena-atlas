/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EventPrimitivesHelpers.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef EVENTPRIMITIVES_EVENTPRIMITIVESHELPERS_H
#define EVENTPRIMITIVES_EVENTPRIMITIVESHELPERS_H

#include "EventPrimitives/EventPrimitives.h"
//
#include <iostream>
#include <vector>

#include "cmath"

/** Event primitives helper functions
 @author  Niels van Eldik
 @author  Robert Johannes Langenberg
 @author  Andreas Salzburger
 @author  Johannes Junggeburth
 @author  Christos Anastopoulos

 */

namespace Amg {
/**  Sometimes the extrapolation to the next surface succeeds but has
   termendoulsy large errors leading to uncertainties larger than the radius of
   the Geneva metropole. These ones themself are clearly unphysical, but if
   extrapolation continues to the next surface the numerical values blow up
   giving rise to floating point exception. The covariance_cutoff defines a
   maximum value for the diagonal elements of the covariance matrix to consider
   them as sanish
*/
inline bool saneCovarianceElement(double ele) {
  // Elements > 3.4028234663852886e+38
  // make no-sense remember Gaudi units are in mm
  // What we say is that the position error must be less
  // than 3.4028234663852886e+38
  constexpr double upper_covariance_cutoff = std::numeric_limits<float>::max();
  return !(std::isnan(ele) || std::isinf(ele) ||
           std::abs(ele) > upper_covariance_cutoff);
}
/// Returns true if all diagonal elements of the covariance matrix
/// are finite, greater than zero and also below the covariance cutoff.
/// This check avoids floating point exceptions raised during the uncertainty
/// calculation on the perigee parameters
template <int N>
inline bool saneCovarianceDiagonal(const AmgSymMatrix(N) & mat) {
  // For now use float min 1.1754943508222875e-38
  // This implies that (sigma(q/p)) ^2 has to be greater than
  // than 1.1754943508222875e-38
  constexpr double MIN_COV_EPSILON = std::numeric_limits<float>::min();
  constexpr int dim = N;
  for (int i = 0; i < dim; ++i) {
    if (mat(i, i) < MIN_COV_EPSILON || !saneCovarianceElement(mat(i, i)))
      return false;
  }
  return true;
}
//// Check whether all components of a vector are finite and whether the
//// length of the vector is still within the Geneva area, i.e. 10 km
template <int N>
inline bool saneVector(const AmgVector(N) & vec) {
  for (int i = 0; i < N; ++i) {
    if (std::isnan(vec[i]) || std::isinf(vec[i]))
      return false;
  }
  constexpr double max_length2 = 1.e16;
  return vec.dot(vec) < max_length2;
}

/** return diagonal error of the matrix
 caller should ensure the matrix is symmetric and the index is in range
 */
inline double error(const Amg::MatrixX& mat, int index) {
  return std::sqrt(mat(index, index));
}
template <int N>
inline double error(const AmgSymMatrix(N) & mat, int index) {
  assert(index < N);
  return std::sqrt(mat(index, index));
}

// expression template to evaluate the required size of the compressed matrix at
// compile time
inline constexpr int CalculateCompressedSize(int n) {
  return (n * (n + 1)) / 2;
}

template <int N>
inline void compress(const AmgSymMatrix(N) & covMatrix,
                     std::vector<float>& vec) {
  vec.reserve(CalculateCompressedSize(N));
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j <= i; ++j)
      vec.emplace_back(covMatrix(i, j));
}

inline void compress(const MatrixX& covMatrix, std::vector<float>& vec) {
  int rows = covMatrix.rows();
  vec.reserve(CalculateCompressedSize(rows));
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j <= i; ++j) {
      vec.emplace_back(covMatrix(i, j));
    }
  }
}

template <int N>
inline void expand(std::vector<float>::const_iterator it,
                   std::vector<float>::const_iterator,
                   AmgSymMatrix(N) & covMatrix) {
  for (unsigned int i = 0; i < N; ++i) {
    for (unsigned int j = 0; j <= i; ++j) {
      covMatrix.fillSymmetric(i, j, *it);
      ++it;
    }
  }
}

inline void expand(std::vector<float>::const_iterator it,
                   std::vector<float>::const_iterator it_end,
                   MatrixX& covMatrix) {
  unsigned int dist = std::distance(it, it_end);
  unsigned int n;
  for (n = 1; dist > n; ++n) {
    dist = dist - n;
  }
  covMatrix = MatrixX(n, n);
  for (unsigned int i = 0; i < n; ++i) {
    for (unsigned int j = 0; j <= i; ++j) {
      covMatrix.fillSymmetric(i, j, *it);
      ++it;
    }
  }
}

/** compare two matrices, returns the indices of the first element that fails
 the condition, returns <-1,-1> if all is ok Users can provide the required
 precision and whether the difference should be evaluate relative to the values
 or absolutely
 */
template <int N>
std::pair<int, int> compare(const AmgSymMatrix(N) & m1,
                            const AmgSymMatrix(N) & m2, double precision = 1e-9,
                            bool relative = false) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      if (relative) {
        if (0.5 * std::abs(m1(i, j) - m2(i, j)) / (m1(i, j) + m2(i, j)) >
            precision) {
          return std::make_pair(i, j);
        }
      } else {
        if (std::abs(m1(i, j) - m2(i, j)) > precision) {
          return std::make_pair(i, j);
        }
      }
    }
  }
  return std::make_pair(-1, -1);
}

/** compare two vectors, returns the indices of the first element that fails the
 condition, returns <-1,-1> if all is ok Users can provide the required
 precision and whether the difference should be evaluate relative to the values
 or absolutely
 */
template <int N>
int compare(const AmgVector(N) & m1, const AmgVector(N) & m2,
            double precision = 1e-9, bool relative = false) {
  for (int i = 0; i < N; ++i) {
    if (relative) {
      if (0.5 * std::abs(m1(i) - m2(i)) / (m1(i) + m2(i)) > precision)
        return i;
    } else {
      if (std::abs(m1(i) - m2(i)) > precision)
        return i;
    }
  }
  return -1;
}

}  // namespace Amg

#endif
