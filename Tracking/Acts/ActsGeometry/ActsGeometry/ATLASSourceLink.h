/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ATLASOURCELINK_H
#define ACTSGEOMETRY_ATLASOURCELINK_H

#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "Acts/EventData/Measurement.hpp"
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
class ATLASSourceLinkGeneric final
{
  enum ElementType : std::int8_t {MEASUREMENT=0, LOC, COV, TYPE, BOUND};

public:
  using Measurement = measurement_t;
  using ElementsType = std::tuple<const measurement_t*, Acts::BoundVector, Acts::BoundMatrix, std::size_t, Acts::SurfaceBounds::BoundsType>;

  ATLASSourceLinkGeneric(const Acts::Surface &surface, ElementsType& elements)
    : m_geometryId(surface.geometryId()),
    m_elements(&elements)
      {}
  
  /// Must be default_constructible to satisfy SourceLinkConcept.
  ATLASSourceLinkGeneric(ATLASSourceLinkGeneric &&) noexcept = default;
  ATLASSourceLinkGeneric(const ATLASSourceLinkGeneric &) = default;
  ATLASSourceLinkGeneric &operator=(ATLASSourceLinkGeneric &&) noexcept = default;
  ATLASSourceLinkGeneric &operator=(const ATLASSourceLinkGeneric &) = default;
  
  const Acts::BoundVector& values() const { 
    const auto& elements = this->collection();
    return std::get<ElementType::LOC>(elements); 
  }
  const Acts::BoundMatrix& cov() const { 
    const auto& elements = this->collection();
    return std::get<ElementType::COV>(elements); 
  }
  size_t dim() const { 
    const auto& elements = this->collection();
    return std::get<ElementType::TYPE>(elements); 
  }

  const measurement_t& atlasHit() const { 
    const auto& elements = this->collection();
    return *std::get<ElementType::MEASUREMENT>(elements); 
  }

  Acts::SurfaceBounds::BoundsType boundsType() const { 
    const auto& elements = this->collection();
    return std::get<ElementType::BOUND>(elements); 
  }

  Acts::GeometryIdentifier geometryId() const { return m_geometryId; }

 private:
  const ElementsType& collection() const { return *m_elements; }

 private:
  Acts::GeometryIdentifier m_geometryId{};
  ElementsType* m_elements;

  friend constexpr bool operator==(const ATLASSourceLinkGeneric &lhs,
                                   const ATLASSourceLinkGeneric &rhs)
  {
    return &lhs.atlasHit() == &rhs.atlasHit();
  }

  friend constexpr bool operator!=(const ATLASSourceLinkGeneric &lhs,
                                   const ATLASSourceLinkGeneric &rhs)
  {
    return &lhs.atlasHit() != &rhs.atlasHit();
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
void ATLASSourceLinkCalibrator::calibrate(const Acts::GeometryContext& /*gctx*/,
					  typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState) {
  auto sourceLink = trackState.getUncalibratedSourceLink().template get<sourceLink_t>();
  trackState.allocateCalibrated(sourceLink.dim());
  if (sourceLink.dim() == 0)
  {
    throw std::runtime_error("Cannot create dim 0 measurement");
  } else if (sourceLink.dim() == 1) {
    trackState.template calibrated<1>() = sourceLink.values().template head<1>();
    trackState.template calibratedCovariance<1>() = sourceLink.cov().template topLeftCorner<1, 1>();
    // Create a projection matrix onto 1D measurement
    Acts::ActsMatrix<Acts::MultiTrajectory<trajectory_t>::MeasurementSizeMax, 2> proj;
    proj.setZero();
    if (sourceLink.boundsType() == Acts::SurfaceBounds::eAnnulus) {
      proj(Acts::eBoundLoc0, Acts::eBoundLoc1) = 1; // transforms predicted[1] -> calibrated[0] in Acts::MeasurementSelector::calculateChi2()
    } else {
      proj(Acts::eBoundLoc0, Acts::eBoundLoc0) = 1;
    }
    trackState.setProjector(proj);
  }
  else if (sourceLink.dim() == 2)
    {
      trackState.template calibrated<2>() = sourceLink.values().template head<2>();
      trackState.template calibratedCovariance<2>() = sourceLink.cov().template topLeftCorner<2, 2>();
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
