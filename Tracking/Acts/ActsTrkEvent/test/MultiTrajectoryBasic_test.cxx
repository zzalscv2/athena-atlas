/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackParameters.hpp"

#include "ActsTrkEvent/MultiTrajectory.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"

#include "CommonHelpers/GenerateParameters.hpp"
#include "CommonHelpers/TestSourceLink.hpp"

namespace {

using namespace Acts;
using namespace Acts::UnitLiterals;
using namespace Acts::Test;
namespace bd = boost::unit_test::data;

using ParametersVector = BoundTrackParameters::ParametersVector;
using CovarianceMatrix = BoundTrackParameters::CovarianceMatrix;
using Jacobian = Acts::BoundMatrix;

struct TestTrackState {
  std::shared_ptr<Surface> surface;
  TestSourceLink sourceLink;
  BoundTrackParameters predicted;
  BoundTrackParameters filtered;
  BoundTrackParameters smoothed;
  Jacobian jacobian;
  double chi2;
  double pathLength;

  // Generate a random TestTrackState.
  //
  // @param rng Random number generator
  // @param size_t nMeasurement either 1 or 2
  template <typename rng_t>
  TestTrackState(rng_t &rng, size_t nMeasurements)
      : surface(Acts::Surface::makeShared<Acts::PlaneSurface>(
            Vector3::Zero(), Vector3::UnitZ())),
        // set bogus parameters first since they are not default-constructible
        predicted(surface, BoundVector::Zero()),
        filtered(surface, BoundVector::Zero()),
        smoothed(surface, BoundVector::Zero()), jacobian(Jacobian::Identity()),
        chi2(std::chi_squared_distribution<double>(nMeasurements)(rng)),
        pathLength(
            std::uniform_real_distribution<ActsScalar>(1_mm, 10_mm)(rng)) {
    // set a random geometry identifier to uniquely identify each surface
    auto geoId =
        std::uniform_int_distribution<GeometryIdentifier::Value>()(rng);
    surface->assignGeometryId(geoId);

    // create source link w/ inline 1d or 2d measurement data
    if (nMeasurements == 1u) {
      auto [par, cov] = generateParametersCovariance<ActsScalar, 1u>(rng);
      sourceLink = TestSourceLink(eBoundLoc0, par[0], cov(0, 0), geoId);
    } else if (nMeasurements == 2u) {
      auto [par, cov] = generateParametersCovariance<ActsScalar, 2u>(rng);
      sourceLink = TestSourceLink(eBoundLoc1, eBoundQOverP, par, cov, geoId);
    } else {
      throw std::runtime_error("invalid number of measurement dimensions");
    }

    // create track parameters
    auto [trkPar, trkCov] = generateBoundParametersCovariance(rng);
    trkPar[eBoundPhi] = 45_degree;
    trkPar[eBoundTheta] = 90_degree;
    trkPar[eBoundQOverP] = 5.;
    // predicted
    predicted = BoundTrackParameters(surface, trkPar, trkCov);
    // filtered, modified q/p, reduced covariance
    trkPar[eBoundQOverP] = 10.;
    filtered = BoundTrackParameters(surface, trkPar, 0.75 * trkCov);
    // smoothed, modified q/p, further reduced covariance
    trkPar[eBoundQOverP] = 15.;
    smoothed = BoundTrackParameters(surface, trkPar, 0.5 * trkCov);

    // propagation jacobian is identity + corrections
    for (Eigen::Index c = 0; c < jacobian.cols(); ++c) {
      for (Eigen::Index r = 0; r < jacobian.rows(); ++r) {
        jacobian(c, r) +=
            std::uniform_real_distribution<ActsScalar>(-0.125, 0.125)(rng);
      }
    }
  }
};

// Fill a TrackStateProxy with values from a TestTrackState.
//
// @param[in] pc TestTrackState with the input values
// @param[in] mask Specifies which components are used/filled
// @param[out] ts TrackStateProxy which is filled
// @param [in] nMeasurements Dimension of the measurement
template <typename track_state_t>
void fillTrackState(const TestTrackState &pc, TrackStatePropMask mask,
                    track_state_t &ts) {
  // always set the reference surface
  // ts.setReferenceSurface(pc.predicted.referenceSurface().getSharedPtr());
  std::cout << "pc type " << typeid(pc).name() << std::endl;
  std::cout << "pc type " << typeid(pc.predicted.parameters()).name()
            << std::endl;
  std::cout << "ts type " << typeid(ts).name() << std::endl;
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Predicted)) {
    ts.predicted() = pc.predicted.parameters();
    BOOST_CHECK_EQUAL(pc.predicted.parameters(), ts.predicted());
    BOOST_CHECK(pc.predicted.covariance().has_value());
    ts.predictedCovariance() = *(pc.predicted.covariance());
    BOOST_CHECK(*(pc.predicted.covariance()) == ts.predictedCovariance());
  }

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Filtered)) {
    ts.filtered() = pc.filtered.parameters();
    BOOST_CHECK(pc.filtered.covariance().has_value());
    ts.filteredCovariance() = *(pc.filtered.covariance());
  }

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Smoothed)) {
    ts.smoothed() = pc.smoothed.parameters();
    BOOST_CHECK(pc.smoothed.covariance().has_value());
    ts.smoothedCovariance() = *(pc.smoothed.covariance());
  }

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Jacobian)) {
    ts.jacobian() = pc.jacobian;
  }
  ts.chi2() = pc.chi2;
  ts.pathLength() = pc.pathLength;

  // TODO Add here the part of code from:
  // https://github.com/acts-project/acts/blob/d8cb0fac3a44e1d44595a481f977df9bd70195fb/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L139
  // requires measurementCovariance_impl(covIdx) to be defined
}

const GeometryContext gctx;
// fixed seed for reproducible tests
std::default_random_engine rng(31415);

} // namespace

BOOST_AUTO_TEST_SUITE(EventDataMultiTrajectory)

struct EmptyMTJ { // setup empty MTJ
  EmptyMTJ() {
    trackStateBackend = std::make_unique<xAOD::TrackStateContainer>();
    trackStateBackendAux = std::make_unique<xAOD::TrackStateAuxContainer>();
    trackStateBackend->setStore(trackStateBackendAux.get());

    parametersBackend = std::make_unique<xAOD::TrackParametersContainer>();
    parametersBackendAux =
        std::make_unique<xAOD::TrackParametersAuxContainer>();
    parametersBackend->setStore(parametersBackendAux.get());

    jacobianBackend = std::make_unique<xAOD::TrackJacobianContainer>();
    jacobianBackendAux = std::make_unique<xAOD::TrackJacobianAuxContainer>();
    jacobianBackend->setStore(jacobianBackendAux.get());

    measurementsBackend = std::make_unique<xAOD::TrackMeasurementContainer>();
    measurementsBackendAux =
        std::make_unique<xAOD::TrackMeasurementAuxContainer>();
    measurementsBackend->setStore(measurementsBackendAux.get());

    mtj = std::make_unique<ActsTrk::MutableMultiTrajectory>(
        trackStateBackend.get(), parametersBackend.get(), jacobianBackend.get(),
        measurementsBackend.get());
    // backends can be shared
    ro_mtj = std::make_unique<ActsTrk::ConstMultiTrajectory>(
        trackStateBackend.get(), parametersBackend.get(), jacobianBackend.get(),
        measurementsBackend.get());
  }
  std::unique_ptr<xAOD::TrackStateContainer> trackStateBackend;
  std::unique_ptr<xAOD::TrackStateAuxContainer> trackStateBackendAux;
  std::unique_ptr<xAOD::TrackParametersContainer> parametersBackend;
  std::unique_ptr<xAOD::TrackParametersAuxContainer> parametersBackendAux;
  std::unique_ptr<xAOD::TrackJacobianContainer> jacobianBackend;
  std::unique_ptr<xAOD::TrackJacobianAuxContainer> jacobianBackendAux;
  std::unique_ptr<xAOD::TrackMeasurementContainer> measurementsBackend;
  std::unique_ptr<xAOD::TrackMeasurementAuxContainer> measurementsBackendAux;

  std::unique_ptr<ActsTrk::MutableMultiTrajectory> mtj;
  std::unique_ptr<ActsTrk::ConstMultiTrajectory> ro_mtj;
};


// cppcheck-suppress syntaxError
BOOST_FIXTURE_TEST_CASE(Fill, EmptyMTJ) {
  BOOST_CHECK(mtj->has_backends());
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;
  auto i0 = mtj->addTrackState(kMask);
  // trajectory bifurcates here into multiple hypotheses
  auto i1a = mtj->addTrackState(kMask, i0);
  auto i1b = mtj->addTrackState(kMask, i0);
  auto i2a = mtj->addTrackState(kMask, i1a);
  auto i2b = mtj->addTrackState(kMask, i1b);

  std::vector<size_t> act;
  auto collect = [&](auto p) {
    act.push_back(p.index());
    BOOST_CHECK(!p.hasCalibrated());
    BOOST_CHECK(!p.hasFiltered());
    BOOST_CHECK(!p.hasSmoothed());
    BOOST_CHECK(!p.hasJacobian());
    BOOST_CHECK(!p.hasProjector());
  };

  std::vector<size_t> exp = {i2a, i1a, i0};
  mtj->applyBackwards(i2a, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());
  // the same test on read only collection TODO, this needs streamlining so we
  // so not repeat identical code
  act.clear();
  ro_mtj->visitBackwards(i2a, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());

  act.clear();
  exp = {i2b, i1b, i0};
  mtj->applyBackwards(i2b, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());
  // the same test on read only collection
  act.clear();
  ro_mtj->visitBackwards(i2b, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());
}

BOOST_FIXTURE_TEST_CASE(Dynamic_columns, EmptyMTJ) {
  using namespace Acts::HashedStringLiteral;
  BOOST_CHECK(mtj->has_backends());
  mtj->enableDecoration<short>("author");
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;
  auto i0 = mtj->addTrackState(kMask);
  auto i1 = mtj->addTrackState(kMask, i0);
  auto i2 = mtj->addTrackState(kMask, i1);
  // dynamic column enabled late
  mtj->enableDecoration<float>("mcprob");
  auto ts0 = mtj->getTrackState(i0);
  auto ts1 = mtj->getTrackState(i1);
  auto ts2 = mtj->getTrackState(i2);
  ts0.component<short, "author"_hash>() = 5;
  ts1.component<short, "author"_hash>() = 6;
  ts2.component<short, "author"_hash>() = 4;

  ts0.component<float, "mcprob"_hash>() = 0.5;
  ts1.component<float, "mcprob"_hash>() = 0.9;
  // unset for ts2

  // read them back
  BOOST_CHECK_EQUAL((ts0.component<short, "author"_hash>()), 5);
  BOOST_CHECK_EQUAL((ts1.component<short, "author"_hash>()), 6);
  BOOST_CHECK_EQUAL((ts2.component<short, "author"_hash>()), 4);

  BOOST_TEST((ts0.component<float, "mcprob"_hash>()) == 0.5,
             boost::test_tools::tolerance(0.01));
  BOOST_TEST((ts1.component<float, "mcprob"_hash>()) == 0.9,
             boost::test_tools::tolerance(0.01));
  BOOST_TEST((ts2.component<float, "mcprob"_hash>()) == 0.0,
             boost::test_tools::tolerance(0.01));

  BOOST_CHECK_THROW((ts2.component<float, "sth"_hash>()), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(UncalibratedSourceLink, EmptyMTJ) {
  auto i0 = mtj->addTrackState();
  auto ts0 = mtj->getTrackState(i0);
  using namespace Acts::HashedStringLiteral;

  BOOST_CHECK_EQUAL((ts0.component<Acts::SourceLink *, "uncalibrated"_hash>()),
                    nullptr);

  auto link1 = std::shared_ptr<ActsTrk::SourceLink>(nullptr);
  ts0.component<Acts::SourceLink *, "uncalibrated"_hash>() = link1.get();
  BOOST_CHECK_EQUAL((ts0.component<Acts::SourceLink *, "uncalibrated"_hash>()),
                    link1.get());

  // TODO add test for an instantiation of MTJ with eager SourceLinks creation
}

BOOST_FIXTURE_TEST_CASE(Clear, EmptyMTJ) {
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;

  BOOST_CHECK_EQUAL(mtj->size(), 0);

  auto i0 = mtj->addTrackState(kMask);
  // trajectory bifurcates here into multiple hypotheses
  auto i1a = mtj->addTrackState(kMask, i0);
  auto i1b = mtj->addTrackState(kMask, i0);
  mtj->addTrackState(kMask, i1a);
  mtj->addTrackState(kMask, i1b);

  BOOST_CHECK_EQUAL(mtj->size(), 5);
  mtj->clear();
  BOOST_CHECK_EQUAL(mtj->size(), 0);
}

BOOST_FIXTURE_TEST_CASE(ApplyWithAbort, EmptyMTJ) {
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;

  // construct trajectory with three components

  auto i0 = mtj->addTrackState(kMask);
  auto i1 = mtj->addTrackState(kMask, i0);
  auto i2 = mtj->addTrackState(kMask, i1);

  size_t n = 0;
  mtj->applyBackwards(i2, [&](const auto &) {
    n++;
    return false;
  });
  BOOST_CHECK_EQUAL(n, 1u);

  n = 0;
  mtj->applyBackwards(i2, [&](const auto &ts) {
    n++;
    if (ts.index() == i1) {
      return false;
    }
    return true;
  });
  BOOST_CHECK_EQUAL(n, 2u);

  n = 0;
  mtj->applyBackwards(i2, [&](const auto &) {
    n++;
    return true;
  });
  BOOST_CHECK_EQUAL(n, 3u);
}

BOOST_FIXTURE_TEST_CASE(BitmaskOperators, EmptyMTJ) {
  using PM = Acts::TrackStatePropMask;

  auto bs1 = PM::Predicted;

  BOOST_CHECK(ACTS_CHECK_BIT(bs1, PM::Predicted));
  BOOST_CHECK(!ACTS_CHECK_BIT(bs1, PM::Calibrated));

  auto bs2 = PM::Calibrated;

  BOOST_CHECK(!ACTS_CHECK_BIT(bs2, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(bs2, PM::Calibrated));

  auto bs3 = PM::Calibrated | PM::Predicted;

  BOOST_CHECK(ACTS_CHECK_BIT(bs3, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(bs3, PM::Calibrated));

  BOOST_CHECK(ACTS_CHECK_BIT(PM::All, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(PM::All, PM::Calibrated));

  auto bs4 = PM::Predicted | PM::Jacobian | PM::Smoothed;
  BOOST_CHECK(ACTS_CHECK_BIT(bs4, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(bs4, PM::Jacobian));
  BOOST_CHECK(ACTS_CHECK_BIT(bs4, PM::Smoothed));
  BOOST_CHECK(!ACTS_CHECK_BIT(bs4, PM::Calibrated));
  BOOST_CHECK(!ACTS_CHECK_BIT(bs4, PM::Filtered));

  auto cnv = [](auto a) -> std::bitset<8> {
    return static_cast<std::underlying_type<PM>::type>(a);
  };

  BOOST_CHECK(cnv(PM::All).all());   // all ones
  BOOST_CHECK(cnv(PM::None).none()); // all zeros

  // test orthogonality
  std::array<PM, 5> values{PM::Predicted, PM::Filtered, PM::Smoothed,
                           PM::Jacobian, PM::Calibrated};
  for (size_t i = 0; i < values.size(); i++) {
    for (size_t j = 0; j < values.size(); j++) {
      PM a = values[i];
      PM b = values[j];

      if (i == j) {
        BOOST_CHECK(cnv(a & b).count() == 1);
      } else {
        BOOST_CHECK(cnv(a & b).none());
      }
    }
  }

  BOOST_CHECK(cnv(PM::Predicted ^ PM::Filtered).count() == 2);
  BOOST_CHECK(cnv(PM::Predicted ^ PM::Predicted).none());
  BOOST_CHECK(~(PM::Predicted | PM::Calibrated) ==
              (PM::All ^ PM::Predicted ^ PM::Calibrated));

  PM base = PM::None;
  BOOST_CHECK(cnv(base) == 0);

  base &= PM::Filtered;
  BOOST_CHECK(cnv(base) == 0);

  base |= PM::Filtered;
  BOOST_CHECK(base == PM::Filtered);

  base |= PM::Calibrated;
  BOOST_CHECK(base == (PM::Filtered | PM::Calibrated));

  base ^= PM::All;
  BOOST_CHECK(base == ~(PM::Filtered | PM::Calibrated));
}

BOOST_FIXTURE_TEST_CASE(AddTrackStateWithBitMask, EmptyMTJ) {

  using PM = Acts::TrackStatePropMask;
  using namespace Acts::HashedStringLiteral;

  // TODO : add tests checking
  // "calibratedSourceLink","referenceSurface","typeFlags" see:
  // https://github.com/acts-project/acts/blob/5cbcbf01f1d6bbe79a98b84bdc6ef076cf763c01/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L321
  auto alwaysPresent = [](auto &ts) {
    BOOST_CHECK(ts.template has<"measdim"_hash>());
    BOOST_CHECK(ts.template has<"chi2"_hash>());
    BOOST_CHECK(ts.template has<"pathLength"_hash>());
  };

  auto ts = mtj->getTrackState(mtj->addTrackState(PM::All));
  BOOST_CHECK(ts.hasPredicted());
  BOOST_CHECK(ts.hasFiltered());
  BOOST_CHECK(ts.hasSmoothed());
  BOOST_CHECK(ts.hasCalibrated());
  BOOST_CHECK(ts.hasProjector());
  BOOST_CHECK(ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::None));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Predicted));
  BOOST_CHECK(ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Filtered));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Smoothed));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Calibrated));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(ts.hasCalibrated());
  BOOST_CHECK(ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());

  ts = mtj->getTrackState(mtj->addTrackState(PM::Jacobian));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(ts.hasJacobian());
  alwaysPresent(ts);
}

BOOST_FIXTURE_TEST_CASE(UseFillTrackState, EmptyMTJ) {
  std::default_random_engine rng(31415);
  TestTrackState pc(rng, 1);
  size_t index = mtj->addTrackState();
  auto ts = mtj->getTrackState(index);
  fillTrackState(pc, TrackStatePropMask::All, ts);
}

// TODO
// a test "assert expected "cross-talk" between trackstate proxies" to be added
// here:
// https://github.com/acts-project/acts/blob/d8cb0fac3a44e1d44595a481f977df9bd70195fb/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L436
// Problem to be solved: TrackState ts  ts.predicted() and ts.parameters() not
// filled

// TODO remaining tests
}