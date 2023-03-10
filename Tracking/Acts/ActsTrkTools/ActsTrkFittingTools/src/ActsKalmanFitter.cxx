/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsKalmanFitter.h"

// ATHENA
#include "GaudiKernel/ListItem.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "GaudiKernel/EventContext.h"

// ACTS
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ActsGeometryContext.h"
#include "ActsInterop/Logger.h"

// STL
#include <vector>



ActsKalmanFitter::ActsKalmanFitter(const std::string& t,const std::string& n,
                                const IInterface* p) :
  base_class(t,n,p)
{}

StatusCode ActsKalmanFitter::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_trkSummaryTool.retrieve());

  m_logger = makeActsAthenaLogger(this, "Acts Kalman Refit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();
  Acts::EigenStepper<> stepper(field, m_overstepLimit);
  Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometryTool->trackingGeometry() } );     
  Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator> propagator(std::move(stepper), 
								     std::move(navigator),
								     logger().cloneWithSuffix("Prop"));

  m_fitter = std::make_unique<Fitter>(std::move(propagator),
				      logger().cloneWithSuffix("KalmanFitter"));


  m_kfExtensions.updater.connect<&ActsKalmanFitter::gainMatrixUpdate<traj_Type>>();
  m_kfExtensions.smoother.connect<&ActsKalmanFitter::gainMatrixSmoother<traj_Type>>();
  m_kfExtensions.calibrator.connect<&ATLASSourceLinkCalibrator::calibrate<traj_Type>>();

  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;
  m_kfExtensions.outlierFinder.connect<&ATLASOutlierFinder::operator()<traj_Type>>(&m_outlierFinder);

  m_reverseFilteringLogic.momentumMax = m_option_ReverseFilteringPt;
  m_kfExtensions.reverseFilteringLogic.connect<&ReverseFilteringLogic::operator()<traj_Type>>(&m_reverseFilteringLogic);

  return StatusCode::SUCCESS;
}

// finalize
StatusCode ActsKalmanFitter::finalize()
{
  ATH_MSG_INFO ("finalize() successful in " << name());
  return StatusCode::SUCCESS;
}

// refit a track
// -------------------------------------------------------
std::unique_ptr<Trk::Track>
ActsKalmanFitter::fit(const EventContext& ctx,
                       const Trk::Track& inputTrack,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*prtHypothesis*/) const
{
  std::unique_ptr<Trk::Track> track = nullptr;
  ATH_MSG_VERBOSE ("--> enter KalmanFitter::fit(Track,,)    with Track from author = "
		   << inputTrack.info().dumpInfo());

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || inputTrack.measurementsOnTrack()->size() < 2) {
    ATH_MSG_INFO ("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_INFO ("input fails to provide track parameters for seeding the KF, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::PropagatorPlainOptions propagationOption;
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the KalmanFitter options
  Acts::KalmanFitterOptions
      kfOptions(tgContext, mfContext, calContext,
                m_kfExtensions,
		propagationOption,
                &(*pSurface));

  std::vector<ATLASSourceLink::ElementsType> elementCollection;

  std::vector<ATLASSourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext,inputTrack,elementCollection);
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_INFO("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return track;
  }

  const auto& initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters((*inputTrack.perigeeParameters()));

  // The covariance from already fitted track are too small and would result an incorect smoothing.
  // We scale up the input covaraiance to avoid this.
  Acts::BoundSymMatrix scaledCov = Acts::BoundSymMatrix::Identity();
  for (int i=0; i<6; ++i) {
    double scale = m_option_seedCovarianceScale;
    (scaledCov)(i,i) = scale * initialParams.covariance().value()(i,i);
  }

  const Acts::BoundTrackParameters scaledInitialParams(initialParams.referenceSurface().getSharedPtr(),
                                                       initialParams.parameters(),
                                                       scaledCov);

  Acts::TrackContainer tracks{
    Acts::VectorTrackContainer{},
    Acts::VectorMultiTrajectory{}};
  
  // Convert to Acts::SourceLink during iteration
  Acts::SourceLinkAdapterIterator begin{trackSourceLinks.begin()};
  Acts::SourceLinkAdapterIterator end{trackSourceLinks.end()};

  // Perform the fit
  auto result = m_fitter->fit(begin, end,			      
    scaledInitialParams, kfOptions, tracks);
  if (result.ok()) {
    track = makeTrack<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder>(ctx, tgContext, tracks, result);
  }
  return track;
}

// fit a set of MeasurementBase objects
// --------------------------------
std::unique_ptr<Trk::Track>
ActsKalmanFitter::fit(const EventContext& ctx,
                       const Trk::MeasurementSet& inputMeasSet,
                       const Trk::TrackParameters& estimatedStartParameters,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{
  std::unique_ptr<Trk::Track> track = nullptr;

  // protection against not having measurements on the input track
  if (inputMeasSet.size() < 2) {
    ATH_MSG_INFO ("called to refit empty measurement set or a measurement set with too little information, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::PropagatorPlainOptions propagationOption;
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the KalmanFitter options
  Acts::KalmanFitterOptions
      kfOptions(tgContext, mfContext, calContext,
                m_kfExtensions,
		propagationOption,
                &(*pSurface));

  std::vector<ATLASSourceLink> trackSourceLinks;
  trackSourceLinks.reserve(inputMeasSet.size());

  std::vector< ATLASSourceLink::ElementsType > elementCollection;
  elementCollection.reserve(inputMeasSet.size());

  for (auto it = inputMeasSet.begin(); it != inputMeasSet.end(); ++it){
    trackSourceLinks.push_back(m_ATLASConverterTool->trkMeasurementToSourceLink(tgContext, **it, elementCollection));
  }
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_INFO("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return track;
  }

  const auto& initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(estimatedStartParameters); 

  Acts::TrackContainer tracks{
    Acts::VectorTrackContainer{},
    Acts::VectorMultiTrajectory{}};

  // Convert to Acts::SourceLink during iteration
  Acts::SourceLinkAdapterIterator begin{trackSourceLinks.begin()};
  Acts::SourceLinkAdapterIterator end{trackSourceLinks.end()};

  // Perform the fit
  auto result = m_fitter->fit(begin, end,
    initialParams, kfOptions, tracks);
  if (result.ok()) {
    track = makeTrack<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder>(ctx, tgContext, tracks, result);
  }
  return track;
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
ActsKalmanFitter::fit(const EventContext& /*ctx*/,
                       const Trk::PrepRawDataSet& /*inputPRDColl*/,
                       const Trk::TrackParameters& /*estimatedStartParameters*/,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*prtHypothesis*/) const
{
  ATH_MSG_INFO ("Fit of PrepRawDataSet not yet implemented");
  return nullptr;
}

// extend a track fit to include an additional set of MeasurementBase objects
// re-implements the TrkFitterUtils/TrackFitter.cxx general code in a more
// mem efficient and stable way
// --------------------------------
std::unique_ptr<Trk::Track>
ActsKalmanFitter::fit(const EventContext& ctx,
                       const Trk::Track& inputTrack,
                       const Trk::MeasurementSet& addMeasColl,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{
  ATH_MSG_VERBOSE ("--> enter KalmanFitter::fit(Track,Meas'BaseSet,,)");
  ATH_MSG_VERBOSE ("    with Track from author = " << inputTrack.info().dumpInfo());

  // protection, if empty MeasurementSet
  if (addMeasColl.empty()) {
    ATH_MSG_WARNING( "client tries to add an empty MeasurementSet to the track fit." );
    return fit(ctx,inputTrack);
  }

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || (inputTrack.measurementsOnTrack()->size() < 2 && addMeasColl.empty())) {
    ATH_MSG_INFO ("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_INFO ("input fails to provide track parameters for seeding the KF, reject fit");
    return nullptr;
  }

   std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::PropagatorPlainOptions propagationOption;
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the KalmanFitter options
  Acts::KalmanFitterOptions
      kfOptions(tgContext, mfContext, calContext,
                m_kfExtensions,
		propagationOption,
                &(*pSurface));

  std::vector<ATLASSourceLink::ElementsType> elementCollection;

  std::vector<ATLASSourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, inputTrack, elementCollection);
  const auto& initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(inputTrack.perigeeParameters()));


  std::vector< ATLASSourceLink::ElementsType > atlasElementCollection;
  atlasElementCollection.reserve(addMeasColl.size());

  for (auto it = addMeasColl.begin(); it != addMeasColl.end(); ++it)
  {
    trackSourceLinks.push_back(m_ATLASConverterTool->trkMeasurementToSourceLink(tgContext, **it, atlasElementCollection));
  }
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_INFO("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return track;
  }

  Acts::TrackContainer tracks{
    Acts::VectorTrackContainer{},
    Acts::VectorMultiTrajectory{}};

  // Convert to Acts::SourceLink during iteration
  Acts::SourceLinkAdapterIterator begin{trackSourceLinks.begin()};
  Acts::SourceLinkAdapterIterator end{trackSourceLinks.end()};

  // Perform the fit
  auto result = m_fitter->fit(begin, end,
    initialParams, kfOptions, tracks);
  if (result.ok()) {
    track = makeTrack<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder>(ctx, tgContext, tracks, result);
  }
  return track;
}

// extend a track fit to include an additional set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
ActsKalmanFitter::fit(const EventContext& /*ctx*/,
                       const Trk::Track& /*inputTrack*/,
                       const Trk::PrepRawDataSet& /*addPrdColl*/,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{

  ATH_MSG_INFO ("Fit of Track with additional PrepRawDataSet not yet implemented");
  return nullptr;
}

// combined fit of two tracks
// --------------------------------
std::unique_ptr<Trk::Track>
ActsKalmanFitter::fit(const EventContext& ctx,
                       const Trk::Track& intrk1,
                       const Trk::Track& intrk2,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{
  ATH_MSG_VERBOSE ("--> enter KalmanFitter::fit(Track,Track,)");
  ATH_MSG_VERBOSE ("    with Tracks from #1 = " << intrk1.info().dumpInfo()
                   << " and #2 = " << intrk2.info().dumpInfo());

  // protection, if empty track2
  if (!intrk2.measurementsOnTrack()) {
    ATH_MSG_WARNING( "input #2 is empty try to fit track 1 alone" );
    return fit(ctx,intrk1);
  }

  // protection, if empty track1
  if (!intrk1.measurementsOnTrack()) {
    ATH_MSG_WARNING( "input #1 is empty try to fit track 2 alone" );
    return fit(ctx,intrk2);
  }

  // protection against not having track parameters on the input track
  if (!intrk1.trackParameters() || intrk1.trackParameters()->empty()) {
    ATH_MSG_INFO ("input #1 fails to provide track parameters for seeding the KF, reject fit");
    return nullptr;
  }

   std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::PropagatorPlainOptions propagationOption;
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the KalmanFitter options
  Acts::KalmanFitterOptions
      kfOptions(tgContext, mfContext, calContext,
                m_kfExtensions,
		propagationOption,
                &(*pSurface));

  std::vector<ATLASSourceLink::ElementsType> elementCollection1;
  std::vector<ATLASSourceLink::ElementsType> elementCollection2;

  std::vector<ATLASSourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, intrk1, elementCollection1);
  std::vector<ATLASSourceLink> trackSourceLinks2 = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, intrk2, elementCollection2);
  trackSourceLinks.insert(trackSourceLinks.end(), trackSourceLinks2.begin(), trackSourceLinks2.end());
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_INFO("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return track;
  }

  const auto &initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(intrk1.perigeeParameters()));

  // The covariance from already fitted track are too small and would result an incorect smoothing.
  // We scale up the input covaraiance to avoid this.
  Acts::BoundSymMatrix scaledCov = Acts::BoundSymMatrix::Identity();
  for (int i=0; i<6; ++i) {
    double scale = m_option_seedCovarianceScale;
    (scaledCov)(i,i) = scale * initialParams.covariance().value()(i,i);
  }

  const Acts::BoundTrackParameters scaledInitialParams(initialParams.referenceSurface().getSharedPtr(),
                                                       initialParams.parameters(),
                                                       scaledCov);


  Acts::TrackContainer tracks{
    Acts::VectorTrackContainer{},
    Acts::VectorMultiTrajectory{}};

  // Convert to Acts::SourceLink during iteration
  Acts::SourceLinkAdapterIterator begin{trackSourceLinks.begin()};
  Acts::SourceLinkAdapterIterator end{trackSourceLinks.end()};

  // Perform the fit
  auto result = m_fitter->fit(begin, end,
    scaledInitialParams, kfOptions, tracks);
  if (result.ok()) {
    track = makeTrack<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder>(ctx, tgContext, tracks, result);
  }
  return track;
}

