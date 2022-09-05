// /*
//   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
// */

// ATHENA   
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"

// ACTS
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"

// PACKAGE  
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ActsDetectorElement.h"

template<typename trajectory_t>
std::unique_ptr<Trk::Track>
ActsKalmanFitter::makeTrack(const EventContext& ctx, Acts::GeometryContext& tgContext, TrackFitterResult<trajectory_t>& fitResult) const {
  std::unique_ptr<Trk::Track> newtrack = nullptr;
  // Get the fit output object
  const auto& fitOutput = fitResult.value();
  if (fitOutput.fittedParameters) {
    auto finalTrajectory = DataVector<const Trk::TrackStateOnSurface>();
    // initialise the number of dead Pixel and Acts strip
    int numberOfDeadPixel = 0;
    int numberOfDeadSCT = 0;

    std::vector<std::unique_ptr<const Acts::BoundTrackParameters>> actsSmoothedParam;
    // Loop over all the output state to create track state
    fitOutput.fittedStates.visitBackwards(fitOutput.lastMeasurementIndex, [&](const auto &state) {
      // First only concider state with an associated detector element not in the TRT
      auto flag = state.typeFlags();
      if (state.referenceSurface().associatedDetectorElement() != nullptr) {
        const auto* actsElement = dynamic_cast<const ActsDetectorElement*>(state.referenceSurface().associatedDetectorElement());
        ATH_MSG_VERBOSE("Try casting to TRT for if");
        if (actsElement != nullptr 
            && dynamic_cast<const InDetDD::TRT_BaseElement*>(actsElement->upstreamDetectorElement()) == nullptr) {
          const auto* trkDetElem = dynamic_cast<const Trk::TrkDetElementBase*>(actsElement->upstreamDetectorElement());
          ATH_MSG_VERBOSE("trkDetElem type: " << static_cast<std::underlying_type_t<Trk::DetectorElemType>>(trkDetElem->detectorType()));
          ATH_MSG_VERBOSE("Try casting to SiDetectorElement");
          const auto* detElem = dynamic_cast<const InDetDD::SiDetectorElement*>(actsElement->upstreamDetectorElement());
          ATH_MSG_VERBOSE("detElem = " << detElem);
          // We need to determine the type of state 
          std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
          std::unique_ptr<const Trk::TrackParameters> parm;

          // State is a hole (no associated measurement), use predicted parameters      
          if (flag[Acts::TrackStateFlag::HoleFlag] == true){         
            const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                       state.predicted(),
                                                       state.predictedCovariance());
            parm = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsParam, tgContext);
            auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);
            
            // Check if this is a hole, a dead sensors or a state outside the sensor boundary
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
          else if (flag[Acts::TrackStateFlag::OutlierFlag] == true || ( fitOutput.reversed && std::find(fitOutput.passedAgainSurfaces.begin(), fitOutput.passedAgainSurfaces.end(), state.referenceSurface().getSharedPtr().get()) == fitOutput.passedAgainSurfaces.end())){
            const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                       state.filtered(),
                                                       state.filteredCovariance());
            parm = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsParam, tgContext);
            typePattern.set(Trk::TrackStateOnSurface::Outlier);
          }
          // The state is a measurement state, use smoothed parameters 
          else{
            const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                       state.smoothed(),
                                                       state.smoothedCovariance());

            actsSmoothedParam.push_back(std::make_unique<const Acts::BoundTrackParameters>(Acts::BoundTrackParameters(actsParam)));
            parm = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsParam, tgContext);
            typePattern.set(Trk::TrackStateOnSurface::Measurement);                                           
          }
          std::unique_ptr<const Trk::MeasurementBase> measState;
          if (state.hasUncalibrated()){
            const auto& sl = static_cast<const ATLASSourceLink&>(state.uncalibrated());
            measState = sl.atlasHit().uniqueClone();
          }
          double nDoF = state.calibratedSize();
          auto quality =std::make_unique<const Trk::FitQualityOnSurface>(state.chi2(), nDoF);
          const Trk::TrackStateOnSurface *perState = new Trk::TrackStateOnSurface(std::move(measState), std::move(parm), std::move(quality), nullptr, typePattern);
          // If a state was succesfully created add it to the trajectory 
          if (perState) {
            finalTrajectory.insert(finalTrajectory.begin(), perState);
          }
        }
      }
    return;
    });

    // Convert the perigee state and add it to the trajectory
    const Acts::BoundTrackParameters actsPer = fitOutput.fittedParameters.value();
    std::unique_ptr<const Trk::TrackParameters> per = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsPer, tgContext);
    std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
    typePattern.set(Trk::TrackStateOnSurface::Perigee);
    const Trk::TrackStateOnSurface *perState = new Trk::TrackStateOnSurface(nullptr, std::move(per), nullptr, nullptr, typePattern);
    if (perState) finalTrajectory.insert(finalTrajectory.begin(), perState);

    // Create the track using the states
    Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::KalmanFitter, Trk::noHypothesis);
    newInfo.setTrackFitter(Trk::TrackInfo::TrackFitter::KalmanFitter); //Mark the fitter as KalmanFitter
    newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
    if (newtrack) {
      // Create the track summary and update the holes information
      auto holeSurfaces = fitOutput.missedActiveSurfaces;
      if (!newtrack->trackSummary()) {
        newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
        newtrack->trackSummary()->update(Trk::numberOfPixelHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfSCTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfTRTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors, numberOfDeadPixel);
        newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors, numberOfDeadSCT);
      }
      m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, nullptr, true);
    }
  }
  return newtrack;
}

template<typename trajectory_t>
Acts::Result<void> ActsKalmanFitter::gainMatrixUpdate(const Acts::GeometryContext& gctx,
    typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState, Acts::NavigationDirection direction, Acts::LoggerWrapper logger) {
  Acts::GainMatrixUpdater updater;
  return updater.template operator()<trajectory_t>(gctx, trackState, direction, logger);
}

template<typename trajectory_t>
Acts::Result<void> ActsKalmanFitter::gainMatrixSmoother(const Acts::GeometryContext& gctx,
    Acts::MultiTrajectory<trajectory_t>& trajectory, size_t entryIndex, Acts::LoggerWrapper logger) {
  Acts::GainMatrixSmoother smoother;
  return smoother.template operator()<trajectory_t>(gctx, trajectory, entryIndex, logger);
}

