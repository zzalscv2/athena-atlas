/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSKALMANFITTER_H
#define ACTSGEOMETRY_ACTSKALMANFITTER_H

#include "ActsFitterHelperFunctions.h"

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
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

// PACKAGE

#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

// STL
#include <string>
#include <memory>//unique_ptr
#include <limits>//for numeric_limits
#include <cmath> //std::abs

class EventContext;

namespace Trk{
  class Track;
}

namespace ActsTrk {

class ActsKalmanFitter : public extends<AthAlgTool, Trk::ITrackFitter> { 
public:

  ActsKalmanFitter(const std::string&,const std::string&,const IInterface*);
  virtual ~ActsKalmanFitter() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;

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
    const Trk::Track& intrk1,
    const Trk::Track& intrk2,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;


  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

  // Create a track from the fitter result
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
    using Fitter = Acts::KalmanFitter<Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>, ActsTrk::TrackStateBackend>;
    std::unique_ptr<Fitter> m_fitter;

    Acts::KalmanFitterExtensions<ActsTrk::TrackStateBackend> getExtensions();
    Acts::KalmanFitterExtensions<ActsTrk::TrackStateBackend> m_kfExtensions;

    ActsTrk::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};
    ActsTrk::FitterHelperFunctions::ReverseFilteringLogic m_reverseFilteringLogic{0};

  /// Private access to the logger
  const Acts::Logger& logger() const {
    return *m_logger;
  }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

}; // end of namespace

}
#endif

