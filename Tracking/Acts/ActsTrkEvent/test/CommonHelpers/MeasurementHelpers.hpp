/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// File based on Acts:
// https://github.com/acts-project/acts/blob/v21.0.0/Tests/CommonHelpers/Acts/Tests/CommonHelpers/MeasurementHelpers.hpp
// TODO: centralize the helper files



#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Utilities/Helpers.hpp"

#include <cstddef>

namespace Acts {

namespace detail {
/// Helper functor for @c visit_measurement. This is the actual functor given
/// to @c template_switch.
/// @tparam I Compile time int value
template <size_t I>
struct visit_measurement_callable {
  /// The invoked function. It will perform the head/top-left corner
  /// extraction, and pass thee results to the given lambda.
  /// @tparam L The lambda type
  /// @tparam A The parameter vector type
  /// @tparam B The covariance matrix type
  /// @note No requirements on @c A and @c B are made, to enable a single
  /// overload for both const and non-const matrices/vectors.
  /// @param param The parameter vector
  /// @param cov The covariance matrix
  /// @param lambda The lambda to call with the statically sized subsets
  template <typename L, typename A, typename B>
  auto static constexpr invoke(A&& param, B&& cov, L&& lambda) {
    return lambda(param.template head<I>(), cov.template topLeftCorner<I, I>());
  }
};
}  // namespace detail

/// Dispatch a lambda call on an overallocated parameter vector and covariance
/// matrix, based on a runtime dimension value. Inside the lambda call, the
/// vector and matrix will have fixed dimensions, but will still point back to
/// the originally given overallocated values.
/// @tparam L The lambda type
/// @tparam A The parameter vector type
/// @tparam B The covariance matrix type
/// @note No requirements on @c A and @c B are made, to enable a single
/// overload for both const and non-const matrices/vectors.
/// @param param The parameter vector
/// @param cov The covariance matrix
/// @param dim The actual dimension as a runtime value
/// @param lambda The lambda to call with the statically sized subsets
template <typename L, typename A, typename B>
auto visit_measurement(A&& param, B&& cov, size_t dim, L&& lambda) {
  return template_switch<detail::visit_measurement_callable, 1, eBoundSize>(
      dim, param, cov, lambda);
}

//MWMW

/// Alternative version of @c template_switch which accepts a generic
/// lambda and communicates the dimension via an integral constant type
/// @tparam N Value from which to start the dispatch chain, i.e. 0 in most cases
/// @tparam NMAX Maximum value up to which to attempt a dispatch
/// @param v The runtime value to dispatch on
/// @param func The lambda to invoke
/// @param args Additional arguments passed to @p func
template <size_t N, size_t NMAX, typename Lambda, typename... Args>
auto template_switch_lambda(size_t v, Lambda&& func, Args&&... args) {
  if (v == N) {
    return func(std::integral_constant<size_t, N>{},
                std::forward<Args>(args)...);
  }
  if constexpr (N < NMAX) {
    return template_switch_lambda<N + 1, NMAX>(v, func,
                                               std::forward<Args>(args)...);
  }
  std::cerr << "template_switch<Fn, " << N << ", " << NMAX << ">(v=" << v
            << ") is not valid (v > NMAX)" << std::endl;
  std::abort();
}

// MWMW end

/// Dispatch a generic lambda on a measurement dimension. This overload doesn't
/// assume anything about what is needed inside the lambda, it communicates the
/// dimension via an integral constant type
/// @tparam L The generic lambda type to call
/// @param dim The runtime dimension of the measurement
/// @param lambda The generic lambda instance to call
/// @return Returns the lambda return value
template <typename L>
auto visit_measurement(size_t dim, L&& lambda) {
  return template_switch_lambda<1, eBoundSize>(dim, lambda);
}

}  // namespace Acts