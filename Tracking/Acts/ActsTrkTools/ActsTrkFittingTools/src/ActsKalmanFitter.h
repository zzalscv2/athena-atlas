/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSKALMANFITTER_H
#define ACTSGEOMETRY_ACTSKALMANFITTER_H

#include "GaudiKernel/ToolHandle.h"


#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkFitterInterfaces/ITrackFitter.h"

#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"

// ACTS
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/EventData/Track.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

// PACKAGE

#include "ActsTrkEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsTrkEventCnv/IActsToTrkConverterTool.h"

// STL
#include <string>
#include <memory>//unique_ptr
#include <limits>//for numeric_limits
#include <cmath> //std::abs

class EventContext;

namespace Trk{
  class Track;
}

class ActsKalmanFitter : public extends<AthAlgTool, Trk::ITrackFitter> { 
public:
  using traj_Type = Acts::VectorMultiTrajectory;

  ActsKalmanFitter(const std::string&,const std::string&,const IInterface*);
  virtual ~ActsKalmanFitter() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  /*
   * Bring in default impl with
   * EventContext for now
  */
  using Trk::ITrackFitter::fit;

  //! refit a track
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! fit a set of PrepRawData objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::PrepRawDataSet&,
    const Trk::TrackParameters&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! fit a set of MeasurementBase objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::MeasurementSet&,
    const Trk::TrackParameters&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! extend a track fit including a new set of PrepRawData objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::PrepRawDataSet&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! extend a track fit including a new set of MeasurementBase objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::MeasurementSet&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! combined track fit
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::Track&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

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
	    double chi2 = (residual.transpose() * ((calibratedCovariance + H * state.predictedCovariance() * H.transpose())).inverse() * residual).value();	    
            return bool(chi2 > StateChiSquaredPerNumberDoFCut * kMeasurementSize);
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

  template<typename trajectory_t>
  using TrackFitterResult =
      typename Acts::Result<Acts::KalmanFitterResult<trajectory_t>>;



  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

  template<typename trajectory_t>
  static Acts::Result<void> gainMatrixUpdate(const Acts::GeometryContext& gctx,
					     typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState, 
					     Acts::NavigationDirection direction, 
					     const Acts::Logger& logger = Acts::getDummyLogger()); 

  template<typename trajectory_t>
  static Acts::Result<void> gainMatrixSmoother(const Acts::GeometryContext& gctx,
					       Acts::MultiTrajectory<trajectory_t>& trajectory, 
					       size_t entryIndex, 
					       const Acts::Logger& logger = Acts::getDummyLogger());

  // Create a track from the fitter result
  template<typename track_container_t, typename traj_t,
           template <typename> class holder_t>
  std::unique_ptr<Trk::Track> makeTrack(const EventContext& ctx, 
					Acts::GeometryContext& tgContext, 
					ActsTrk::TrackContainer& tracks,
					Acts::Result<ActsTrk::TrackContainer::TrackProxy, std::error_code>& fitResult) const;

  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", "ActsExtrapolationTool"};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", "ActsToTrkConverterTool"};
  ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool {this, "SummaryTool", "", "ToolHandle for track summary tool"};
  ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool {this, 
                                                           "BoundaryCheckTool", 
                                                           "InDet::InDetBoundaryCheckTool", 
                                                           "Boundary checking tool for detector sensitivities"};

    // the settable job options
  Gaudi::Property< double > m_option_outlierChi2Cut {this, "OutlierChi2Cut", 12.5, 
      "Chi2 cut used by the outlier finder" };
  Gaudi::Property< double > m_option_ReverseFilteringPt {this, "ReverseFilteringPt", 1.0 * Acts::UnitConstants::GeV,
      "Pt cut used for the ReverseFiltering logic"};
  Gaudi::Property< int > m_option_maxPropagationStep {this, "MaxPropagationStep", 5000, 
      "Maximum number of steps for one propagate call"};
  Gaudi::Property< double > m_option_seedCovarianceScale {this, "SeedCovarianceScale", 100.,
      "Scale factor for the input seed covariance when doing refitting"};

  Gaudi::Property<double> m_overstepLimit{this, "OverstepLimit", 100 * Acts::UnitConstants::mm, 
      "Overstep limit / tolerance for the Eigen stepper (use ACTS units!)"};

  /// Type erased track fitter function.
    using Fitter = Acts::KalmanFitter<Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>, traj_Type>;
    std::unique_ptr<Fitter> m_fitter;

    Acts::KalmanFitterExtensions<traj_Type> getExtensions();

    ATLASOutlierFinder m_outlierFinder{0};
    ReverseFilteringLogic m_reverseFilteringLogic{0};
    Acts::KalmanFitterExtensions<traj_Type> m_kfExtensions;

  /// Private access to the logger
  const Acts::Logger& logger() const {
    return *m_logger;
  }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

}; // end of namespace

#include "ActsKalmanFitter.icc"

#endif

