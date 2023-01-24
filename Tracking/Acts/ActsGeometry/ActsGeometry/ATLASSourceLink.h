
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ATLASOURCELINK_H
#define ACTSGEOMETRY_ATLASOURCELINK_H

#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"

// add include when finished

#include <stdexcept>
#include <string>

/// Source link class for simulation in the acts-framework.
///
/// The source link stores the measuremts, surface, and the associated simulated
/// truth hit.
///
/// @todo Allow multiple truth hits e.g. for merged hits.
template <typename measurement_t>
class ATLASSourceLinkGeneric : public Acts::SourceLink
{
public:
  using Measurement = measurement_t;

  ATLASSourceLinkGeneric(const Acts::Surface &surface, const measurement_t &atlasHit,
                         size_t dim, Acts::BoundVector values, Acts::BoundMatrix cov)
      : Acts::SourceLink{surface.geometryId()},
        m_values(values),
        m_cov(cov),
        m_dim(dim),
        m_atlasHit(&atlasHit) {}

  /// Must be default_constructible to satisfy SourceLinkConcept.
  ATLASSourceLinkGeneric(ATLASSourceLinkGeneric &&) = default;
  ATLASSourceLinkGeneric(const ATLASSourceLinkGeneric &) = default;
  ATLASSourceLinkGeneric &operator=(ATLASSourceLinkGeneric &&) = default;
  ATLASSourceLinkGeneric &operator=(const ATLASSourceLinkGeneric &) = default;

  Acts::BoundVector values() const { return m_values; }
  Acts::BoundMatrix cov() const { return m_cov; }
  constexpr size_t dim() const { return m_dim; }
  constexpr const measurement_t &atlasHit() const { return *m_atlasHit; }

private:
  Acts::BoundVector m_values;
  Acts::BoundMatrix m_cov;
  size_t m_dim = 0u;
  // need to store pointers to make the object copyable
  const measurement_t *m_atlasHit;

  friend constexpr bool operator==(const ATLASSourceLinkGeneric &lhs,
                                   const ATLASSourceLinkGeneric &rhs)
  {
    return lhs.m_atlasHit == rhs.m_atlasHit;
  }

  friend constexpr bool operator!=(const ATLASSourceLinkGeneric &lhs,
                                   const ATLASSourceLinkGeneric &rhs)
  {
    return lhs.m_atlasHit != rhs.m_atlasHit;
  }
};

using ATLASSourceLink = ATLASSourceLinkGeneric<Trk::MeasurementBase>;
using ATLASUncalibSourceLink = ATLASSourceLinkGeneric<xAOD::UncalibratedMeasurement>;

/// A calibrator to extract the measurement from a ATLASSourceLink.
struct ATLASSourceLinkCalibrator final
{
  /// Extract the measurement.
  ///
  /// @tparam gctx The geometry context
  /// @param trackState The track state to calibrate
  template <typename trajectory_t, typename sourceLink_t = ATLASSourceLinkGeneric<Trk::MeasurementBase>>
  static void calibrate(const Acts::GeometryContext &gctx,
                        typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState);
};

template <typename trajectory_t, typename sourceLink_t>
void ATLASSourceLinkCalibrator::calibrate(const Acts::GeometryContext & /*gctx*/,
                                          typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState)
{
  const auto &sourceLink = static_cast<const sourceLink_t &>(trackState.uncalibrated());
  trackState.allocateCalibrated(sourceLink.dim());
  if (sourceLink.dim() == 0)
  {
    throw std::runtime_error("Cannot create dim 0 measurement");
  }
  else if (sourceLink.dim() == 1)
  {
    // return Acts::makeMeasurement(sourceLink, sourceLink.values().head<1>(), sourceLink.cov().topLeftCorner<1, 1>(), Acts::eBoundLoc0);
    trackState.template calibrated<1>() = sourceLink.values().template head<1>();
    trackState.template calibratedCovariance<1>() = sourceLink.cov().template topLeftCorner<1, 1>();
    trackState.calibratedSize() = sourceLink.dim();
    // Create a 1D projection matrix
    Acts::ActsMatrix<Acts::MultiTrajectory<trajectory_t>::MeasurementSizeMax, 1> proj;
    proj.setZero();
    proj(Acts::eBoundLoc0, Acts::eBoundLoc0) = 1;
    trackState.setProjector(proj);
  }
  else if (sourceLink.dim() == 2)
  {
    // return Acts::makeMeasurement(sourceLink, sourceLink.values().head<2>(), sourceLink.cov().topLeftCorner<2, 2>(), Acts::eBoundLoc0, Acts::eBoundLoc1);
    trackState.template calibrated<2>() = sourceLink.values().template head<2>();
    trackState.template calibratedCovariance<2>() = sourceLink.cov().template topLeftCorner<2, 2>();
    trackState.calibratedSize() = sourceLink.dim();
    // Create a 2D projection matrix
    Acts::ActsMatrix<Acts::MultiTrajectory<trajectory_t>::MeasurementSizeMax, 2> proj;
    proj.setZero();
    proj(Acts::eBoundLoc0, Acts::eBoundLoc0) = 1;
    proj(Acts::eBoundLoc1, Acts::eBoundLoc1) = 1;
    trackState.setProjector(proj);
  }
  else
  {
    throw std::runtime_error("Dim " + std::to_string(sourceLink.dim()) +
                             " currently not supported.");
  }
}

#endif
