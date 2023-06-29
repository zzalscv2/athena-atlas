/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// File based on Acts:
// https://github.com/acts-project/acts/blob/v21.0.0/Tests/CommonHelpers/Acts/Tests/CommonHelpers/MeasurementHelpersTests.hpp
// TODO: centralize the helper files

#include <boost/test/unit_test.hpp>

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/MeasurementHelpers.hpp"

BOOST_AUTO_TEST_SUITE(EventDataMeasurementHelpers)

BOOST_AUTO_TEST_CASE(visit_measurement_test) {
  // Overallocated full size parameter vector and covariance
  Acts::BoundVector parFull = Acts::BoundVector::Random();
  Acts::BoundMatrix covFull = Acts::BoundMatrix::Random();
  // constant variants
  const auto& parFullConst = parFull;
  const auto& covFullConst = covFull;

  for (Acts::BoundVector::Index dim = 1; dim <= parFull.size(); ++dim) {
    Acts::visit_measurement(parFull, covFull, dim, [&](auto param, auto cov) {
      BOOST_CHECK_EQUAL(param, parFull.head(dim));
      BOOST_CHECK_EQUAL(cov, covFull.topLeftCorner(dim, dim));
    });
    Acts::visit_measurement(
        parFull, covFull, dim, [&](const auto& param, const auto& cov) {
          BOOST_CHECK_EQUAL(param, parFull.head(dim));
          BOOST_CHECK_EQUAL(cov, covFull.topLeftCorner(dim, dim));
        });
    Acts::visit_measurement(parFullConst, covFullConst, dim,
                            [&](const auto& param, const auto& cov) {
                              BOOST_CHECK_EQUAL(param, parFullConst.head(dim));
                              BOOST_CHECK_EQUAL(
                                  cov, covFullConst.topLeftCorner(dim, dim));
                            });
  }
}

BOOST_AUTO_TEST_SUITE_END()
