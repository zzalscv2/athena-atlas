/*§
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GaussianSumFitter.cxx
 * @begin         Monday 7th March 2005
 * @author        Anthony Morley, Tom  Atkinson, Christos Anastopoulos
 * @brief         Implementation code for Gaussian Sum Fitter class
 */

#include "TrkGaussianSumFilter/GaussianSumFitter.h"
#include "TrkGaussianSumFilter/IMultiStateExtrapolator.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateCombiner.h"
//
#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventUtils/MeasurementBaseComparisonFunction.h"
#include "TrkEventUtils/PrepRawDataComparisonFunction.h"
#include "TrkMaterialOnTrack/EstimatedBremOnTrack.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
//
#include <algorithm>
#include <vector>

namespace {

std::unique_ptr<Trk::FitQuality>
buildFitQuality(const Trk::SmoothedTrajectory& smoothedTrajectory)
{

  /*
   * Build fit quality by summing the chi2 and ndof for each
   * measurements
   */
  double chiSquared = 0.;
  int numberDoF = -5;
  Trk::SmoothedTrajectory::const_iterator stateOnSurface =
    smoothedTrajectory.begin();
  for (; stateOnSurface != smoothedTrajectory.end(); ++stateOnSurface) {
    if (!(*stateOnSurface)->type(Trk::TrackStateOnSurface::Measurement)) {
      continue;
    }
    if (!(*stateOnSurface)->fitQualityOnSurface()) {
      continue;
    }
    chiSquared += (*stateOnSurface)->fitQualityOnSurface().chiSquared();
    numberDoF += (*stateOnSurface)->fitQualityOnSurface().numberDoF();
  }
  if (std::isnan(chiSquared) || chiSquared <= 0.) {
    return nullptr;
  }
  return std::make_unique<Trk::FitQuality>(chiSquared, numberDoF);
}

/*
 * Helper to return the MultiComponent TSOS
 * that we will push back to the Trajectory DataVector
 * (taking ownership)
 */
Trk::MultiComponentStateOnSurface*
smootherHelper(Trk::MultiComponentState&& updatedState,
               std::unique_ptr<const Trk::MeasurementBase>&& measurement,
               const Trk::FitQualityOnSurface& fitQuality,
               bool islast,
               bool useMode)

{
  // If is the last we want also to combine
  // the Multi component state in a single TrackParameter
  if (islast) {
    auto combinedLastState =
      Trk::MultiComponentStateCombiner::combineToSingle(updatedState, useMode);
    if (combinedLastState) {
      return new Trk::MultiComponentStateOnSurface(fitQuality,
                                                   std::move(measurement),
                                                   std::move(combinedLastState),
                                                   std::move(updatedState));
    }
  }
  return new Trk::MultiComponentStateOnSurface(
    fitQuality, std::move(measurement), std::move(updatedState));
}

} // end of anonymous namespace

Trk::GaussianSumFitter::GaussianSumFitter(const std::string& type,
                                          const std::string& name,
                                          const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_directionToPerigee(Trk::oppositeMomentum)
  , m_trkParametersComparisonFunction{}
  , m_inputPreparator(nullptr)
  , m_sortingReferencePoint{ 0, 0, 0 }
  , m_FitPRD{ 0 }
  , m_FitMeasurementBase{ 0 }
  , m_fitSuccess{ 0 }
{
  declareInterface<ITrackFitter>(this);
  declareProperty("SortingReferencePoint", m_sortingReferencePoint);
}

StatusCode
Trk::GaussianSumFitter::initialize()
{

  if (m_maximumNumberOfComponents > GSFConstants::maxNumberofStateComponents) {
    ATH_MSG_FATAL("Requested MaximumNumberOfComponents > "
                  << GSFConstants::maxNumberofStateComponents);
    return StatusCode::FAILURE;
  }

  // GSF extrapolator
  ATH_CHECK(m_extrapolator.retrieve());

  // We need a to create RIO_OnTrack (calibrated/corrected)
  // measurements only if we start from PrePRawData.
  if (!m_refitOnMeasurementBase) {
    ATH_CHECK(m_rioOnTrackCreator.retrieve());
  } else {
    m_rioOnTrackCreator.disable();
  }
  // Initialise the closest track parameters search algorithm
  Amg::Vector3D referencePosition(m_sortingReferencePoint[0],
                                  m_sortingReferencePoint[1],
                                  m_sortingReferencePoint[2]);

  m_trkParametersComparisonFunction =
    Trk::TrkParametersComparisonFunction(referencePosition);
  m_inputPreparator = std::make_unique<TrackFitInputPreparator>();

  return StatusCode::SUCCESS;
}

StatusCode
Trk::GaussianSumFitter::finalize()
{
  ATH_MSG_INFO(
      '\n'
      << "            Some Brief GSF Statistics          " << '\n'
      << "-----------------------------------------------" << '\n'
      << "Number of Fit PrepRawData Calls:          " << m_FitPRD << '\n'
      << "Number of Fit MeasurementBase Calls:      " << m_FitMeasurementBase
      << '\n'
      << "Number of successful fits:                " << m_fitSuccess << '\n'
      << '\n'
      << "-----------------------------------------------" << '\n');
  return StatusCode::SUCCESS;
}

/*
 * Interface Method
 * Refitting of an input track.
 * Extract the measurementBase or
 * PrepRawData and refit.
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(
  const EventContext& ctx,
  const Trk::Track& inputTrack,
  const Trk::RunOutlierRemoval outlierRemoval,
  const Trk::ParticleHypothesis particleHypothesis) const
{
  // Check that the input track has well defined parameters
  if (inputTrack.trackParameters()->empty()) {
    ATH_MSG_FATAL("No track parameters on Track!");
    return nullptr;
  }
  // Check that the input track has associated MeasurementBase objects
  if (inputTrack.trackStateOnSurfaces()->empty()) {
    ATH_MSG_FATAL("Empty MeasurementBase ");
    return nullptr;
  }
  // Retrieve the set of track parameters closest to the reference point
  const Trk::TrackParameters* parametersNearestReference =
    *(std::min_element(inputTrack.trackParameters()->begin(),
                       inputTrack.trackParameters()->end(),
                       m_trkParametersComparisonFunction));

  // If we refit using measurement base then
  // extract the measurements from the input track
  // and fit them
  if (m_refitOnMeasurementBase) {

    MeasurementSet measurementSet;

    for (const auto* tsos : *(inputTrack.trackStateOnSurfaces())) {

      if (!(tsos)) {
        ATH_MSG_WARNING("Track contains an empty MeasurementBase object ");
        continue;
      }

      const MeasurementBase* meas = tsos->measurementOnTrack();
      if (meas) {
        if (tsos->type(TrackStateOnSurface::Measurement)) {
          measurementSet.push_back(meas);
        } else if (m_reintegrateOutliers &&
                   tsos->type(TrackStateOnSurface::Outlier)) {
          measurementSet.push_back(meas);
        }
      }
    }

    return fit(ctx,
               measurementSet,
               *parametersNearestReference,
               outlierRemoval,
               particleHypothesis);
  }

  // If we refit using PrepRawData level then
  // extract the the PrepRawData  from the input track
  // and fit them
  PrepRawDataSet prepRawDataSet;

  for (const auto* meas : *(inputTrack.measurementsOnTrack())) {

    if (!meas) {
      continue;
    }
    const Trk::RIO_OnTrack* rioOnTrack = nullptr;
    if (meas->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
      rioOnTrack = static_cast<const Trk::RIO_OnTrack*>(meas);
    }
    if (!rioOnTrack) {
      continue;
    }
    const PrepRawData* prepRawData = rioOnTrack->prepRawData();
    if (!prepRawData) {
      continue;
    }
    prepRawDataSet.push_back(prepRawData);
  }

  return fit(ctx,
             prepRawDataSet,
             *parametersNearestReference,
             outlierRemoval,
             particleHypothesis);
}

/*
 * Interface Method
 * Fitting of a set of PrepRawData objects
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(
  const EventContext& ctx,
  const Trk::PrepRawDataSet& prepRawDataSet,
  const Trk::TrackParameters& estimatedParametersNearOrigin,
  const Trk::RunOutlierRemoval /* Not used*/,
  const Trk::ParticleHypothesis particleHypothesis) const
{
  m_FitPRD.fetch_add(1, std::memory_order_relaxed);

  // Protect against empty PrepRawDataSet object
  if (prepRawDataSet.empty()) {
    ATH_MSG_FATAL("PrepRawData set for fit is empty");
    return nullptr;
  }

  // We need a sorted PrepRawDataSet
  Trk::PrepRawDataSet sortedPrepRawDataSet = PrepRawDataSet(prepRawDataSet);
  Trk::PrepRawDataComparisonFunction prdComparisonFunction =
    Trk::PrepRawDataComparisonFunction(
      estimatedParametersNearOrigin.position(),
      estimatedParametersNearOrigin.momentum());

  std::sort(sortedPrepRawDataSet.begin(),
            sortedPrepRawDataSet.end(),
            prdComparisonFunction);

  // Create Extrapolator cache that holds material effects cache;
  Trk::IMultiStateExtrapolator::Cache extrapolatorCache;
  // Perform GSF forward fit
  MultiTrajectory forwardTrajectory = fitPRD(ctx,
                                             extrapolatorCache,
                                             sortedPrepRawDataSet,
                                             estimatedParametersNearOrigin,
                                             particleHypothesis);

  if (forwardTrajectory.empty()) {
    return nullptr;
  }
  // Perform GSF smoother operation
  MultiTrajectory smoothedTrajectory =
    smootherFit(ctx, extrapolatorCache, forwardTrajectory, particleHypothesis);
  if (smoothedTrajectory.empty()) {
    return nullptr;
  }

  // Fit quality
  std::unique_ptr<FitQuality> fitQuality = buildFitQuality(smoothedTrajectory);
  if (!fitQuality) {
    return nullptr;
  }

  // Create parameters at perigee if needed
  auto perigeeMultiStateOnSurface =
    makePerigee(ctx, extrapolatorCache, smoothedTrajectory, particleHypothesis);
  if (perigeeMultiStateOnSurface) {
    smoothedTrajectory.push_back(perigeeMultiStateOnSurface.release());
  } else {
    return nullptr;
  }

  // Reverse the order of the TSOS's after the smoother
  // to make be order flow from inside to out
  std::reverse(smoothedTrajectory.begin(), smoothedTrajectory.end());

  // Create Trk::Track
  Trk::TrackInfo info(Trk::TrackInfo::GaussianSumFilter, particleHypothesis);
  info.setTrackProperties(TrackInfo::BremFit);
  info.setTrackProperties(TrackInfo::BremFitSuccessful);
  m_fitSuccess.fetch_add(1, std::memory_order_relaxed);
  return std::make_unique<Track>(
    info, std::move(smoothedTrajectory), std::move(fitQuality));
}

/*
 * Interface method
 * Fitting of a set of MeasurementBase objects
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(
  const EventContext& ctx,
  const Trk::MeasurementSet& measurementSet,
  const Trk::TrackParameters& estimatedParametersNearOrigin,
  const Trk::RunOutlierRemoval /* Not used*/,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  m_FitMeasurementBase.fetch_add(1, std::memory_order_relaxed);
  // Protect against empty PrepRawDataSet object
  if (measurementSet.empty()) {
    ATH_MSG_FATAL("MeasurementSet for fit is empty");
    return nullptr;
  }

  // We need to separate the possible CaloCluster on Track
  // measurement from the rest
  const Trk::CaloCluster_OnTrack* ccot(nullptr);

  Trk::MeasurementSet cleanedMeasurementSet;
  cleanedMeasurementSet.reserve(measurementSet.size());

  for (const auto* meas : measurementSet) {
    if (!meas) {
      ATH_MSG_WARNING(
        "There is an empty MeasurementBase object in the track! ");
      continue;
    }
    if (meas->type(Trk::MeasurementBaseType::CaloCluster_OnTrack)) {
      ccot = static_cast<const Trk::CaloCluster_OnTrack*>(meas);
    } else {
      cleanedMeasurementSet.push_back(meas);
    }
  }

  // We need a sorted measurement set
  Trk::MeasurementSet sortedMeasurementSet =
    MeasurementSet(cleanedMeasurementSet);

  Trk::MeasurementBaseComparisonFunction measurementBaseComparisonFunction(
    estimatedParametersNearOrigin.position(),
    estimatedParametersNearOrigin.momentum());
  sort(sortedMeasurementSet.begin(),
       sortedMeasurementSet.end(),
       measurementBaseComparisonFunction);

  // Create Extrapolator cache that holds material effects cache;
  Trk::IMultiStateExtrapolator::Cache extrapolatorCache;

  // Perform GSF forwards fit
  MultiTrajectory forwardTrajectory =
    fitMeasurements(ctx,
                    extrapolatorCache,
                    sortedMeasurementSet,
                    estimatedParametersNearOrigin,
                    particleHypothesis);

  if (forwardTrajectory.empty()) {
    return nullptr;
  }

  // Perform GSF smoother operation
  MultiTrajectory smoothedTrajectory = smootherFit(
    ctx, extrapolatorCache, forwardTrajectory, particleHypothesis, ccot);
  if (smoothedTrajectory.empty()) {
    return nullptr;
  }

  // fit quality
  std::unique_ptr<FitQuality> fitQuality = buildFitQuality(smoothedTrajectory);
  if (!fitQuality) {
    return nullptr;
  }

  // Create parameters at perigee if needed
  auto perigeeMultiStateOnSurface =
    makePerigee(ctx, extrapolatorCache, smoothedTrajectory, particleHypothesis);
  if (perigeeMultiStateOnSurface) {
    smoothedTrajectory.push_back(perigeeMultiStateOnSurface.release());
  } else {
    return nullptr;
  }

  // Reverse the order of the TSOS's to make be order flow from inside to out
  std::reverse(smoothedTrajectory.begin(), smoothedTrajectory.end());

  // Create track
  Trk::TrackInfo info(Trk::TrackInfo::GaussianSumFilter, particleHypothesis);
  info.setTrackProperties(TrackInfo::BremFit);
  info.setTrackProperties(TrackInfo::BremFitSuccessful);
  m_fitSuccess.fetch_add(1, std::memory_order_relaxed);
  return std::make_unique<Track>(
    info, std::move(smoothedTrajectory), std::move(fitQuality));
}

/*
 * Interface method
 * Refit a track adding a PrepRawData  set
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(const EventContext& ctx,
                            const Track& intrk,
                            const PrepRawDataSet& addPrdColl,
                            const RunOutlierRemoval runOutlier,
                            const ParticleHypothesis matEffects) const
{
  // protection, if empty PrepRawDataSet
  if (addPrdColl.empty()) {
    ATH_MSG_WARNING(
      "client tries to add an empty PrepRawDataSet to the track fit.");
    return fit(ctx, intrk, runOutlier, matEffects);
  }
  // determine the Track Parameter which is the start of the trajectory
  const TrackParameters* estimatedStartParameters =
    *(std::min_element(intrk.trackParameters()->begin(),
                       intrk.trackParameters()->end(),
                       m_trkParametersComparisonFunction));

  // use external preparator class to prepare PRD set for fitter interface
  PrepRawDataSet PRDColl = Trk::TrackFitInputPreparator::stripPrepRawData(
    intrk, addPrdColl, false, true);

  // delegate to fitting PrepRawData interface method
  return fit(ctx, PRDColl, *estimatedStartParameters, runOutlier, matEffects);
}

/*
 *  Interface method
 *  Refit a track adding a RIO_OnTrack set
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(const EventContext& ctx,
                            const Track& inputTrack,
                            const MeasurementSet& measurementSet,
                            const RunOutlierRemoval runOutlier,
                            const ParticleHypothesis matEffects) const
{

  // protection, if empty MeasurementSet
  if (measurementSet.empty()) {
    ATH_MSG_WARNING(
      "Client tries to add an empty MeasurementSet to the track fit.");
    return fit(ctx, inputTrack, runOutlier, matEffects);
  }
  // Check that the input track has well defined parameters
  if (inputTrack.trackParameters()->empty()) {
    ATH_MSG_FATAL("No estimation of track parameters near origin!");
    return nullptr;
  }
  // Check that the input track has associated MeasurementBase objects
  if (inputTrack.trackStateOnSurfaces()->empty()) {
    ATH_MSG_FATAL("Attempting to fit track to empty MeasurementBase "
                  "collection!");
    return nullptr;
  }
  // Retrieve the set of track parameters closest to the reference point
  const Trk::TrackParameters* parametersNearestReference =
    *(std::min_element(inputTrack.trackParameters()->begin(),
                       inputTrack.trackParameters()->end(),
                       m_trkParametersComparisonFunction));

  MeasurementSet combinedMS = m_inputPreparator->stripMeasurements(
    inputTrack, measurementSet, true, false);

  // delegate to  measurementBase fit method
  return fit(
    ctx, combinedMS, *parametersNearestReference, runOutlier, matEffects);
}

/*
 * Interface method
 * Combine two tracks by refitting
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(const EventContext& ctx,
                            const Track& intrk1,
                            const Track& intrk2,
                            const RunOutlierRemoval runOutlier,
                            const ParticleHypothesis matEffects) const
{
  // Just add the hits on tracks
  // protection against not having measurements on the input tracks
  if (!intrk1.trackStateOnSurfaces() || !intrk2.trackStateOnSurfaces() ||
      intrk1.trackStateOnSurfaces()->size() < 2) {
    ATH_MSG_WARNING("called to refit empty track or track with too little "
                    "information, reject fit");
    return nullptr;
  }
  if (!intrk1.trackParameters() || intrk1.trackParameters()->empty()) {
    ATH_MSG_WARNING("input #1 fails to provide track parameters for "
                    "seeding the GXF, reject fit");
    return nullptr;
  }

  const TrackParameters* minPar = *(intrk1.trackParameters()->begin());
  Trk::MeasurementSet ms;
  for (const auto* tsos : *(intrk1.trackStateOnSurfaces())) {
    if (!(tsos->type(Trk::TrackStateOnSurface::Measurement) ||
          tsos->type(Trk::TrackStateOnSurface::Outlier))) {
      continue;
    }

    if (tsos->measurementOnTrack()->type(
          Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
      continue;
    }

    ms.push_back(tsos->measurementOnTrack());
  }

  for (const auto* tsos : *(intrk2.trackStateOnSurfaces())) {

    if (!(tsos->type(Trk::TrackStateOnSurface::Measurement) ||
          tsos->type(Trk::TrackStateOnSurface::Outlier))) {
      continue;
    }

    if (tsos->measurementOnTrack()->type(
          Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
      continue;
    }
    ms.push_back(tsos->measurementOnTrack());
  }
  // call measurement base interface
  return fit(ctx, ms, *minPar, runOutlier, matEffects);
}

/*
 * Helper creating a multicomponent
 * perigee TrackStateOnSurface
 */
std::unique_ptr<Trk::MultiComponentStateOnSurface>
Trk::GaussianSumFitter::makePerigee(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const MultiTrajectory& smoothedTrajectory,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  // Start at the end of the smoothed trajectory
  // we should be the closest to perigee/origin.
  const Trk::MultiComponentStateOnSurface*
    multiComponentStateOnSurfaceNearestOrigin = smoothedTrajectory.back();
  const Trk::MultiComponentState* multiComponentState =
    &(multiComponentStateOnSurfaceNearestOrigin->components());

  // Extrapolate to perigee
  const Trk::PerigeeSurface perigeeSurface;
  Trk::MultiComponentState stateExtrapolatedToPerigee =
    m_extrapolator->extrapolate(ctx,
                                extrapolatorCache,
                                *multiComponentState,
                                perigeeSurface,
                                m_directionToPerigee,
                                false,
                                particleHypothesis);

  if (stateExtrapolatedToPerigee.empty()) {
    return nullptr;
  }

  // Determine the combined state
  std::unique_ptr<Trk::TrackParameters> combinedPerigee =
    MultiComponentStateCombiner::combineToSingle(stateExtrapolatedToPerigee, m_useMode);

  // Perigee is an additional MultiComponentStateOnSurface
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
    pattern(0);
  pattern.set(Trk::TrackStateOnSurface::Perigee);

  if (std::abs(combinedPerigee->parameters()[Trk::qOverP]) > 1e8) {
    ATH_MSG_ERROR(
      "makePerigee() about to return with 0 momentum!! Returning null instead");
    return nullptr;
  }

  return std::make_unique<MultiComponentStateOnSurface>(
    Trk::FitQualityOnSurface{},
    nullptr,
    std::move(combinedPerigee),
    std::move(stateExtrapolatedToPerigee),
    nullptr,
    pattern);
}

/*
 * Private method for
 * Forward fit on a  set of PrepRawData
 */
Trk::GaussianSumFitter::MultiTrajectory
Trk::GaussianSumFitter::fitPRD(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const Trk::PrepRawDataSet& inputPrepRawDataSet,
  const Trk::TrackParameters& estimatedTrackParametersNearOrigin,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  // Extract PrepRawDataSet into new local object and check that the PrepRawData
  // is associated with a detector element
  Trk::PrepRawDataSet prepRawDataSet;
  for (const auto* rawData : inputPrepRawDataSet) {
    if (!(rawData->detectorElement())) {
      ATH_MSG_WARNING("PrepRawData has no Element link... disregard it");
    } else {
      prepRawDataSet.push_back(rawData);
    }
  }
  // For starting guess a multicompoment state
  // that has single component with weight 1
  const AmgVector(5)& par = estimatedTrackParametersNearOrigin.parameters();
  Trk::ComponentParameters componentParametersNearOrigin(
    estimatedTrackParametersNearOrigin.associatedSurface()
      .createUniqueTrackParameters(par[Trk::loc1],
                                   par[Trk::loc2],
                                   par[Trk::phi],
                                   par[Trk::theta],
                                   par[Trk::qOverP],
                                   std::nullopt /*no errors*/),
    1.);

  Trk::MultiComponentState multiComponentStateNearOrigin{};
  multiComponentStateNearOrigin.push_back(
    std::move(componentParametersNearOrigin));

  // Create new trajectory
  MultiTrajectory forwardTrajectory{};
  forwardTrajectory.reserve(prepRawDataSet.size());
  for (const auto* prepRawData : prepRawDataSet) {
    // Every step the ForwardTrajectory is updated
    bool stepIsValid = stepForwardFit(
      ctx,
      extrapolatorCache,
      forwardTrajectory,
      prepRawData,
      nullptr,
      prepRawData->detectorElement()->surface(prepRawData->identify()),
      multiComponentStateNearOrigin,
      particleHypothesis);

    if (!stepIsValid) {
      return MultiTrajectory{};
    }
  }
  return forwardTrajectory;
}

/*
 * Private method for
 * forward fit on a set of Measurements
 */
Trk::GaussianSumFitter::MultiTrajectory
Trk::GaussianSumFitter::fitMeasurements(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const Trk::MeasurementSet& inputMeasurementSet,
  const Trk::TrackParameters& estimatedTrackParametersNearOrigin,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  if (inputMeasurementSet.empty()) {
    ATH_MSG_ERROR("fitMeasurements: Input MeasurementSet is empty!");
    return MultiTrajectory{};
  }
  // For starting guess a multicompoment state
  // that has single component with weight 1
  const AmgVector(5)& par = estimatedTrackParametersNearOrigin.parameters();
  Trk::ComponentParameters componentParametersNearOrigin(
    estimatedTrackParametersNearOrigin.associatedSurface()
      .createUniqueTrackParameters(par[Trk::loc1],
                                   par[Trk::loc2],
                                   par[Trk::phi],
                                   par[Trk::theta],
                                   par[Trk::qOverP],
                                   std::nullopt /*no errors*/),
    1.);
  Trk::MultiComponentState multiComponentStateNearOrigin{};
  multiComponentStateNearOrigin.push_back(
    std::move(componentParametersNearOrigin));

  MultiTrajectory forwardTrajectory{};
  forwardTrajectory.reserve(inputMeasurementSet.size());
  for (const auto* measurement : inputMeasurementSet) {
    // Every step the ForwardTrajectory is updated
    bool stepIsValid = stepForwardFit(ctx,
                                      extrapolatorCache,
                                      forwardTrajectory,
                                      nullptr,
                                      measurement,
                                      measurement->associatedSurface(),
                                      multiComponentStateNearOrigin,
                                      particleHypothesis);

    if (!stepIsValid) {
      return MultiTrajectory{};
    }
  }
  return forwardTrajectory;
}

/*
 * Actual
 * Implementation method for StepForwardFit
 */
bool
Trk::GaussianSumFitter::stepForwardFit(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  MultiTrajectory& forwardTrajectory,
  const Trk::PrepRawData* originalPrepRawData,
  const Trk::MeasurementBase* originalMeasurement,
  const Trk::Surface& surface,
  Trk::MultiComponentState& updatedState,
  const Trk::ParticleHypothesis particleHypothesis) const
{
  // Protect against undefined Measurement or PrepRawData
  if (!originalPrepRawData && !originalMeasurement) {
    ATH_MSG_WARNING("No measurement base or PrepRawData passed to "
                    "StepForwardFit!");
    return false;
  }

  if (!originalMeasurement && m_refitOnMeasurementBase) {
    ATH_MSG_WARNING("No measurement base information passed to StepForwardFit");
    return false;
  }
  // Propagate the multi-component state to the next measurement surface
  // accounting for the material effects. This gives us
  // the prediction at that surface.
  Trk::MultiComponentState extrapolatedState =
    m_extrapolator->extrapolate(ctx,
                                extrapolatorCache,
                                updatedState,
                                surface,
                                Trk::alongMomentum,
                                false,
                                particleHypothesis);
  if (extrapolatedState.empty()) {
    return false;
  }

  // we need to account for either measurement base input
  // or PrepRawData input.
  std::unique_ptr<const Trk::MeasurementBase> measurement = nullptr;
  if (originalMeasurement) { // clone original MeasurementBase object
    measurement.reset(originalMeasurement->clone());
  } else {
    std::unique_ptr<Trk::TrackParameters> combinedState =
      MultiComponentStateCombiner::combineToSingle(extrapolatedState);
    if (!combinedState) {
      ATH_MSG_WARNING("State combination failed... exiting");
      return false;
    }
    // Create a new MeasurementBase object from PrepRawData
    measurement.reset(
      m_rioOnTrackCreator->correct(*originalPrepRawData, *combinedState));
    combinedState.reset();
  }
  if (!measurement) {
    ATH_MSG_WARNING("stepForwardFit no measurement to update with");
    return false;
  }

  // Perform measurement update
  auto fitQuality = Trk::FitQualityOnSurface{};
  // We need to keep the extrapolatedState so clone
  updatedState = Trk::GsfMeasurementUpdator::update(
    MultiComponentStateHelpers::clone(extrapolatedState),
    *measurement,
    fitQuality);

  if (updatedState.empty()) {
    return false;
  }

  // Hits with excessive chi2 are outliers.
  // We ingore the update, reset back to the extrapolated
  // state before the update
  if (fitQuality.chiSquared() >
      m_cutChiSquaredPerNumberDOF * fitQuality.numberDoF()) {
    fitQuality = FitQualityOnSurface(1, 1);
    std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> type(0);
    type.set(TrackStateOnSurface::Outlier);
    const Trk::MultiComponentStateOnSurface* multiComponentStateOnSurface =
      new MultiComponentStateOnSurface(
        fitQuality,
        std::move(measurement),
        // used below for the updated state so clone
        Trk::MultiComponentStateHelpers::clone(extrapolatedState),
        nullptr,
        type);
    forwardTrajectory.push_back(multiComponentStateOnSurface);
    // reset the updated state to the extrapolated state
    // before the measurement update
    updatedState = std::move(extrapolatedState);
  } else {
    const Trk::MultiComponentStateOnSurface* multiComponentStateOnSurface =
      new MultiComponentStateOnSurface(
        fitQuality, std::move(measurement), std::move(extrapolatedState));
    forwardTrajectory.push_back(multiComponentStateOnSurface);
  }
  return true;
}

/*
 * Actual
 * Implementation of the smoothing of the trajectory.
 */
Trk::GaussianSumFitter::MultiTrajectory
Trk::GaussianSumFitter::smootherFit(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const MultiTrajectory& forwardTrajectory,
  const ParticleHypothesis particleHypothesis,
  const Trk::CaloCluster_OnTrack* ccot) const
{
  if (forwardTrajectory.empty()) {
    ATH_MSG_ERROR(
      "Attempting to smooth an empty forward trajectory... Exiting!");
    return MultiTrajectory();
  }

  auto smoothedTrajectory = MultiTrajectory();
  smoothedTrajectory.reserve(forwardTrajectory.size());
  // For the smoother we start from the end and we go
  // to begin. We need to find the the first track
  // state on surface  in this reverse direction
  MultiTrajectory::const_reverse_iterator trackStateOnSurface =
    forwardTrajectory.rbegin();
  for (; trackStateOnSurface != forwardTrajectory.rend();
       ++trackStateOnSurface) {
    if (!(*trackStateOnSurface)->type(TrackStateOnSurface::Measurement)) {
      smoothedTrajectory.push_back((*trackStateOnSurface)->clone());
    } else {
      break;
    }
  }
  // This is the 1st track state on surface for a measurement
  // in the reverse direction. Our starting point for the smoother
  const MultiComponentStateOnSurface* smootherPredictionMultiStateOnSurface =
    *trackStateOnSurface;
  MultiComponentState smootherPredictionMultiState =
    MultiComponentStateHelpers::clone(
      (smootherPredictionMultiStateOnSurface->components()));

  // Perform the  update with the measurement and create the
  // the 1st updated/smoothed entry in the trajectory
  Trk::FitQualityOnSurface fitQuality;
  std::unique_ptr<Trk::MeasurementBase> firstSmootherMeasurementOnTrack =
    smootherPredictionMultiStateOnSurface->measurementOnTrack()->uniqueClone();

  if (!firstSmootherMeasurementOnTrack) {
    ATH_MSG_WARNING(
      "Initial state on surface in smoother does not have an associated "
      "MeasurementBase object");
    return MultiTrajectory();
  }
  Trk::MultiComponentState firstSmoothedState =
    Trk::GsfMeasurementUpdator::update(std::move(smootherPredictionMultiState),
                                       *firstSmootherMeasurementOnTrack,
                                       fitQuality);
  if (firstSmoothedState.empty()) {
    return MultiTrajectory();
  }

  if (!MultiComponentStateHelpers::allHaveCovariance(firstSmoothedState)) {
    ATH_MSG_WARNING(
      "Not all components have covariance. Rejecting smoothed state.");
    return MultiTrajectory();
  }
  // The last TSOS is special so we do a proper collapse
  // of the multi component to single TrackParameter
  std::unique_ptr<Trk::TrackParameters> combinedFirstSmoothedState =
    MultiComponentStateCombiner::combineToSingle(firstSmoothedState, m_useMode);

  // The 1st updated/smoothed state owned  by the trajectory
  const Trk::MultiComponentStateOnSurface* updatedFirstStateOnSurface =
    new MultiComponentStateOnSurface(
      fitQuality,
      std::move(firstSmootherMeasurementOnTrack),
      std::move(combinedFirstSmoothedState),
      MultiComponentStateHelpers::clone(firstSmoothedState));
  smoothedTrajectory.push_back(updatedFirstStateOnSurface);

  // Generate a prediction by scaling the covariance of all components in the
  // first smoothed state and perform a measurement update to it.
  // This way there is no
  // dependance on error of prediction NB local Y and theta are not blown out
  // too much to help in the TRT.
  Trk::MultiComponentState smoothedStateWithScaledError =
    MultiComponentStateHelpers::WithScaledError(
      std::move(firstSmoothedState), 15., 5., 15., 5., 15.);
  Trk::FitQualityOnSurface fitQualityWithScaledErrors;
  Trk::MultiComponentState updatedState = Trk::GsfMeasurementUpdator::update(
    std::move(smoothedStateWithScaledError),
    *(updatedFirstStateOnSurface->measurementOnTrack()),
    fitQualityWithScaledErrors);
  if (updatedState.empty()) {
    ATH_MSG_WARNING("Smoother prediction could not be determined");
    return MultiTrajectory();
  }
  // continue the reverse looping of the TrackStateOnSurfaces
  // in the forward trajectory
  ++trackStateOnSurface;
  // The is the last one we will see
  auto lasttrackStateOnSurface = forwardTrajectory.rend() - 1;
  // TSOS that the cluster measuremenet will added on.
  auto secondLastTrackStateOnSurface = forwardTrajectory.rend() - 2;
  // This will be the next tsos that we will push to the
  // smoothedTrajectory DataVector (that takes ownership)
  Trk::MultiComponentStateOnSurface* updatedStateOnSurface = nullptr;
  // loopUpdatedState is a ptr to the most recent
  // predicted MultiComponentState
  // we start from our previous inflated prediction
  Trk::MultiComponentState* loopUpdatedState = &updatedState;

  for (; trackStateOnSurface != forwardTrajectory.rend();
       ++trackStateOnSurface) {
    // Retrieve the MeasurementBase object from the TrackStateOnSurface object
    const Trk::MeasurementBase* measurement_in =
      (*trackStateOnSurface)->measurementOnTrack();
    if (!measurement_in) {
      ATH_MSG_WARNING("MeasurementBase object could not be extracted from a "
                      "measurement... continuing");
      continue;
    }
    // Clone the MeasurementBase object
    auto measurement = measurement_in->uniqueClone();
    // Create prediction for the next measurement surface.
    // For the smoother the direction of propagation
    // is opposite to the direction of momentum
    Trk::MultiComponentState extrapolatedState =
      m_extrapolator->extrapolate(ctx,
                                  extrapolatorCache,
                                  (*loopUpdatedState),
                                  measurement->associatedSurface(),
                                  Trk::oppositeMomentum,
                                  false,
                                  particleHypothesis);

    if (extrapolatedState.empty()) {
      return MultiTrajectory();
    }
    // Handle the case where Original measurement was flagged as an outlier
    if (!(*trackStateOnSurface)->type(TrackStateOnSurface::Measurement)) {
      std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> type(
        0);
      type.set(TrackStateOnSurface::Outlier);
      updatedStateOnSurface =
        new Trk::MultiComponentStateOnSurface(FitQualityOnSurface(1, 1),
                                              std::move(measurement),
                                              std::move(extrapolatedState),
                                              nullptr,
                                              type);
      loopUpdatedState = &(updatedStateOnSurface->components());
      smoothedTrajectory.push_back(updatedStateOnSurface);
      continue;
    }
    // Update with the measurement
    updatedState = Trk::GsfMeasurementUpdator::update(
      std::move(extrapolatedState), *measurement, fitQuality);
    if (updatedState.empty()) {
      ATH_MSG_WARNING(
        "Could not update the multi-component state... rejecting track!");
      return MultiTrajectory();
    }
    // last is special as we collapse to single track Parameters
    bool islast = (trackStateOnSurface == lasttrackStateOnSurface);
    // Optional combine smoother state with fitter state
    if (m_combineWithFitter) {
      // combine the current tsos (from the forward) with
      // the updated from the smoother
      const Trk::MultiComponentState& forwardsMultiState =
        (*trackStateOnSurface)->components();
      Trk::MultiComponentState combinedfitterState =
        Trk::MultiComponentStateCombiner::combineWithSmoother(
          forwardsMultiState, updatedState, m_maximumNumberOfComponents);
      if (combinedfitterState.empty()) {
        ATH_MSG_WARNING("Could not combine state from forward fit with "
                        "smoother state");
        return MultiTrajectory();
      }
      auto combinedFitQuality = Trk::GsfMeasurementUpdator::fitQuality(
        combinedfitterState, *measurement);

      updatedStateOnSurface = smootherHelper(std::move(combinedfitterState),
                                             std::move(measurement),
                                             combinedFitQuality,
                                             islast,
                                             m_useMode);
    } else { // If combination with forwards state is not done
      updatedStateOnSurface = smootherHelper(
        std::move(updatedState), std::move(measurement), fitQuality, islast, m_useMode);
    }
    // For the next iteration start from last added
    loopUpdatedState = &(updatedStateOnSurface->components());
    smoothedTrajectory.push_back(updatedStateOnSurface);
    // Handle adding measurement from calo if it is present
    if (ccot && trackStateOnSurface == secondLastTrackStateOnSurface) {
      Trk::MultiComponentState ccotState =
        addCCOT(ctx, updatedStateOnSurface, ccot, smoothedTrajectory);
      if (!ccotState.empty()) {
        (*loopUpdatedState) = std::move(ccotState);
      }
    }
  } // End for loop over all components

  return smoothedTrajectory;
}

/*
 * Account for additional measurement from
 * the calorimeter
 */
Trk::MultiComponentState
Trk::GaussianSumFitter::addCCOT(
  const EventContext& ctx,
  const Trk::MultiComponentStateOnSurface* currentState,
  const Trk::CaloCluster_OnTrack* ccot,
  MultiTrajectory& smoothedTrajectory) const
{

  const Trk::MultiComponentStateOnSurface* currentMultiStateOS = currentState;
  if (!currentMultiStateOS || !ccot) {
    return {};
  }
  const auto& currentMultiComponentState = currentMultiStateOS->components();
  const Trk::MeasurementBase* measurement = currentState->measurementOnTrack();
  const Trk::Surface* currentSurface(nullptr);
  if (measurement) {
    currentSurface = &(measurement->associatedSurface());
  }
  Trk::MultiComponentState extrapolatedState{};
  // Extrapolate to the Calo to get prediction
  if (currentSurface) {
    extrapolatedState =
      m_extrapolator->extrapolateDirectly(ctx,
                                          currentMultiComponentState,
                                          ccot->associatedSurface(),
                                          Trk::alongMomentum,
                                          false,
                                          Trk::nonInteracting);
  }
  if (extrapolatedState.empty()) {
    return {};
  }
  // Update newly extrapolated state with measurement
  Trk::FitQualityOnSurface fitQuality;
  Trk::MultiComponentState updatedState = Trk::GsfMeasurementUpdator::update(
    std::move(extrapolatedState), *ccot, fitQuality);
  if (updatedState.empty()) {
    return {};
  }

  // Extrapolate back to the surface nearest the origin
  extrapolatedState = m_extrapolator->extrapolateDirectly(ctx,
                                                          updatedState,
                                                          *currentSurface,
                                                          Trk::oppositeMomentum,
                                                          false,
                                                          Trk::nonInteracting);
  if (extrapolatedState.empty()) {
    return {};
  }

  // Build TSOS with CCOT at the surface of calo
  // updated state not used after this point
  auto updatedMCSOS = std::make_unique<Trk::MultiComponentStateOnSurface>(
    fitQuality,
    std::unique_ptr<Trk::CaloCluster_OnTrack>(ccot->clone()),
    std::move(updatedState));

  // Now build a dummy measurement ....  we dont want to a double count the
  // measurement but we need to extrapolate back to origin to allow for the
  // perigee parameters to be estimated.
  // Note this only important if the track is
  // refit otherwise it has no influence.
  AmgSymMatrix(5) covMatrix;
  covMatrix.setZero();
  covMatrix(0, 0) = 1e6;

  Trk::DefinedParameter locX(0, Trk::locX);
  Trk::LocalParameters locpars(locX);

  auto pseudoMeasurement = std::make_unique<Trk::PseudoMeasurementOnTrack>(
    locpars, covMatrix, *currentSurface);

  //  Combine the state  and find the mode of the distribution
  std::unique_ptr<Trk::TrackParameters> combinedState =
    MultiComponentStateCombiner::combineToSingle(extrapolatedState, m_useMode);
  auto combinedFitQuality =
    Trk::GsfMeasurementUpdator::fitQuality(extrapolatedState, *ccot);

  // Build a TSOS using the dummy measurement and combined state
  auto finalSOS = std::make_unique<Trk::MultiComponentStateOnSurface>(
    combinedFitQuality,
    std::move(pseudoMeasurement),
    std::move(combinedState),
    MultiComponentStateHelpers::clone(extrapolatedState));

  // Pointers owned by the smoothedTrajectory
  smoothedTrajectory.push_back(updatedMCSOS.release());
  smoothedTrajectory.push_back(finalSOS.release());

  return extrapolatedState;
}
