/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSFITTERHELPER_H
#define ACTSGEOMETRY_ACTSFITTERHELPER_H

// ATHENA   
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"

// ACTS
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"

// PACKAGE  
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ActsDetectorElement.h"

namespace ActsTrk::FitterHelperFunctions {
  template<typename trajectory_t>
  Acts::Result<void> gainMatrixUpdate(const Acts::GeometryContext& gctx,
                    typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState, 
                    Acts::Direction direction, 
                    const Acts::Logger& logger) {
    Acts::GainMatrixUpdater updater;
    return updater.template operator()<trajectory_t>(gctx, trackState, direction, logger);
  }

  template<typename trajectory_t>
  Acts::Result<void> gainMatrixSmoother(const Acts::GeometryContext& gctx,
                Acts::MultiTrajectory<trajectory_t>& trajectory, 
                size_t entryIndex, 
                const Acts::Logger& logger) {
    Acts::GainMatrixSmoother smoother;
    return smoother.template operator()<trajectory_t>(gctx, trajectory, entryIndex, logger);
  }

  /// Outlier finder using a Chi2 cut.
  struct ATLASOutlierFinder {
    double StateChiSquaredPerNumberDoFCut = std::numeric_limits<double>::max();
    /// Classify a measurement as a valid one or an outlier.
    ///
    /// @tparam track_state_t Type of the track state
    /// @param state The track state to classify
    /// @retval False if the measurement is not an outlier
    /// @retval True if the measurement is an outlier
    template<typename trajectory_t>
    bool operator()(typename Acts::MultiTrajectory<trajectory_t>::ConstTrackStateProxy state) const {
      // can't determine an outlier w/o a measurement or predicted parameters
      if (not state.hasCalibrated() or not state.hasPredicted()) {
        return false;
      }
      return Acts::visit_measurement(
          state.calibratedSize(),
    [&] (auto N) -> bool {
      constexpr size_t kMeasurementSize = decltype(N)::value;

      typename Acts::TrackStateTraits<kMeasurementSize, true>::Measurement calibrated{
        state.template calibrated<Acts::MultiTrajectoryTraits::MeasurementSizeMax>().data()};
      
      typename Acts::TrackStateTraits<kMeasurementSize, true>::MeasurementCovariance
        calibratedCovariance{state.template calibratedCovariance<Acts::MultiTrajectoryTraits::MeasurementSizeMax>().data()};
      
      // Take the projector (measurement mapping function)
            const auto H =
                state.projector()
                    .template topLeftCorner<kMeasurementSize, Acts::BoundIndices::eBoundSize>()
                    .eval();
      
      const auto residual = calibrated - H * state.predicted();
      double chi2 = (residual.transpose() * (calibratedCovariance + H * state.predictedCovariance() * H.transpose()).inverse() * residual).value();     
            return (chi2 > StateChiSquaredPerNumberDoFCut * kMeasurementSize);
          });
    }
  };

  /// Determine if the smoothing of a track should be done with or without reverse
  /// filtering
  struct ReverseFilteringLogic {
    double momentumMax = std::numeric_limits<double>::max();

    /// Determine if the smoothing of a track should be done with or without reverse
    /// filtering
    ///
    /// @tparam track_state_t Type of the track state
    /// @param trackState The trackState of the last measurement
    /// @retval False if we don't use the reverse filtering for the smoothing of the track
    /// @retval True if we use the reverse filtering for the smoothing of the track
    template<typename trajectory_t>
    bool operator()(typename Acts::MultiTrajectory<trajectory_t>::ConstTrackStateProxy trackState) const {
      // can't determine an outlier w/o a measurement or predicted parameters
      auto momentum = std::abs(1. / trackState.filtered()[Acts::eBoundQOverP]);
      return (momentum <= momentumMax);
    }
  };

} // namespace

#endif
