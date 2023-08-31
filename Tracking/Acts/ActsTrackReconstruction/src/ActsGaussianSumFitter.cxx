/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGaussianSumFitter.h"

// ATHENA
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TrkTrack/TrackStateOnSurface.h"

// ACTS
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "ActsEvent/TrackContainer.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsInterop/Logger.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsGeometryContext.h"

// STL
#include <vector>
#include <bitset>
#include <type_traits>
#include <system_error>

namespace ActsTrk {

ActsGaussianSumFitter::ActsGaussianSumFitter(const std::string& t,
					     const std::string& n,
					     const IInterface* p) :
  base_class(t,n,p)
{}

StatusCode ActsGaussianSumFitter::initialize() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_trkSummaryTool.retrieve());

  m_logger = makeActsAthenaLogger(this, "Acts Gaussian Sum Refit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();
  Acts::MultiEigenStepperLoop<> stepper(field);
  Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometryTool->trackingGeometry() } );     
  Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator> propagator(std::move(stepper), 
                     std::move(navigator),
                     logger().cloneWithSuffix("Prop"));
  Acts::Experimental::AtlasBetheHeitlerApprox<6, 5> bha = Acts::Experimental::makeDefaultBetheHeitlerApprox();

  m_fitter = std::make_unique<Fitter>(std::move(propagator), std::move(bha),
              logger().cloneWithSuffix("GaussianSumFitter"));

  m_gsfExtensions.updater.connect<&ActsTrk::FitterHelperFunctions::gainMatrixUpdate<ActsTrk::TrackStateBackend>>();
  m_gsfExtensions.calibrator.connect<&ATLASSourceLinkCalibrator::calibrate<ActsTrk::TrackStateBackend>>();
  
  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;
  m_gsfExtensions.outlierFinder.connect<&ActsTrk::FitterHelperFunctions::ATLASOutlierFinder::operator()<ActsTrk::TrackStateBackend>>(&m_outlierFinder);

  return StatusCode::SUCCESS;
}

// refit a track
// -------------------------------------------------------
std::unique_ptr<Trk::Track>
ActsGaussianSumFitter::fit(const EventContext& ctx,
                       const Trk::Track& inputTrack,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*prtHypothesis*/) const
{
  std::unique_ptr<Trk::Track> track = nullptr;
  ATH_MSG_VERBOSE ("--> enter GaussianSumFitter::fit(Track,,)    with Track from author = "
       << inputTrack.info().dumpInfo());

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || inputTrack.measurementsOnTrack()->size() < 2) {
    ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the GSF, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext{};

  // Set the GaussianSumFitter options
  Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext, 
				*pSurface);

  std::vector<typename ATLASSourceLink::ElementsType> elementCollection;

  std::vector<ATLASSourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext,inputTrack,elementCollection);
  const auto& initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters((*inputTrack.perigeeParameters()), tgContext);

  return performFit(ctx, 
		    tgContext,
		    gsfOptions,
		    trackSourceLinks, 
		    initialParams);
}

// fit a set of MeasurementBase objects
// --------------------------------
std::unique_ptr<Trk::Track>
ActsGaussianSumFitter::fit(const EventContext& ctx,
                       const Trk::MeasurementSet& inputMeasSet,
                       const Trk::TrackParameters& estimatedStartParameters,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{
  std::unique_ptr<Trk::Track> track = nullptr;

  // protection against not having measurements on the input track
  if (inputMeasSet.size() < 2) {
    ATH_MSG_DEBUG("called to refit empty measurement set or a measurement set with too little information, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext{};

  // Set the GaussianSumFitter options
  Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext, 
				*pSurface);

  // Set abortOnError to false, else the refitting crashes if no forward propagation is done. Here, we just skip the event and continue.
  gsfOptions.abortOnError = false;

  std::vector< ATLASSourceLink > trackSourceLinks;
  trackSourceLinks.reserve(inputMeasSet.size());

  std::vector< ATLASSourceLink::ElementsType > elementCollection;
  elementCollection.reserve(inputMeasSet.size());

  for (auto* measSet : inputMeasSet) {
    trackSourceLinks.push_back(m_ATLASConverterTool->trkMeasurementToSourceLink(tgContext, *measSet, elementCollection));
  }
  const auto& initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(estimatedStartParameters, tgContext); 

  return performFit(ctx,
		    tgContext,
		    gsfOptions,
                    trackSourceLinks,
                    initialParams);
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
ActsGaussianSumFitter::fit(const EventContext& /*ctx*/,
                       const Trk::PrepRawDataSet& /*inputPRDColl*/,
                       const Trk::TrackParameters& /*estimatedStartParameters*/,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*prtHypothesis*/) const
{
  ATH_MSG_DEBUG("Fit of PrepRawDataSet not yet implemented");
  return nullptr;
}

// extend a track fit to include an additional set of MeasurementBase objects
// re-implements the TrkFitterUtils/TrackFitter.cxx general code in a more
// mem efficient and stable way
// --------------------------------
std::unique_ptr<Trk::Track>
ActsGaussianSumFitter::fit(const EventContext& ctx,
                       const Trk::Track& inputTrack,
                       const Trk::MeasurementSet& addMeasColl,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{
  ATH_MSG_VERBOSE ("--> enter GaussianSumFitter::fit(Track,Meas'BaseSet,,)");
  ATH_MSG_VERBOSE ("    with Track from author = " << inputTrack.info().dumpInfo());

  // protection, if empty MeasurementSet
  if (addMeasColl.empty()) {
    ATH_MSG_DEBUG( "client tries to add an empty MeasurementSet to the track fit." );
    return fit(ctx,inputTrack);
  }

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || (inputTrack.measurementsOnTrack()->size() < 2 && addMeasColl.empty())) {
    ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the GSF, reject fit");
    return nullptr;
  }

  std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext{};

  // Set the GaussianSumFitter options
  Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext,
				*pSurface);

  std::vector< ATLASSourceLink::ElementsType > elementCollection;

  std::vector<ATLASSourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, inputTrack, elementCollection);
  const auto& initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(inputTrack.perigeeParameters()), tgContext);


  std::vector< typename ATLASSourceLink::ElementsType > atlasElementCollection;
  atlasElementCollection.reserve(addMeasColl.size());

  for (auto* meas : addMeasColl)  {
    trackSourceLinks.push_back(m_ATLASConverterTool->trkMeasurementToSourceLink(tgContext, *meas, atlasElementCollection));
  }

  return performFit(ctx,
		    tgContext,
		    gsfOptions,
                    trackSourceLinks,
                    initialParams);
}

// extend a track fit to include an additional set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
ActsGaussianSumFitter::fit(const EventContext& /*ctx*/,
                       const Trk::Track& /*inputTrack*/,
                       const Trk::PrepRawDataSet& /*addPrdColl*/,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{

  ATH_MSG_DEBUG("Fit of Track with additional PrepRawDataSet not yet implemented");
  return nullptr;
}

// combined fit of two tracks
// --------------------------------
std::unique_ptr<Trk::Track>
ActsGaussianSumFitter::fit(const EventContext& ctx,
                       const Trk::Track& intrk1,
                       const Trk::Track& intrk2,
                       const Trk::RunOutlierRemoval /*runOutlier*/,
                       const Trk::ParticleHypothesis /*matEffects*/) const
{ 
  ATH_MSG_VERBOSE ("--> enter GaussianSumFitter::fit(Track,Track,)");
  ATH_MSG_VERBOSE ("    with Tracks from #1 = " << intrk1.info().dumpInfo()
                   << " and #2 = " << intrk2.info().dumpInfo());

  // protection, if empty track2
  if (!intrk2.measurementsOnTrack()) {
    ATH_MSG_DEBUG( "input #2 is empty try to fit track 1 alone" );
    return fit(ctx,intrk1);
  }

  // protection, if empty track1
  if (!intrk1.measurementsOnTrack()) {
    ATH_MSG_DEBUG( "input #1 is empty try to fit track 2 alone" );
    return fit(ctx,intrk2);
  }

  // protection against not having track parameters on the input track
  if (!intrk1.trackParameters() || intrk1.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input #1 fails to provide track parameters for seeding the GSF, reject fit");
    return nullptr;
  }

   std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
   std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3{0., 0., 0.});
  
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext{};

  // Set the GaussianSumFitter options
  Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext,
				*pSurface);

  std::vector< typename ATLASSourceLink::ElementsType > elementCollection1;
  std::vector< typename ATLASSourceLink::ElementsType > elementCollection2;

  std::vector<ATLASSourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, intrk1, elementCollection1);
  std::vector<ATLASSourceLink> trackSourceLinks2 = m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, intrk2, elementCollection2);
  trackSourceLinks.insert(trackSourceLinks.end(), trackSourceLinks2.begin(), trackSourceLinks2.end());
  const auto &initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(intrk1.perigeeParameters()), tgContext);

  return performFit(ctx,
		    tgContext,
		    gsfOptions,
                    trackSourceLinks,
                    initialParams);
}

std::unique_ptr<Trk::Track> 
ActsGaussianSumFitter::makeTrack(const EventContext& ctx,
          const Acts::GeometryContext& tgContext,
          ActsTrk::TrackContainer& tracks,
          Acts::Result<typename ActsTrk::TrackContainer::TrackProxy, std::error_code>& fitResult) const {
  if (not fitResult.ok()) 
    return nullptr;    

  std::unique_ptr<Trk::Track> newtrack = nullptr;
  // Get the fit output object
  const auto& acts_track = fitResult.value();
  auto finalTrajectory = std::make_unique<Trk::TrackStates>();
  // initialise the number of dead Pixel and Acts strip
  int numberOfDeadPixel = 0;
  int numberOfDeadSCT = 0;

  std::vector<std::unique_ptr<const Acts::BoundTrackParameters>> actsSmoothedParam;
  // Loop over all the output state to create track state
  tracks.trackStateContainer().visitBackwards(acts_track.tipIndex(), 
                [&] (const auto &state) -> void
  {
    // First only concider state with an associated detector element not in the TRT
    auto flag = state.typeFlags();
    const auto* associatedDetEl = state.referenceSurface().associatedDetectorElement();
    if (not associatedDetEl) 
      return;
    
    const auto* actsElement = dynamic_cast<const ActsDetectorElement*>(associatedDetEl);
    if (not actsElement) 
      return;

    const auto* upstreamDetEl = actsElement->upstreamDetectorElement();
    if (not upstreamDetEl) 
      return;

    ATH_MSG_VERBOSE("Try casting to TRT for if");    
    if (dynamic_cast<const InDetDD::TRT_BaseElement*>(upstreamDetEl))
      return;

    const auto* trkDetElem = dynamic_cast<const Trk::TrkDetElementBase*>(upstreamDetEl);
    if (not trkDetElem)
      return;

    ATH_MSG_VERBOSE("trkDetElem type: " << static_cast<std::underlying_type_t<Trk::DetectorElemType>>(trkDetElem->detectorType()));

    ATH_MSG_VERBOSE("Try casting to SiDetectorElement");
    const auto* detElem = dynamic_cast<const InDetDD::SiDetectorElement*>(upstreamDetEl);
    if (not detElem)
      return;
    ATH_MSG_VERBOSE("detElem = " << detElem);

    // We need to determine the type of state 
    std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
    std::unique_ptr<Trk::TrackParameters> parm;

    // State is a hole (no associated measurement), use predicted parameters      
    if (flag.test(Acts::TrackStateFlag::HoleFlag)){
      ATH_MSG_VERBOSE("State is a hole (no associated measurement), use predicted parameters");
      const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
             state.predicted(),
             state.predictedCovariance());
      parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
      auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);
      
      // Check if this is a hole, a dead sensors or a state outside the sensor boundary
      ATH_MSG_VERBOSE("Check if this is a hole, a dead sensors or a state outside the sensor boundary");
      if(boundaryCheck == Trk::BoundaryCheckResult::DeadElement){
  if (detElem->isPixel()) {
    ++numberOfDeadPixel;
  }
  else if (detElem->isSCT()) {
    ++numberOfDeadSCT;
  }
  // Dead sensors states are not stored              
  return;
      } else if (boundaryCheck != Trk::BoundaryCheckResult::Candidate){
  // States outside the sensor boundary are ignored
  return;
      }
      typePattern.set(Trk::TrackStateOnSurface::Hole);
    }
    // The state was tagged as an outlier or was missed in the reverse filtering, use filtered parameters
    else if (flag.test(Acts::TrackStateFlag::OutlierFlag) or not state.hasSmoothed()) {
      ATH_MSG_VERBOSE("The state was tagged as an outlier or was missed in the reverse filtering, use filtered parameters");
      const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
             state.filtered(),
             state.filteredCovariance());
      parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
      typePattern.set(Trk::TrackStateOnSurface::Outlier);
    }
    // The state is a measurement state, use smoothed parameters 
    else{
      ATH_MSG_VERBOSE("The state is a measurement state, use smoothed parameters");

      const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
             state.smoothed(),
             state.smoothedCovariance());
      
      actsSmoothedParam.push_back(std::make_unique<const Acts::BoundTrackParameters>(Acts::BoundTrackParameters(actsParam)));
      parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
      typePattern.set(Trk::TrackStateOnSurface::Measurement);                                           
    }

    std::unique_ptr<Trk::MeasurementBase> measState;
    if (state.hasUncalibratedSourceLink()){
      auto sl = state.getUncalibratedSourceLink().template get<ATLASSourceLink>();
      measState = sl.atlasHit().uniqueClone();
    }
    double nDoF = state.calibratedSize();
    auto quality =Trk::FitQualityOnSurface(state.chi2(), nDoF);
    const Trk::TrackStateOnSurface *perState = new Trk::TrackStateOnSurface(quality, std::move(measState), std::move(parm), nullptr, typePattern);
    // If a state was succesfully created add it to the trajectory 
    if (perState) {
      ATH_MSG_VERBOSE("State succesfully creates, adding it to the trajectory");
      finalTrajectory->insert(finalTrajectory->begin(), perState);
    }
  });
  
  // Convert the perigee state and add it to the trajectory
  const Acts::BoundTrackParameters actsPer(acts_track.referenceSurface().getSharedPtr(), 
                 acts_track.parameters(), 
             acts_track.covariance());
  std::unique_ptr<Trk::TrackParameters> per = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsPer, tgContext);
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
  typePattern.set(Trk::TrackStateOnSurface::Perigee);
  const Trk::TrackStateOnSurface *perState = new Trk::TrackStateOnSurface(nullptr, std::move(per), nullptr, typePattern);
  if (perState) finalTrajectory->insert(finalTrajectory->begin(), perState);

  // Create the track using the states
  Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::GaussianSumFilter, Trk::noHypothesis);
  newInfo.setTrackFitter(Trk::TrackInfo::TrackFitter::GaussianSumFilter); //Mark the fitter as GaussianSumFilter
  newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
  if (newtrack) {
    // Create the track summary and update the holes information
    if (!newtrack->trackSummary()) {
      newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
      newtrack->trackSummary()->update(Trk::numberOfPixelHoles, 0);
      newtrack->trackSummary()->update(Trk::numberOfSCTHoles, 0);
      newtrack->trackSummary()->update(Trk::numberOfTRTHoles, 0);
      newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors, numberOfDeadPixel);
      newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors, numberOfDeadSCT);
    }
    m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, true);
  }
  
  return newtrack;
}

const Acts::Experimental::GsfExtensions<typename ActsTrk::TrackStateBackend>& 
ActsGaussianSumFitter::getExtensions() const 
{ 
  return m_gsfExtensions; 
}

/// Private access to the logger
const Acts::Logger& 
ActsGaussianSumFitter::logger() const 
{ 
  return *m_logger; 
}

Acts::Experimental::GsfOptions<typename ActsTrk::TrackStateBackend> 
ActsGaussianSumFitter::prepareOptions(const Acts::GeometryContext& tgContext,
				      const Acts::MagneticFieldContext& mfContext,
				      const Acts::CalibrationContext& calContext,
				      const Acts::PerigeeSurface& surface) const
{
  Acts::PropagatorPlainOptions propagationOption;
  propagationOption.maxSteps = m_option_maxPropagationStep;

  Acts::Experimental::GsfOptions<typename ActsTrk::TrackStateBackend> gsfOptions{tgContext, mfContext, calContext, m_gsfExtensions, std::move(propagationOption)};
  gsfOptions.referenceSurface = &surface;

  // Set abortOnError to false, else the refitting crashes if no forward propagation is done. Here, we just skip the event and continue.
  gsfOptions.abortOnError = false;

  return gsfOptions;
}

std::unique_ptr<Trk::Track> 
ActsGaussianSumFitter::performFit(const EventContext& ctx,
				  const Acts::GeometryContext& tgContext,
				  const Acts::Experimental::GsfOptions<ActsTrk::TrackStateBackend>& gsfOptions,
				  const std::vector<ATLASSourceLink>& trackSourceLinks,
				  const Acts::BoundTrackParameters& initialParams) const
{
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }

  Acts::TrackContainer tracks{
    Acts::VectorTrackContainer{},
    Acts::VectorMultiTrajectory{}};

  // Convert to Acts::SourceLink during iteration
  Acts::SourceLinkAdapterIterator begin{trackSourceLinks.begin()};
  Acts::SourceLinkAdapterIterator end{trackSourceLinks.end()};

  // Perform the fit
  auto result = m_fitter->fit(begin, end,
			      initialParams, gsfOptions, tracks);

  // Convert
  if (not result.ok()) return nullptr;
  return makeTrack(ctx, tgContext, tracks, result);
}

}
