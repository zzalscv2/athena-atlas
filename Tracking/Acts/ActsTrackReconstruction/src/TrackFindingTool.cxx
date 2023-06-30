/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingTool.h"

// Athena
#include "TrkParameters/TrackParameters.h"
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
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/SourceLinkAccessorConcept.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
// ACTS glue
#include "ActsEvent/TrackContainer.h"

// PACKAGE
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"

// Other
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <sstream>
#include <functional>
#include <tuple>
#include <algorithm>

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
		   typename ActsTrk::TrackStateBackend::TrackStateProxy trackState,
                   Acts::Direction direction,
                   const Acts::Logger &logger)
  {
    Acts::GainMatrixUpdater updater;
    return updater.template operator()<ActsTrk::TrackStateBackend>(gctx, trackState, direction, logger);
  }

  static Acts::Result<void>
  gainMatrixSmoother(const Acts::GeometryContext &gctx,
                     ActsTrk::TrackStateBackend &trajectory,
                     size_t entryIndex,
                     const Acts::Logger &logger)
  {
    Acts::GainMatrixSmoother smoother;
    return smoother.template operator()<ActsTrk::TrackStateBackend>(gctx, trajectory, entryIndex, logger);
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

  /// Container of uncalibrated source links.
  ///
  /// Since the source links provide a `.geometryId()` accessor, they can be
  /// stored in an ordered geometry container.
  using UncalibSourceLinkMultiset =
      GeometryIdMultiset<ATLASUncalibSourceLink>;

  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  struct UncalibSourceLinkAccessor
      : GeometryIdMultisetAccessor<ATLASUncalibSourceLink>
  {
    using BaseIterator = GeometryIdMultisetAccessor<ATLASUncalibSourceLink>::Iterator;
    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;

    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
      assert(container != nullptr);
      auto [begin, end] = container->equal_range(surface.geometryId());
      return {Iterator{begin}, Iterator{end}};
    }
  };

  /// Adapted from Acts Examples/Algorithms/TrackFinding/src/TrackFindingAlgorithmFunction.cpp

  using Stepper = Acts::EigenStepper<>;
  using Navigator = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsTrk::TrackStateBackend>;

  // Small holder class to keep CKF and related objects.
  // Keep a unique_ptr<CKF_pimpl> in TrackFindingTool, so we don't have to expose the
  // Acts class definitions to in TrackFindingTool.h.
  struct CKF_config
  {
    // CKF algorithm
    CKF ckf;
    // CKF configuration
    Acts::MeasurementSelector measurementSelector;
    Acts::PropagatorPlainOptions pOptions;
    Acts::CombinatorialKalmanFilterExtensions<ActsTrk::TrackStateBackend> ckfExtensions;
  };

  // get Athena SiDetectorElement from Acts surface
  static const InDetDD::SiDetectorElement *actsToDetElem(const Acts::Surface &surface)
  {
    const auto *actsElement = dynamic_cast<const ActsDetectorElement *>(surface.associatedDetectorElement());
    if (!actsElement)
    {
      return nullptr;
    }
    return dynamic_cast<const InDetDD::SiDetectorElement *>(actsElement->upstreamDetectorElement());
  }

  // Helper class to convert xAOD::PixelClusterContainer or xAOD::StripClusterContainer to UncalibSourceLinkMultiset.
  struct Measurements_impl : public ActsTrk::ITrackFindingTool::Measurements
  {

    Measurements_impl(size_t numMeasurements, const ActsTrk::IActsToTrkConverterTool *converterTool, const ActsTrk::ITrackStatePrinter *trackStatePrinter)
        : m_numMeasurements(numMeasurements), m_ATLASConverterTool(converterTool), m_trackStatePrinter(trackStatePrinter)
    {
      m_sourceLinks.reserve(m_numMeasurements);
      m_elementsCollection.reserve(m_numMeasurements);
      m_sourceLinksVec.reserve(2);
      m_measurementOffset.reserve(2);
    }

    void addMeasurements(size_t type, const EventContext &ctx, const ActsTrk::UncalibratedMeasurementContainerPtr &clusterContainer, const InDetDD::SiDetectorElementCollection *detElems) override
    {
      // the following is just in case we call addMeasurements out of order or not for 2 types of measurements
      if (!(type < m_sourceLinksVec.size()))
        m_sourceLinksVec.resize(type + 1);
      if (!(type < m_measurementOffset.size()))
        m_measurementOffset.resize(type + 1);
      m_measurementOffset[type] = m_sourceLinks.size();

      std::visit([&](auto &&clusterContainerVar)
                 {
                    m_sourceLinksVec[type].reserve(clusterContainerVar->size());
                    for (auto *measurement : *clusterContainerVar)
                    {
                      auto sl = m_ATLASConverterTool->uncalibratedTrkMeasurementToSourceLink(*detElems, *measurement, m_elementsCollection);
                      m_sourceLinks.insert(m_sourceLinks.end(), sl);
                      m_sourceLinksVec[type].push_back(sl);
                    } },
                 clusterContainer);

      if (m_trackStatePrinter)
      {
        m_trackStatePrinter->printSourceLinks(ctx, m_sourceLinksVec[type], type, m_measurementOffset[type]);
      }
    }

    const UncalibSourceLinkMultiset &sourceLinks() const { return m_sourceLinks; }
    const std::vector<ATLASUncalibSourceLink> &sourceLinkVec(size_t type) const { return m_sourceLinksVec[type]; }
    size_t measurementOffset(size_t type) const { return m_measurementOffset[type]; }

  private:
    size_t m_numMeasurements = 0;
    const ActsTrk::IActsToTrkConverterTool *m_ATLASConverterTool = nullptr;
    const ActsTrk::ITrackStatePrinter *m_trackStatePrinter = nullptr;
    UncalibSourceLinkMultiset m_sourceLinks;
    std::vector<std::vector<ATLASUncalibSourceLink>> m_sourceLinksVec;
    std::vector<ATLASUncalibSourceLink::ElementsType> m_elementsCollection;
    std::vector<size_t> m_measurementOffset;
  };

} // anonymous namespace

/// =========================================================================
namespace ActsTrk
{
  struct TrackFindingTool::CKF_pimpl : public CKF_config
  {
  };

  struct TrackFindingTool::DuplicateSeedDetector
  {
    // Identify duplicate seeds: seeds where all measurements were already located in a previously followed trajectory.
    // Currently local to findTracks(), so won't remove duplicate seeds between pixels and strips.
    DuplicateSeedDetector(const ActsTrk::SeedContainer *seeds, const std::vector<ATLASUncalibSourceLink> &measurements, bool enabled)
        : m_disabled(!enabled || !seeds),
          m_nUsedMeasurements(enabled && seeds ? seeds->size() : 0u, 0u),
          m_nSeedMeasurements(enabled && seeds ? seeds->size() : 0u, 0u),
          m_isDuplicateSeed(enabled && seeds ? seeds->size() : 0u, false)
    {
      if (m_disabled)
        return;
      m_seedIndex.reserve(6 * seeds->size()); // 6 hits/seed for strips (3 for pixels)
      size_t iseed = 0;
      for (const auto *seed : *seeds)
      {
        if (!seed)
          continue;
        for (auto *sp : seed->sp())
        {
          for (auto imeasurement : sp->measurementIndexes())
          {
            if (!(imeasurement < measurements.size()))
              continue;
            m_seedIndex.insert({&measurements[imeasurement].atlasHit(), iseed});
            ++m_nSeedMeasurements[iseed];
          }
        }
        ++iseed;
      }
    }

    void newTrajectory()
    {
      if (m_disabled || m_nextSeed == m_nUsedMeasurements.size())
        return;
      auto beg = m_nUsedMeasurements.begin();
      if (m_nextSeed < m_nUsedMeasurements.size())
        std::advance(beg, m_nextSeed);
      std::fill(beg, m_nUsedMeasurements.end(), 0u);
    }

    void addMeasurement(const ATLASUncalibSourceLink &sl)
    {
      if (m_disabled || m_nextSeed == m_nUsedMeasurements.size())
        return;
      for (auto [iiseed, eiseed] = m_seedIndex.equal_range(&sl.atlasHit()); iiseed != eiseed; ++iiseed)
      {
        size_t iseed = iiseed->second;
        assert(iseed < m_nUsedMeasurements.size());
        if (iseed < m_nextSeed || m_isDuplicateSeed[iseed])
          continue;
        if (++m_nUsedMeasurements[iseed] >= m_nSeedMeasurements[iseed])
        {
          assert(m_nUsedMeasurements[iseed] == m_nSeedMeasurements[iseed]); // shouldn't ever find more
          m_isDuplicateSeed[iseed] = true;
        }
      }
    }

    // Assumes isDuplicate(iseed) is called for monotonically increasing iseed - for as long as DuplicateSeedDetector exists.
    bool isDuplicate(size_t iseed)
    {
      if (m_disabled || !(iseed < m_isDuplicateSeed.size()))
        return false;
      // If iseed not increasing, we will miss some duplicate seeds, but won't exclude needed seeds.
      if (iseed >= m_nextSeed)
        m_nextSeed = iseed + 1;
      return m_isDuplicateSeed[iseed];
    }

  private:
    bool m_disabled = false;
    boost::container::flat_multimap<const xAOD::UncalibratedMeasurement *, size_t> m_seedIndex;
    std::vector<size_t> m_nUsedMeasurements;
    std::vector<size_t> m_nSeedMeasurements;
    std::vector<bool> m_isDuplicateSeed;
    size_t m_nextSeed = 0u;
  };

  TrackFindingTool::TrackFindingTool(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent)
      : base_class(type, name, parent)
  {
  }

  TrackFindingTool::~TrackFindingTool() = default;

  StatusCode TrackFindingTool::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_skipDuplicateSeeds);
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
    if (!m_trackStatePrinter.empty())
    {
      ATH_CHECK(m_trackStatePrinter.retrieve());
    }

    m_logger = makeActsAthenaLogger(this, "Acts");

    auto magneticField = std::make_unique<ATLASMagneticFieldWrapper>();
    auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();

    Stepper stepper(std::move(magneticField));
    Navigator::Config cfg{trackingGeometry};
    cfg.resolvePassive = false;
    cfg.resolveMaterial = true;
    cfg.resolveSensitive = true;
    Navigator navigator(cfg);
    Propagator propagator(std::move(stepper), std::move(navigator), logger().cloneWithSuffix("Prop"));

    Acts::MeasurementSelector::Config measurementSelectorCfg{{Acts::GeometryIdentifier(),
                                                              {m_etaBins, m_chi2CutOff, m_numMeasurementsCutOff}}};

    m_trackFinder.reset(static_cast<CKF_pimpl *>(new CKF_config{{std::move(propagator), logger().cloneWithSuffix("CKF")}, measurementSelectorCfg, {}, {}}));

    m_trackFinder->pOptions.maxSteps = m_maxPropagationStep;

    m_trackFinder->ckfExtensions.updater.connect<&gainMatrixUpdate>();
    m_trackFinder->ckfExtensions.smoother.connect<&gainMatrixSmoother>();
    m_trackFinder->ckfExtensions.calibrator.connect<&ATLASSourceLinkCalibrator::calibrate<ActsTrk::TrackStateBackend, ATLASUncalibSourceLink>>();
    m_trackFinder->ckfExtensions.measurementSelector.connect<&Acts::MeasurementSelector::select<ActsTrk::TrackStateBackend>>(&m_trackFinder->measurementSelector);

    return StatusCode::SUCCESS;
  }

  // finalize
  StatusCode TrackFindingTool::finalize()
  {
    ATH_MSG_INFO(name() << " statistics:");
    ATH_MSG_INFO("- total seeds: " << m_nTotalSeeds);
    ATH_MSG_INFO("- duplicate seeds: " << m_nDuplicateSeeds);
    ATH_MSG_INFO("- failed seeds: " << m_nFailedSeeds);
    ATH_MSG_INFO("- output tracks: " << m_nOutputTracks);
    ATH_MSG_INFO("- failure ratio: " << static_cast<double>(m_nFailedSeeds) / m_nTotalSeeds);
    ATH_MSG_INFO("- duplication ratio: " << static_cast<double>(m_nDuplicateSeeds) / m_nTotalSeeds);
    return StatusCode::SUCCESS;
  }

  std::unique_ptr<ActsTrk::ITrackFindingTool::Measurements>
  ActsTrk::TrackFindingTool::initMeasurements(size_t numMeasurements) const
  {
    return std::unique_ptr<ActsTrk::ITrackFindingTool::Measurements>(new Measurements_impl(numMeasurements,
                                                                                           &*m_ATLASConverterTool,
                                                                                           m_trackStatePrinter.empty() ? nullptr : &*m_trackStatePrinter));
  }

  StatusCode
  TrackFindingTool::findTracks(const EventContext &ctx,
                               const Measurements &measurements,
                               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
                               const ActsTrk::SeedContainer *seeds,
                               ActsTrk::TrackContainer &tracksContainer,
                               ::TrackCollection &tracksCollection,
                               size_t seedCollectionIndex,
                               const char *seedType) const
  {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    if (seeds && seeds->size() != estimatedTrackParameters.size())
    {
      // should be the same, but we can cope if not
      ATH_MSG_WARNING("Have " << seeds->size() << " " << seedType << " seeds, but " << estimatedTrackParameters.size() << "estimated track parameters");
    }

    // Get measurement source links. We cast to Measurements_impl to give access to this file's local data members.
    auto meas = dynamic_cast<const Measurements_impl &>(measurements);

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    // CalibrationContext converter not implemented yet.
    Acts::CalibrationContext calContext = Acts::CalibrationContext();

    UncalibSourceLinkAccessor slAccessor;
    slAccessor.container = &meas.sourceLinks();
    Acts::SourceLinkAccessorDelegate<UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&UncalibSourceLinkAccessor::range>(&slAccessor);

    // Set the CombinatorialKalmanFilter options
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<UncalibSourceLinkAccessor::Iterator, ActsTrk::TrackStateBackend>;
    TrackFinderOptions options(tgContext,
                               mfContext,
                               calContext,
                               slAccessorDelegate,
                               m_trackFinder->ckfExtensions,
                               m_trackFinder->pOptions,
                               &(*pSurface));

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << estimatedTrackParameters.size() << ' ' << seedType << " seeds.");

    m_nTotalSeeds += estimatedTrackParameters.size();
    size_t addTracks = 0;

    DuplicateSeedDetector duplicateSeedDetector(seeds, meas.sourceLinkVec(seedCollectionIndex), m_skipDuplicateSeeds);
    size_t measurementOffset = meas.measurementOffset(seedCollectionIndex);

    // Loop over the track finding results for all initial parameters
    for (std::size_t iseed = 0; iseed < estimatedTrackParameters.size(); ++iseed)
    {
      if (!estimatedTrackParameters[iseed])
      {
        ATH_MSG_WARNING("No " << seedType << " seed " << iseed);
        ++m_nFailedSeeds;
        continue;
      }

      const Acts::BoundTrackParameters &initialParameters = *estimatedTrackParameters[iseed];

      if (!m_trackStatePrinter.empty() && seeds)
      {
        m_trackStatePrinter->printSeed(tgContext, *(*seeds)[iseed], initialParameters, measurementOffset, iseed, iseed == 0 ? estimatedTrackParameters.size() : 0, seedType);
      }

      if (duplicateSeedDetector.isDuplicate(iseed))
      {
        ATH_MSG_DEBUG("skip " << seedType << " seed " << iseed << " - already found");
        ++m_nDuplicateSeeds;
        continue;
      }

      // Get the Acts tracks, given this seed
      // Result here contains a vector of TrackProxy objects

      auto result = m_trackFinder->ckf.findTracks(initialParameters, options, tracksContainer);

      // The result for this seed
      if (not result.ok())
      {
        ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
        ++m_nFailedSeeds;
        continue;
      }
      // Get the track finding output and add to tracksCollection
      size_t ntracks = makeTracks(ctx, tgContext, tracksContainer, result.value(), tracksCollection, duplicateSeedDetector);
      addTracks += ntracks;

      if (!m_trackStatePrinter.empty())
      {
        m_trackStatePrinter->printTracks(tgContext, tracksContainer, result.value(), meas.sourceLinkVec(seedCollectionIndex), measurementOffset);
      }

      if (ntracks == 0)
      {
        ATH_MSG_WARNING("Track finding found no track candidates for " << seedType << " seed " << iseed);
        ++m_nFailedSeeds;
      }
    }

    m_nOutputTracks += addTracks;

    ATH_MSG_DEBUG("Completed " << seedType << " track finding with " << addTracks << " track candidates.");

    return StatusCode::SUCCESS;
  }

  /// based on Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  size_t
  TrackFindingTool::makeTracks(const EventContext &ctx,
                               const Acts::GeometryContext &tgContext,
                               const ActsTrk::TrackContainer &tracksContainer,
                               const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                               ::TrackCollection &tracksCollection,
                               TrackFindingTool::DuplicateSeedDetector &duplicateSeedDetector) const
  {
    size_t ntracks = 0;
    for (auto &track : fitResult)
    {
      const auto lastMeasurementIndex = track.tipIndex();

      auto finalTrajectory = DataVector<const Trk::TrackStateOnSurface>();
      // initialise the number of dead Pixel and Acts strip
      int numberOfDeadPixel = 0;
      int numberOfDeadSCT = 0;
      duplicateSeedDetector.newTrajectory();

      std::vector<std::unique_ptr<const Acts::BoundTrackParameters>> actsSmoothedParam;
      // Loop over all the output state to create track state
      tracksContainer.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&](const ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
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
            if (flag.test(Acts::TrackStateFlag::HoleFlag))
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.predicted(),
                                                         state.predictedCovariance());
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);

              // Check if this is a hole, a dead sensors or a state outside the sensor boundary
              if (boundaryCheck == Trk::BoundaryCheckResult::DeadElement)
              {
                auto *detElem = actsToDetElem(state.referenceSurface());
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
            else if (flag.test(Acts::TrackStateFlag::OutlierFlag)
                     //  || (fitResult.reversed &&     // TODO!
                     //      std::find(fitResult.passedAgainSurfaces.begin(),
                     //                fitResult.passedAgainSurfaces.end(),
                     //                state.referenceSurface().getSharedPtr().get()) == fitResult.passedAgainSurfaces.end())
            )
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.filtered(),
                                                         state.filteredCovariance());
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
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
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Measurement);
            }

            std::unique_ptr<const Trk::MeasurementBase> measState;
            if (state.hasUncalibratedSourceLink())
            {
              auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
              duplicateSeedDetector.addMeasurement(sl);
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

      // Convert the perigee state and add it to the trajectory
      const Acts::BoundTrackParameters actsPer(track.referenceSurface().getSharedPtr(),
                                               track.parameters(),
                                               track.covariance());

      std::unique_ptr<const Trk::TrackParameters> per = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsPer, tgContext);
      std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
      typePattern.set(Trk::TrackStateOnSurface::Perigee);
      auto perState = new Trk::TrackStateOnSurface(nullptr,
                                                   std::move(per),
                                                   nullptr,
                                                   typePattern);
      finalTrajectory.insert(finalTrajectory.begin(), perState);

      // Create the track using Athena TrackFitter::KalmanFitter and TrackPatternRecoInfo::SiSPSeededFinder algorithm enums
      Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::KalmanFitter, Trk::noHypothesis);
      newInfo.setPatternRecognitionInfo(Trk::TrackInfo::TrackPatternRecoInfo::SiSPSeededFinder);

      auto newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
      // TODO: use TrackSummaryTool to create trackSummary?
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
      tracksCollection.push_back(std::move(newtrack));
      ++ntracks;
    }

    return ntracks;
  }

  std::unique_ptr<const Trk::MeasurementBase>
  TrackFindingTool::makeRIO_OnTrack(const xAOD::UncalibratedMeasurement &uncalibMeas,
                                    const Trk::TrackParameters *parm) const
  {
    const Trk::PrepRawData *rio = nullptr;
    const xAOD::UncalibMeasType measurementType = uncalibMeas.type();

    if (measurementType == xAOD::UncalibMeasType::PixelClusterType)
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::PixelClusterCollection>> pixelLinkAcc("pixelClusterLink");
      auto pixcl = dynamic_cast<const xAOD::PixelCluster *>(&uncalibMeas);
      if (not pixcl or not pixelLinkAcc.isAvailable(*pixcl))
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
    else if (measurementType == xAOD::UncalibMeasType::StripClusterType)
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::SCT_ClusterCollection>> stripLinkAcc("sctClusterLink");
      auto stripcl = dynamic_cast<const xAOD::StripCluster *>(&uncalibMeas);
      if (not stripcl or not stripLinkAcc.isAvailable(*stripcl))
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
