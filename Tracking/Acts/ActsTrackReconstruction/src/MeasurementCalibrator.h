/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MEASUREMENTCALIBRATOR_H
#define MEASUREMENTCALIBRATOR_H

#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Utilities/AlgebraHelpers.hpp"
#include <Eigen/Core>

#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

#include "ActsGeometry/TrackingSurfaceHelper.h"

#include <stdexcept>
#include <string>
#include <cassert>

namespace ActsTrk {
template <typename trajectory_t>
class MeasurementCalibratorBase
{
protected:
   template <typename Derived>
   static Acts::ProjectorBitset makeProjectorBitset(const Eigen::MatrixBase<Derived> &proj) {
      constexpr int rows = Eigen::MatrixBase<Derived>::RowsAtCompileTime;
      constexpr int cols = Eigen::MatrixBase<Derived>::ColsAtCompileTime;

      typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy::Projector fullProjector =
        decltype(fullProjector)::Zero();

      fullProjector.template topLeftCorner<rows, cols>() = proj;

      return Acts::matrixToBitset(fullProjector).to_ullong();
   }

   static Acts::ProjectorBitset makeStripProjector(bool annulus_strip) {
      Acts::ActsMatrix<Acts::MultiTrajectory<trajectory_t>::MeasurementSizeMax, 2> proj;
      proj.setZero();
      if (annulus_strip) {
         // transforms predicted[1] -> calibrated[0] in Acts::MeasurementSelector::calculateChi2()
         proj(Acts::eBoundLoc0, Acts::eBoundLoc1) = 1;
      } else {
         proj(Acts::eBoundLoc0, Acts::eBoundLoc0) = 1;
      }
      return makeProjectorBitset(proj);
   }
   // create strip projector bitsets for normal and annulus bounds
   static std::array<Acts::ProjectorBitset,2> makeStripProjectorArray() {
      return std::array<Acts::ProjectorBitset,2>{makeStripProjector(false), makeStripProjector(true) };
   }

   // create pixel projector bitsets
   static Acts::ProjectorBitset makePixelProjector() {
      Acts::ActsMatrix<Acts::MultiTrajectory<trajectory_t>::MeasurementSizeMax, 2> proj;
      proj.setZero();
      proj(Acts::eBoundLoc0, Acts::eBoundLoc0) = 1;
      proj(Acts::eBoundLoc1, Acts::eBoundLoc1) = 1;
      return makeProjectorBitset(proj);
   }

   // unfortunately cannot make static constexpr because of eigen
   // could make static but usage easier if the projectors are members
   std::array<Acts::ProjectorBitset,2> m_stripProjector; // strip projector for normal and annulus bounds
   Acts::ProjectorBitset               m_pixelProjector;

   const ActsTrk::IActsToTrkConverterTool *m_converterTool;
public:
   MeasurementCalibratorBase(const ActsTrk::IActsToTrkConverterTool &converter_tool)
      : m_stripProjector( makeStripProjectorArray()),
        m_pixelProjector( makePixelProjector() ),
        m_converterTool(&converter_tool) { assert(m_converterTool); }

   template <class measurement_t>
   inline void setStateFromMeasurement( const measurement_t &measurement,
                                        Acts::SurfaceBounds::BoundsType bound_type,
                                        typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy &trackState ) const {
      switch (measurement.type()) {
      case (xAOD::UncalibMeasType::StripClusterType): {
         trackState.allocateCalibrated(1);
         const std::size_t projector_idx  = bound_type == Acts::SurfaceBounds::eAnnulus;
         trackState.setProjectorBitset(m_stripProjector[ projector_idx ] );
         trackState.template calibrated<1>()[Acts::eBoundLoc0]
            = measurement.template localPosition<1>()[Trk::locX];
         trackState.template calibratedCovariance<1>()
            = measurement.template localCovariance<1>()
                .template topLeftCorner<1, 1>();
         break;
      }
      case (xAOD::UncalibMeasType::PixelClusterType): {
         trackState.allocateCalibrated(2);
         trackState.setProjectorBitset(m_pixelProjector);
         trackState.template calibrated<2>()[Acts::eBoundLoc0]
            = measurement.template localPosition<2>()[Trk::locX];
         trackState.template calibrated<2>()[Acts::eBoundLoc1]
            = measurement.template localPosition<2>()[Trk::locY];

         trackState.template calibratedCovariance<2>()
            = measurement.template localCovariance<2>()
                .template topLeftCorner<2, 2>();
         break;
      }
      default:
         throw std::domain_error("Can only handle measurement type pixel or strip");
      };
   }
};

template <typename trajectory_t>
class UncalibratedMeasurementCalibrator : public MeasurementCalibratorBase<trajectory_t> {
protected:
public:
   UncalibratedMeasurementCalibrator(const ActsTrk::IActsToTrkConverterTool &converter_tool,
                                     const TrackingSurfaceHelper &surface_helper)
      : MeasurementCalibratorBase<trajectory_t>(converter_tool),
        m_surfaceHelper(&surface_helper)
   { assert( m_surfaceHelper); }

   class MeasurementAdapter {
   public:
      MeasurementAdapter(const xAOD::UncalibratedMeasurement &measurement) : m_measurement(&measurement) {}
      xAOD::UncalibMeasType type() const {
         return m_measurement->type();
      }
      template <std::size_t DIM>
      inline auto localPosition() const {
         return m_measurement->template localPosition<DIM>().template cast<Acts::ActsScalar>();
      }
      template <std::size_t DIM>
      inline auto localCovariance() const {
         return m_measurement->template localCovariance<DIM>().template cast<Acts::ActsScalar>();
      }
   private:
      const xAOD::UncalibratedMeasurement *m_measurement;
   };


   void operator()([[maybe_unused]] const Acts::GeometryContext &gctx,
                   [[maybe_unused]] const Acts::CalibrationContext & cctx,
                   const Acts::SourceLink& sl,
                   typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState) const {
      auto sourceLink = sl.template get<ATLASUncalibSourceLink>();
      trackState.setUncalibratedSourceLink(sl);
      assert(sourceLink.isValid() && *sourceLink );
      const xAOD::UncalibratedMeasurement *measurement = *sourceLink;
      const Acts::Surface &surface = m_surfaceHelper->associatedActsSurface( *measurement );

      this->setStateFromMeasurement(MeasurementAdapter(*measurement),
                                    surface.bounds().type(),
                                    trackState);
   }

private:
   const TrackingSurfaceHelper *m_surfaceHelper;
};

template <typename trajectory_t>
class TrkMeasurementCalibrator : public MeasurementCalibratorBase<trajectory_t> {
public:
   class MeasurementAdapter {
   public:
      MeasurementAdapter(const Trk::MeasurementBase &measurement) : m_measurement(&measurement) {}
      xAOD::UncalibMeasType type() const {
         switch (m_measurement->localParameters().dimension()) {
         case 1: {
            return xAOD::UncalibMeasType::StripClusterType;
         }
         case 2: {
            return xAOD::UncalibMeasType::PixelClusterType;
         }
         default: {
            return xAOD::UncalibMeasType::Other;
         }
         }
      }
      template <std::size_t DIM>
      inline const Trk::LocalParameters& localPosition() const {
         assert( m_measurement && DIM == m_measurement->localParameters().dimension());
         return m_measurement->localParameters();
      }
      template <std::size_t DIM>
      inline const Amg::MatrixX &localCovariance() const {
         assert( m_measurement && DIM == m_measurement->localParameters().dimension());
         return m_measurement->localCovariance();
      }
   private:
      const Trk::MeasurementBase *m_measurement;
   };

   TrkMeasurementCalibrator(const ActsTrk::IActsToTrkConverterTool &converter_tool)
      : MeasurementCalibratorBase<trajectory_t>(converter_tool) {}
   void operator()([[maybe_unused]] const Acts::GeometryContext &gctx,
                   [[maybe_unused]] const Acts::CalibrationContext & cctx,
                   const Acts::SourceLink& sl,
                   typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState) const {
      auto sourceLink = sl.template get<ATLASSourceLink>();
      trackState.setUncalibratedSourceLink(sl);
      assert(sourceLink);
      const Acts::Surface &surface = this->m_converterTool->trkSurfaceToActsSurface(sourceLink->associatedSurface());
      this->setStateFromMeasurement(MeasurementAdapter(*sourceLink),
                                    surface.bounds().type(),
                                    trackState);
   }
};
}
#endif
