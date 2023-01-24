/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingTool.h"

// Athena
#include "TrkTrackSummary/TrackSummary.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/EventData/Measurement.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/SourceLinkAccessorConcept.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ActsATLASConverterTool.h"
#include "ActsGeometry/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"

// Other
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <functional>
#include <vector>

namespace
{
  /// =========================================================================
  /// Include all sorts of stuff needed to interface with the Acts Core classes.
  /// This is only required by code in this file, so we keep it in the anonymous namespace.
  /// The actual TrackFindingTool class definition comes later.
  /// =========================================================================

  /// Borrowed from Athena Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  /// We could also access them directly from there, but that would pull inline a lot of other stuff we
  /// don't need.

  static Acts::Result<void>
  gainMatrixUpdate(const Acts::GeometryContext &gctx,
                   typename Acts::MultiTrajectory<ActsTrk::TrackFindingTool::traj_Type>::TrackStateProxy trackState,
                   Acts::NavigationDirection direction,
                   Acts::LoggerWrapper logger)
  {
    Acts::GainMatrixUpdater updater;
    return updater.template operator()<ActsTrk::TrackFindingTool::traj_Type>(gctx, trackState, direction, logger);
  }

  static Acts::Result<void>
  gainMatrixSmoother(const Acts::GeometryContext &gctx,
                     Acts::MultiTrajectory<ActsTrk::TrackFindingTool::traj_Type> &trajectory,
                     size_t entryIndex,
                     Acts::LoggerWrapper logger)
  {
    Acts::GainMatrixSmoother smoother;
    return smoother.template operator()<ActsTrk::TrackFindingTool::traj_Type>(gctx, trajectory, entryIndex, logger);
  }

  /// Borrowed from Acts Examples/Framework/include/ActsExamples/EventData/GeometryContainers.hpp

  // extract the geometry identifier from a variety of types
  struct GeometryIdGetter
  {
    // explicit geometry identifier are just forwarded
    constexpr Acts::GeometryIdentifier operator()(
        Acts::GeometryIdentifier geometryId) const
    {
      return geometryId;
    }
    // encoded geometry ids are converted back to geometry identifiers.
    constexpr Acts::GeometryIdentifier operator()(
        Acts::GeometryIdentifier::Value encoded) const
    {
      return Acts::GeometryIdentifier(encoded);
    }
    // support elements in map-like structures.
    template <typename T>
    constexpr Acts::GeometryIdentifier operator()(
        const std::pair<Acts::GeometryIdentifier, T> &mapItem) const
    {
      return mapItem.first;
    }
    // support elements that implement `.geometryId()`.
    template <typename T>
    inline auto operator()(const T &thing) const
        -> decltype(thing.geometryId(), Acts::GeometryIdentifier())
    {
      return thing.geometryId();
    }
    // support reference_wrappers around such types as well
    template <typename T>
    inline auto operator()(std::reference_wrapper<T> thing) const
        -> decltype(thing.get().geometryId(), Acts::GeometryIdentifier())
    {
      return thing.get().geometryId();
    }
  };

  struct CompareGeometryId
  {
    // indicate that comparisons between keys and full objects are allowed.
    using is_transparent = void;
    // compare two elements using the automatic key extraction.
    template <typename Left, typename Right>
    constexpr bool operator()(Left &&lhs, Right &&rhs) const
    {
      return GeometryIdGetter()(lhs) < GeometryIdGetter()(rhs);
    }
  };

  /// Store elements that know their detector geometry id, e.g. simulation hits.
  ///
  /// @tparam T type to be stored, must be compatible with `CompareGeometryId`
  ///
  /// The container stores an arbitrary number of elements for any geometry
  /// id. Elements can be retrieved via the geometry id; elements can be selected
  /// for a specific geometry id or for a larger range, e.g. a volume or a layer
  /// within the geometry hierachy using the helper functions below. Elements can
  /// also be accessed by index that uniquely identifies each element regardless
  /// of geometry id.
  template <typename T>
  using GeometryIdMultiset =
      boost::container::flat_multiset<T, CompareGeometryId>;

  /// The accessor for the GeometryIdMultiset container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  template <typename T>
  struct GeometryIdMultisetAccessor
  {
    using Container = GeometryIdMultiset<T>;
    using Key = Acts::GeometryIdentifier;
    using Value = typename GeometryIdMultiset<T>::value_type;
    using Iterator = typename GeometryIdMultiset<T>::const_iterator;

    // pointer to the container
    const Container *container = nullptr;

    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
      assert(container != nullptr);
      return container->equal_range(surface.geometryId());
    }
  };

  /// Adapted from Acts Examples/Framework/include/ActsExamples/EventData/IndexSourceLink.hpp

  /// Container of index source links.
  ///
  /// Since the source links provide a `.geometryId()` accessor, they can be
  /// stored in an ordered geometry container.
  using UncalibSourceLinkMultiset =
      GeometryIdMultiset<std::reference_wrapper<const ATLASUncalibSourceLink>>;
  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  using UncalibSourceLinkAccessor =
      GeometryIdMultisetAccessor<std::reference_wrapper<const ATLASUncalibSourceLink>>;

  /// Adapted from Acts Examples/Algorithms/TrackFinding/src/TrackFindingAlgorithmFunction.cpp

  using Stepper = Acts::EigenStepper<>;
  using Navigator = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsTrk::TrackFindingTool::traj_Type>;

} // anonymous namespace

/// =========================================================================
namespace ActsTrk
{
  struct TrackFindingTool::CKF_pimpl : public CKF
  {
    using CKF::CKF;
  };

  TrackFindingTool::TrackFindingTool(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent)
      : base_class(type, name, parent) {}

  TrackFindingTool::~TrackFindingTool() {}

  StatusCode TrackFindingTool::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_etaBins);
    ATH_MSG_DEBUG("   " << m_chi2CutOff);
    ATH_MSG_DEBUG("   " << m_numMeasurementsCutOff);

    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_trkSummaryTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_boundaryCheckTool.retrieve());
    if (!m_RotCreatorTool.empty())
    {
      ATH_MSG_INFO("RotCreatorTool will be used");
      ATH_CHECK(m_RotCreatorTool.retrieve());
    }

    auto magneticField = std::make_unique<ATLASMagneticFieldWrapper>();
    auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();

    Stepper stepper(std::move(magneticField));
    Navigator::Config cfg{trackingGeometry};
    cfg.resolvePassive = false;
    cfg.resolveMaterial = true;
    cfg.resolveSensitive = true;
    Navigator navigator(cfg);
    Propagator propagator(std::move(stepper), std::move(navigator));
    m_trackFinder.reset(new CKF_pimpl(std::move(propagator)));

    m_pOptions.maxSteps = m_maxPropagationStep;

    Acts::MeasurementSelector::Config measurementSelectorCfg{{Acts::GeometryIdentifier(),
                                                              {m_etaBins, m_chi2CutOff, m_numMeasurementsCutOff}}};
    m_measurementSelector.reset(new Acts::MeasurementSelector{measurementSelectorCfg});

    m_ckfExtensions.updater.connect<&gainMatrixUpdate>();
    m_ckfExtensions.smoother.connect<&gainMatrixSmoother>();
    m_ckfExtensions.calibrator.connect<&ATLASSourceLinkCalibrator::calibrate<traj_Type, ATLASUncalibSourceLink>>();
    m_ckfExtensions.measurementSelector.connect<&Acts::MeasurementSelector::select<traj_Type>>(m_measurementSelector.get());

    m_logger = makeActsAthenaLogger(this, "Acts");

    return StatusCode::SUCCESS;
  }

  // finalize
  StatusCode TrackFindingTool::finalize()
  {
    ATH_MSG_INFO(name() << " statistics:");
    ATH_MSG_INFO("- total seeds: " << m_nTotalSeeds);
    ATH_MSG_INFO("- failed seeds: " << m_nFailedSeeds);
    ATH_MSG_INFO("- failure ratio: " << static_cast<double>(m_nFailedSeeds) / m_nTotalSeeds);
    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingTool::findTracks(const EventContext &ctx,
                               const std::vector<ATLASUncalibSourceLink> &uncalibSourceLinks,
                               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
                               ::TrackCollection &tracksContainer) const
  {
    /// ACTS container is just a std::vector of reconstructed track states for multiple tracks.
    /// Athena uses a DataVector.
    using TrackParametersContainer = std::vector<Acts::BoundTrackParameters>;

    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    TrackParametersContainer initialParameters;
    initialParameters.reserve(estimatedTrackParameters.size());
    for (auto *trkParms : estimatedTrackParameters)
    {
      initialParameters.push_back(*trkParms);
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    // CalibrationContext converter not implemented yet.
    Acts::CalibrationContext calContext = Acts::CalibrationContext();

    UncalibSourceLinkMultiset sourceLinks;
    sourceLinks.reserve(uncalibSourceLinks.size());
    for (auto &sourceLink : uncalibSourceLinks)
    {
      sourceLinks.insert(sourceLinks.end(), sourceLink);
    }

    UncalibSourceLinkAccessor slAccessor;
    slAccessor.container = &sourceLinks;
    Acts::SourceLinkAccessorDelegate<UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&UncalibSourceLinkAccessor::range>(&slAccessor);

    // Set the CombinatorialKalmanFilter options
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<UncalibSourceLinkAccessor::Iterator, traj_Type>;
    TrackFinderOptions options(tgContext,
                               mfContext,
                               calContext,
                               slAccessorDelegate,
                               m_ckfExtensions,
                               Acts::LoggerWrapper{logger()},
                               m_pOptions,
                               &(*pSurface));

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << initialParameters.size() << " seeds.");

    auto results = m_trackFinder->findTracks(initialParameters, options);

    m_nTotalSeeds += initialParameters.size();

    // Loop over the track finding results for all initial parameters
    for (std::size_t iseed = 0; iseed < initialParameters.size(); ++iseed)
    {
      // The result for this seed
      auto &result = results[iseed];
      if (result.ok())
      {
        // Get the track finding output and add to tracksContainer
        size_t ntracks = makeTracks(ctx, tgContext, result.value(), tracksContainer);
        if (ntracks == 0)
        {
          ATH_MSG_WARNING("Track finding found no track candidates for seed " << iseed);
          m_nFailedSeeds++;
        }
      }
      else
      {
        ATH_MSG_WARNING("Track finding failed for seed " << iseed << " with error" << result.error());
        m_nFailedSeeds++;
      }
    }

    ATH_MSG_DEBUG("Completed track finding with " << tracksContainer.size() << " track candidates.");

    return StatusCode::SUCCESS;
  }

  /// based on Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  size_t
  TrackFindingTool::makeTracks(const EventContext &ctx,
                               Acts::GeometryContext &tgContext,
                               const Acts::CombinatorialKalmanFilterResult<traj_Type> &trackFindingOutput,
                               ::TrackCollection &tracksContainer) const
  {
    size_t ntracks = 0;
    for (const auto lastMeasurementIndex : trackFindingOutput.lastMeasurementIndices)
    {
      if (trackFindingOutput.fittedParameters.count(lastMeasurementIndex) <= 0)
      {
        continue;
      }

      auto finalTrajectory = DataVector<const Trk::TrackStateOnSurface>();
      // initialise the number of dead Pixel and Acts strip
      int numberOfDeadPixel = 0;
      int numberOfDeadSCT = 0;

      std::vector<std::unique_ptr<const Acts::BoundTrackParameters>> actsSmoothedParam;
      // Loop over all the output state to create track state
      trackFindingOutput.fittedStates->visitBackwards(
          lastMeasurementIndex,
          [&](const auto &state)
          {
            // First only consider states with an associated detector element
            if (!state.referenceSurface().associatedDetectorElement())
            {
              return;
            }
            // We need to determine the type of state
            auto flag = state.typeFlags();
            std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
            std::unique_ptr<const Trk::TrackParameters> parm;

            // State is a hole (no associated measurement), use predicted parameters
            if (flag[Acts::TrackStateFlag::HoleFlag])
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.predicted(),
                                                         state.predictedCovariance());
              parm = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsParam, tgContext);
              auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);

              // Check if this is a hole, a dead sensors or a state outside the sensor boundary
              if (boundaryCheck == Trk::BoundaryCheckResult::DeadElement)
              {
                auto *detElem = ActsToDetElem(state.referenceSurface());
                if (!detElem)
                {
                  return;
                }
                if (detElem->isPixel())
                {
                  ++numberOfDeadPixel;
                }
                else if (detElem->isSCT())
                {
                  ++numberOfDeadSCT;
                }
                // Dead sensors states are not stored
                return;
              }
              else if (boundaryCheck != Trk::BoundaryCheckResult::Candidate)
              {
                // States outside the sensor boundary are ignored
                return;
              }
              typePattern.set(Trk::TrackStateOnSurface::Hole);
            }
            // The state was tagged as an outlier or (TODO!) was missed in the reverse filtering, use filtered parameters
            else if (flag[Acts::TrackStateFlag::OutlierFlag]
                     //  || (trackFindingOutput.reversed &&     // TODO!
                     //      std::find(trackFindingOutput.passedAgainSurfaces.begin(),
                     //                trackFindingOutput.passedAgainSurfaces.end(),
                     //                state.referenceSurface().getSharedPtr().get()) == trackFindingOutput.passedAgainSurfaces.end())
            )
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.filtered(),
                                                         state.filteredCovariance());
              parm = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Outlier);
            }
            // The state is a measurement state, use smoothed parameters
            else
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.smoothed(),
                                                         state.smoothedCovariance());

              // is it really necessary to keep our own copy of all the smoothed parameters?
              actsSmoothedParam.push_back(std::make_unique<const Acts::BoundTrackParameters>(Acts::BoundTrackParameters(actsParam)));
              parm = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Measurement);
            }
            std::unique_ptr<const Trk::MeasurementBase> measState;
            if (state.hasUncalibrated())
            {
              const auto &sl = static_cast<const ATLASUncalibSourceLink &>(state.uncalibrated());
              const xAOD::UncalibratedMeasurement &uncalibMeas = sl.atlasHit();
              measState = makeRIO_OnTrack(uncalibMeas, parm.get());
            }
            double nDoF = state.calibratedSize();
            auto quality = Trk::FitQualityOnSurface(state.chi2(), nDoF);
            auto perState = new Trk::TrackStateOnSurface(quality,
                                                         std::move(measState),
                                                         std::move(parm),
                                                         nullptr,
                                                         typePattern);
            // If a state was succesfully created add it to the trajectory
            finalTrajectory.insert(finalTrajectory.begin(), perState);
          });

      // Convert the perigee state and add it to the trajectory - TODO: check this is the right TS
      auto actsPerIt = trackFindingOutput.fittedParameters.find(lastMeasurementIndex);
      if (actsPerIt != trackFindingOutput.fittedParameters.end())
      {
        auto per = m_ATLASConverterTool->ActsTrackParameterToATLAS(actsPerIt->second, tgContext);
        std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
        typePattern.set(Trk::TrackStateOnSurface::Perigee);
        auto perState = new Trk::TrackStateOnSurface(nullptr,
                                                     std::move(per),
                                                     nullptr,
                                                     typePattern);
        finalTrajectory.insert(finalTrajectory.begin(), perState);
      }

      // Create the track using the states
      Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::KalmanFitter, Trk::noHypothesis);
      newInfo.setTrackFitter(Trk::TrackInfo::TrackFitter::KalmanFitter); // Mark the fitter as KalmanFitter
      auto newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
      if (!newtrack->trackSummary())
      {
        newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
        newtrack->trackSummary()->update(Trk::numberOfPixelHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfSCTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfTRTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors, numberOfDeadPixel);
        newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors, numberOfDeadSCT);
      }
      m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, true);
      tracksContainer.push_back(std::move(newtrack));
      ++ntracks;
    }
    return ntracks;
  }

  const InDetDD::SiDetectorElement *
  TrackFindingTool::ActsToDetElem(const Acts::Surface &surface)
  {
    const auto *actsElement = dynamic_cast<const ActsDetectorElement *>(surface.associatedDetectorElement());
    if (!actsElement)
    {
      return nullptr;
    }
    return dynamic_cast<const InDetDD::SiDetectorElement *>(actsElement->upstreamDetectorElement());
  }

  std::unique_ptr<const Trk::MeasurementBase>
  TrackFindingTool::makeRIO_OnTrack(const xAOD::UncalibratedMeasurement &uncalibMeas,
                                    const Trk::TrackParameters *parm) const
  {
    const Trk::PrepRawData *rio = nullptr;
    if (auto pixcl = dynamic_cast<const xAOD::PixelCluster *>(&uncalibMeas))
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::PixelClusterCollection>> pixelLinkAcc("pixelClusterLink");
      if (!pixelLinkAcc.isAvailable(*pixcl))
      {
        ATH_MSG_WARNING("no pixelClusterLink for cluster associated to measurement");
        return nullptr;
      }
      auto pix = *pixelLinkAcc(*pixcl);
      if (m_RotCreatorTool.empty())
      {
        const InDetDD::SiDetectorElement *element = pix->detectorElement();
        if (!element)
        {
          ATH_MSG_WARNING("Cannot access pixel detector element");
          return nullptr;
        }
        ATH_MSG_DEBUG("create InDet::PixelClusterOnTrack without correction");
        return std::make_unique<const InDet::PixelClusterOnTrack>(pix,
                                                                  Trk::LocalParameters(pix->localPosition()),
                                                                  pix->localCovariance(),
                                                                  element->identifyHash(),
                                                                  pix->globalPosition(),
                                                                  pix->gangedPixel(),
                                                                  false);
      }
      rio = pix;
    }
    else if (auto stripcl = dynamic_cast<const xAOD::StripCluster *>(&uncalibMeas))
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::SCT_ClusterCollection>> stripLinkAcc("sctClusterLink");
      if (!stripLinkAcc.isAvailable(*stripcl))
      {
        ATH_MSG_WARNING("no sctClusterLink for clusters associated to measurement");
        return nullptr;
      }
      auto sct = *stripLinkAcc(*stripcl);
      if (m_RotCreatorTool.empty())
      {
        const InDetDD::SiDetectorElement *element = sct->detectorElement();
        if (!element)
        {
          ATH_MSG_WARNING("Cannot access strip detector element");
          return nullptr;
        }
        ATH_MSG_DEBUG("create InDet::SCT_ClusterOnTrack without correction");
        return std::make_unique<const InDet::SCT_ClusterOnTrack>(sct,
                                                                 Trk::LocalParameters(sct->localPosition()),
                                                                 sct->localCovariance(),
                                                                 element->identifyHash(),
                                                                 sct->globalPosition(),
                                                                 false);
      }
      rio = sct;
    }
    else
    {
      ATH_MSG_WARNING("xAOD::UncalibratedMeasurement is neither xAOD::PixelCluster nor xAOD::StripCluster");
      return nullptr;
    }

    ATH_MSG_DEBUG("use Trk::RIO_OnTrackCreator::correct to create corrected Trk::RIO_OnTrack");
    assert(!m_RotCreatorTool.empty());
    assert(rio != nullptr);
    return std::unique_ptr<const Trk::MeasurementBase>(m_RotCreatorTool->correct(*rio, *parm));
  }

} // namespace ActsTrk
