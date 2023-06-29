/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSGAUSSIANSUMFITTER_H
#define ACTSGEOMETRY_ACTSGAUSSIANSUMFITTER_H



// ATHENA 
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"

// ACTS
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/TrackFitting/BetheHeitlerApprox.hpp"
#include "Acts/TrackFitting/GsfOptions.hpp"
#include "Acts/Utilities/Logger.hpp"

// PACKAGE
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsFitterHelperFunctions.h"

// STL
#include <string>
#include <memory>

namespace ActsTrk {

class ActsGaussianSumFitter : public extends<AthAlgTool, Trk::ITrackFitter> {
public:
  
  ActsGaussianSumFitter(const std::string&, const std::string&, const IInterface*);
  virtual ~ActsGaussianSumFitter() = default;

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
  Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend> prepareOptions(const Acts::GeometryContext& tgContext,
									    const Acts::MagneticFieldContext& mfContext,
									    const Acts::CalibrationContext& calContext,
									    const Acts::PerigeeSurface& surface) const;
  
  std::unique_ptr<Trk::Track> performFit(const EventContext& ctx,
					 const Acts::GeometryContext& tgContext,
					 const Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend>& gsfOptions,
					 const std::vector<ATLASSourceLink>& trackSourceLinks,
					 const Acts::BoundTrackParameters& initialParams) const;

  // Create a track from the fitter result
  std::unique_ptr<Trk::Track> makeTrack(const EventContext& ctx, 
          const Acts::GeometryContext& tgContext, 
          ActsTrk::TrackContainer& tracks,
          Acts::Result<typename ActsTrk::TrackContainer::TrackProxy, std::error_code>& fitResult) const;

  const Acts::Experimental::GsfExtensions<ActsTrk::TrackStateBackend>& getExtensions() const;

  /// Private access to the logger
  const Acts::Logger& logger() const;

 private:
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
  Gaudi::Property< int > m_option_maxPropagationStep {this, "MaxPropagationStep", 5000, 
      "Maximum number of steps for one propagate call"};

  /// Type erased track fitter function.
  using Fitter = Acts::Experimental::GaussianSumFitter< Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator>,
                                                        Acts::Experimental::AtlasBetheHeitlerApprox<6, 5>,
                                                        ActsTrk::TrackStateBackend>;
  std::unique_ptr<Fitter> m_fitter;

  Acts::Experimental::GsfExtensions<ActsTrk::TrackStateBackend> m_gsfExtensions;

  ActsTrk::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

}; // end of namespace

}

#endif

