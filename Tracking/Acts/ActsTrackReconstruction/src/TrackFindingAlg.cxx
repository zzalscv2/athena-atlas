/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingAlg.h"
#include "src/TrackFindingData.h"

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
#include "AthenaMonitoringKernel/Monitored.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/SourceLinkAccessorConcept.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
// ACTS glue
#include "ActsEvent/TrackContainer.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"

// Other
#include <sstream>
#include <functional>
#include <tuple>
#include <algorithm>

namespace {
   template <class T_MeasurementContainer>
   std::pair< xAOD::DetectorIDHashType, bool> getMaxHashAndCheckOrder(const T_MeasurementContainer &measurements) {
      std::pair< xAOD::DetectorIDHashType, bool> max_hash_ordered { 0, true };
      for (const auto &measurement :  measurements) {
         xAOD::DetectorIDHashType id_hash = measurement->identifierHash();
         max_hash_ordered.second =  (id_hash >= max_hash_ordered.first);
         max_hash_ordered.first = std::max( max_hash_ordered.first, id_hash );
      }
      return max_hash_ordered;
   }

   void gatherGeoIds(const ActsTrk::IActsToTrkConverterTool &converter_tool,
                     const InDetDD::SiDetectorElementCollection &detectorElements,
                     std::vector<Acts::GeometryIdentifier> &geo_ids) {
      for (const auto *det_el :  detectorElements) {
         const Acts::Surface &surface =
            converter_tool.trkSurfaceToActsSurface(det_el->surface());
         geo_ids.push_back(surface.geometryId());
      }
   }
}

namespace ActsTrk
{
  struct TrackFindingAlg::CKF_pimpl : public CKF_config
  {
  };

  TrackFindingAlg::CKF_pimpl &TrackFindingAlg::trackFinder() { return *m_trackFinder; }
  const TrackFindingAlg::CKF_pimpl &TrackFindingAlg::trackFinder() const { return *m_trackFinder; }

  TrackFindingAlg::TrackFindingAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  TrackFindingAlg::~TrackFindingAlg() = default;

  StatusCode TrackFindingAlg::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << " ... ");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_skipDuplicateSeeds);
    ATH_MSG_DEBUG("   " << m_etaBins);
    ATH_MSG_DEBUG("   " << m_chi2CutOff);
    ATH_MSG_DEBUG("   " << m_numMeasurementsCutOff);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_etaMin);
    ATH_MSG_DEBUG("   " << m_etaMax);
    ATH_MSG_DEBUG("   " << m_absEtaMin);
    ATH_MSG_DEBUG("   " << m_absEtaMax);
    ATH_MSG_DEBUG("   " << m_ptMin);
    ATH_MSG_DEBUG("   " << m_ptMax);
    ATH_MSG_DEBUG("   " << m_minMeasurements);

    // Read and Write handles
    ATH_CHECK(m_pixelSeedsKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripSeedsKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_pixelClusterContainerKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripClusterContainerKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_pixelDetEleCollKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripDetEleCollKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_pixelEstimatedTrackParametersKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripEstimatedTrackParametersKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_sourceLinksOutKey.initialize());

    ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_trackStatePrinter.retrieve(EnableTool{not m_trackStatePrinter.empty()}));

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

    Acts::TrackSelector::Config trackSelectorCfg;
    trackSelectorCfg.phiMin = m_phiMin;
    trackSelectorCfg.phiMax = m_phiMax;
    trackSelectorCfg.etaMin = m_etaMin;
    trackSelectorCfg.etaMax = m_etaMax;
    trackSelectorCfg.absEtaMin = m_absEtaMin;
    trackSelectorCfg.absEtaMax = m_absEtaMax;
    trackSelectorCfg.ptMin = m_ptMin;
    trackSelectorCfg.ptMax = m_ptMax;
    trackSelectorCfg.minMeasurements = m_minMeasurements;

    m_trackFinder.reset(new CKF_pimpl{CKF_config{{std::move(propagator), logger().cloneWithSuffix("CKF")}, measurementSelectorCfg, {}, {}, trackSelectorCfg}});

    trackFinder().pOptions.maxSteps = m_maxPropagationStep;

    trackFinder().ckfExtensions.updater.connect<&gainMatrixUpdate>();
    trackFinder().ckfExtensions.smoother.connect<&gainMatrixSmoother>();
    trackFinder().ckfExtensions.calibrator.connect<&ATLASSourceLinkCalibrator::calibrate<ActsTrk::TrackStateBackend, ATLASUncalibSourceLink>>();
    trackFinder().ckfExtensions.measurementSelector.connect<&Acts::MeasurementSelector::select<ActsTrk::TrackStateBackend>>(&trackFinder().measurementSelector);

    return StatusCode::SUCCESS;
  }

  // finalize
  StatusCode TrackFindingAlg::finalize()
  {
    ATH_MSG_INFO(name() << " statistics:");
    ATH_MSG_INFO("- total seeds: " << m_nTotalSeeds);
    ATH_MSG_INFO("- duplicate seeds: " << m_nDuplicateSeeds);
    ATH_MSG_INFO("- failed seeds: " << m_nFailedSeeds);
    ATH_MSG_INFO("- output tracks: " << m_nOutputTracks);
    ATH_MSG_INFO("- selected tracks: " << m_nSelectedTracks);
    ATH_MSG_INFO("- failure ratio: " << static_cast<double>(m_nFailedSeeds) / m_nTotalSeeds);
    ATH_MSG_INFO("- duplication ratio: " << static_cast<double>(m_nDuplicateSeeds) / m_nTotalSeeds);
    return StatusCode::SUCCESS;
  }

  StatusCode TrackFindingAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
    auto mon = Monitored::Group(m_monTool, timer);

    // ================================================== //
    // ===================== INPUTS ===================== //
    // ================================================== //

    // SEED PARAMETERS
    const ActsTrk::BoundTrackParametersContainer *pixelEstimatedTrackParameters = nullptr;
    if (!m_pixelEstimatedTrackParametersKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_pixelEstimatedTrackParametersKey.key());
      SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> pixelEstimatedTrackParametersHandle = SG::makeHandle(m_pixelEstimatedTrackParametersKey, ctx);
      ATH_CHECK(pixelEstimatedTrackParametersHandle.isValid());
      pixelEstimatedTrackParameters = pixelEstimatedTrackParametersHandle.get();
      ATH_MSG_DEBUG("Retrieved " << pixelEstimatedTrackParameters->size() << " input elements from key " << m_pixelEstimatedTrackParametersKey.key());
    }

    const ActsTrk::BoundTrackParametersContainer *stripEstimatedTrackParameters = nullptr;
    if (!m_stripEstimatedTrackParametersKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_stripEstimatedTrackParametersKey.key());
      SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> stripEstimatedTrackParametersHandle = SG::makeHandle(m_stripEstimatedTrackParametersKey, ctx);
      ATH_CHECK(stripEstimatedTrackParametersHandle.isValid());
      stripEstimatedTrackParameters = stripEstimatedTrackParametersHandle.get();
      ATH_MSG_DEBUG("Retrieved " << stripEstimatedTrackParameters->size() << " input elements from key " << m_stripEstimatedTrackParametersKey.key());
    }

    // SEED TRIPLETS
    const ActsTrk::SeedContainer *pixelSeeds = nullptr;
    if (!m_pixelSeedsKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_pixelSeedsKey.key());
      SG::ReadHandle<ActsTrk::SeedContainer> pixelSeedsHandle = SG::makeHandle(m_pixelSeedsKey, ctx);
      ATH_CHECK(pixelSeedsHandle.isValid());
      pixelSeeds = pixelSeedsHandle.get();
      ATH_MSG_DEBUG("Retrieved " << pixelSeeds->size() << " input elements from key " << m_pixelSeedsKey.key());
    }

    const ActsTrk::SeedContainer *stripSeeds = nullptr;
    if (!m_stripSeedsKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_stripSeedsKey.key());
      SG::ReadHandle<ActsTrk::SeedContainer> stripSeedsHandle = SG::makeHandle(m_stripSeedsKey, ctx);
      ATH_CHECK(stripSeedsHandle.isValid());
      stripSeeds = stripSeedsHandle.get();
      ATH_MSG_DEBUG("Retrieved " << stripSeeds->size() << " input elements from key " << m_stripSeedsKey.key());
    }

    // MEASUREMENTS
    const xAOD::PixelClusterContainer *pixelClusterContainer = nullptr;
    if (!m_pixelClusterContainerKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_pixelClusterContainerKey.key());
      SG::ReadHandle<xAOD::PixelClusterContainer> pixelClusterContainerHandle = SG::makeHandle(m_pixelClusterContainerKey, ctx);
      ATH_CHECK(pixelClusterContainerHandle.isValid());
      pixelClusterContainer = pixelClusterContainerHandle.get();
      ATH_MSG_DEBUG("Retrieved " << pixelClusterContainer->size() << " input elements from key " << m_pixelClusterContainerKey.key());
    }

    const xAOD::StripClusterContainer *stripClusterContainer = nullptr;
    if (!m_stripClusterContainerKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_stripClusterContainerKey.key());
      SG::ReadHandle<xAOD::StripClusterContainer> stripClusterContainerHandle = SG::makeHandle(m_stripClusterContainerKey, ctx);
      ATH_CHECK(stripClusterContainerHandle.isValid());
      stripClusterContainer = stripClusterContainerHandle.get();
      ATH_MSG_DEBUG("Retrieved " << stripClusterContainer->size() << " input elements from key " << m_stripClusterContainerKey.key());
    }

    const InDetDD::SiDetectorElementCollection *pixelDetEleColl = nullptr;
    if (!m_pixelDetEleCollKey.empty())
    {
      ATH_MSG_DEBUG("Reading input condition data with key " << m_pixelDetEleCollKey.key());
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleCollHandle(m_pixelDetEleCollKey, ctx);
      ATH_CHECK(pixelDetEleCollHandle.isValid());
      pixelDetEleColl = pixelDetEleCollHandle.retrieve();
      if (pixelDetEleColl == nullptr)
      {
        ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Retrieved " << pixelDetEleColl->size() << " input condition elements from key " << m_pixelDetEleCollKey.key());
    }

    const InDetDD::SiDetectorElementCollection *stripDetEleColl = nullptr;
    if (!m_stripDetEleCollKey.empty())
    {
      ATH_MSG_DEBUG("Reading input condition data with key " << m_stripDetEleCollKey.key());
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleCollHandle(m_stripDetEleCollKey, ctx);
      ATH_CHECK(stripDetEleCollHandle.isValid());
      stripDetEleColl = stripDetEleCollHandle.retrieve();
      if (stripDetEleColl == nullptr)
      {
        ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Retrieved " << stripDetEleColl->size() << " input condition elements from key " << m_stripDetEleCollKey.key());
    }

    // convert all measurements to source links, so the CKF can find them from either strip or pixel seeds.
    SG::WriteHandle<std::vector<ATLASUncalibSourceLink::ElementsType>> sourceLinksOutHandle(m_sourceLinksOutKey, ctx);
    ATH_CHECK(sourceLinksOutHandle.record(std::make_unique<std::vector<ATLASUncalibSourceLink::ElementsType>>()));
    if (!sourceLinksOutHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write ATLASUncalibSourceLink elements collection with key " << m_sourceLinksOutKey.key());
      return StatusCode::FAILURE;
    }

    size_t numPixelMeasurements = (pixelClusterContainer ? pixelClusterContainer->size() : 0u);
    size_t numStripMeasurements = (stripClusterContainer ? stripClusterContainer->size() : 0u);
    std::array<xAOD::DetectorIDHashType, 3> max_hash{ };
    {
       std::pair< xAOD::DetectorIDHashType, bool> max_hash_ordered = getMaxHashAndCheckOrder(*pixelClusterContainer);
       if (!max_hash_ordered.second) {
          ATH_MSG_ERROR("Measurements " << m_pixelClusterContainerKey.key() << " not ordered by identifier hash." );
          return StatusCode::FAILURE;
       }
       static_assert( static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType) < max_hash.size());
       max_hash[static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType) ]=max_hash_ordered.first;
    }
    {
       std::pair< xAOD::DetectorIDHashType, bool> max_hash_ordered = getMaxHashAndCheckOrder(*stripClusterContainer);
       if (!max_hash_ordered.second) {
          ATH_MSG_ERROR("Measurements " << m_stripClusterContainerKey.key() << " not ordered by identifier hash." );
          return StatusCode::FAILURE;
       }
       static_assert( static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType) < max_hash.size());
       max_hash[static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)]=max_hash_ordered.first;
    }

    // @TODO make this condition data
    std::vector< Acts::GeometryIdentifier > geo_ids;
    geo_ids.reserve(   max_hash[ static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType)]
                     + max_hash[static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)]);
    gatherGeoIds(*m_ATLASConverterTool, *pixelDetEleColl, geo_ids);
    gatherGeoIds(*m_ATLASConverterTool, *stripDetEleColl, geo_ids);
    std::sort( geo_ids.begin(), geo_ids.end());

    TrackFindingMeasurements measurements(numPixelMeasurements + numStripMeasurements,
                                          std::max(numPixelMeasurements, numStripMeasurements),
                                          geo_ids,
                                          !m_trackStatePrinter.empty(),
                                          sourceLinksOutHandle.ptr());

    DuplicateSeedDetector duplicateSeedDetector(((pixelSeeds ? pixelSeeds->size() : 0u) +
                                                 (stripSeeds ? stripSeeds->size() : 0u)),
                                                m_skipDuplicateSeeds);

    if (pixelClusterContainer && pixelDetEleColl)
    {
      ATH_MSG_DEBUG("Create " << pixelClusterContainer->size() << " source links from pixel measurements");
      measurements.addMeasurements(0, ctx, *pixelClusterContainer, *pixelDetEleColl, pixelSeeds,
                                   m_ATLASConverterTool, m_trackStatePrinter, duplicateSeedDetector);
    }
    if (stripClusterContainer && stripDetEleColl)
    {
      ATH_MSG_DEBUG("Create " << stripClusterContainer->size() << " source links from strip measurements");
      measurements.addMeasurements(1, ctx, *stripClusterContainer, *stripDetEleColl, stripSeeds,
                                   m_ATLASConverterTool, m_trackStatePrinter, duplicateSeedDetector);
    }

    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //

    auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);

    Acts::TrackContainer tracksContainer{Acts::VectorTrackContainer{}, ActsTrk::TrackStateBackend{}};
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_trackContainerKey.key() << "` created ...");

    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //

    // Perform the track finding for all initial parameters.
    // Until the CKF can do a backward search, start with the pixel seeds
    // (will become relevant when we can remove pixel/strip duplicates).
    // Afterwards, we could start with strips where the occupancy is lower.
    if (pixelEstimatedTrackParameters && !pixelEstimatedTrackParameters->empty())
    {
      ATH_CHECK(findTracks(ctx,
                           measurements,
                           duplicateSeedDetector,
                           *pixelEstimatedTrackParameters,
                           pixelSeeds,
                           tracksContainer,
                           0,
                           "pixel"));
    }

    if (stripEstimatedTrackParameters && !stripEstimatedTrackParameters->empty())
    {
      ATH_CHECK(findTracks(ctx,
                           measurements,
                           duplicateSeedDetector,
                           *stripEstimatedTrackParameters,
                           stripSeeds,
                           tracksContainer,
                           1,
                           "strip"));
    }

    ATH_MSG_DEBUG("    \\__ Created " << tracksContainer.size() << " tracks");

    // ================================================== //
    // ===================== STORE OUTPUT =============== //
    // ================================================== //
    // TODO once have final version of containers, they need to have movable backends also here
    ActsTrk::ConstTrackStateBackend trackStateBackend(tracksContainer.trackStateContainer());
    ActsTrk::ConstTrackBackend trackBackend(tracksContainer.container());
    auto constTrackContainer = std::make_unique<ActsTrk::ConstTrackContainer>(std::move(trackBackend), std::move(trackStateBackend));
    ATH_CHECK(trackContainerHandle.record(std::move(constTrackContainer)));
    if (!trackContainerHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write TrackContainer with key " << m_trackContainerKey.key());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingAlg::findTracks(const EventContext &ctx,
                              const TrackFindingMeasurements &measurements,
                              DuplicateSeedDetector &duplicateSeedDetector,
                              const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
                              const ActsTrk::SeedContainer *seeds,
                              ActsTrk::TrackContainer &tracksContainer,
                              size_t typeIndex,
                              const char *seedType) const
  {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    if (seeds && seeds->size() != estimatedTrackParameters.size())
    {
      // should be the same, but we can cope if not
      ATH_MSG_WARNING("Have " << seeds->size() << " " << seedType << " seeds, but " << estimatedTrackParameters.size() << "estimated track parameters");
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    // CalibrationContext converter not implemented yet.
    Acts::CalibrationContext calContext = Acts::CalibrationContext();

    UncalibSourceLinkAccessor slAccessor( measurements.sourceLinkVec(),
                                          measurements.orderedGeoIds(),
                                          measurements.measurementRanges());
    Acts::SourceLinkAccessorDelegate<UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&UncalibSourceLinkAccessor::range>(&slAccessor);

    // Set the CombinatorialKalmanFilter options
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<UncalibSourceLinkAccessor::Iterator, ActsTrk::TrackStateBackend>;
    TrackFinderOptions options(tgContext,
                               mfContext,
                               calContext,
                               slAccessorDelegate,
                               trackFinder().ckfExtensions,
                               trackFinder().pOptions,
                               &(*pSurface));

    Acts::TrackContainer tracksContainerTemp{Acts::VectorTrackContainer{}, ActsTrk::TrackStateBackend{}};

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << estimatedTrackParameters.size() << ' ' << seedType << " seeds.");

    m_nTotalSeeds += estimatedTrackParameters.size();
    size_t addTracks = 0;
    size_t selTracks = 0;

    // Loop over the track finding results for all initial parameters
    for (std::size_t iseed = 0; iseed < estimatedTrackParameters.size(); ++iseed)
    {
      tracksContainerTemp.clear();

      if (!estimatedTrackParameters[iseed])
      {
        ATH_MSG_WARNING("No " << seedType << " seed " << iseed);
        ++m_nFailedSeeds;
        continue;
      }

      const Acts::BoundTrackParameters &initialParameters = *estimatedTrackParameters[iseed];

      if (!m_trackStatePrinter.empty() && seeds)
      {
        if (iseed == 0)
        {
          ATH_MSG_INFO("CKF results for " << estimatedTrackParameters.size() << ' ' << seedType << " seeds:");
        }
        m_trackStatePrinter->printSeed(tgContext, *(*seeds)[iseed], initialParameters, measurements.measurementOffset(typeIndex), iseed);
      }

      if (duplicateSeedDetector.isDuplicate(measurements.seedOffset(typeIndex) + iseed))
      {
        ATH_MSG_DEBUG("skip " << seedType << " seed " << iseed << " - already found");
        ++m_nDuplicateSeeds;
        continue;
      }

      // Get the Acts tracks, given this seed
      // Result here contains a vector of TrackProxy objects

      auto result = trackFinder().ckf.findTracks(initialParameters, options, tracksContainerTemp);

      // The result for this seed
      if (not result.ok())
      {
        ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
        ++m_nFailedSeeds;
        continue;
      }
      const auto &tracksForSeed = result.value();

      // Fill the track infos into the duplicate seed detector
      ATH_CHECK(storeSeedInfo(tracksContainerTemp, tracksForSeed, duplicateSeedDetector));

      size_t ntracks = tracksForSeed.size();
      addTracks += ntracks;

      if (!m_trackStatePrinter.empty())
      {
        m_trackStatePrinter->printTracks(tgContext, tracksContainerTemp, result.value(), measurements.sourceLinkVec());
      }

      if (ntracks == 0)
      {
        ATH_MSG_WARNING("Track finding found no track candidates for " << seedType << " seed " << iseed);
        ++m_nFailedSeeds;
      }

      // copy selected tracks into output tracksContainer
      size_t itrack = 0;
      for (auto &track : tracksForSeed)
      {
        if (trackFinder().trackSelector.isValidTrack(track))
        {
          auto destProxy = tracksContainer.getTrack(tracksContainer.addTrack());
          destProxy.copyFrom(track, true); // make sure we copy track states!
          selTracks++;
        }
        else
        {
          ATH_MSG_DEBUG("Track " << itrack << " from " << seedType << " seed " << iseed << " failed track selection");
        }
        itrack++;
      }
    }

    m_nOutputTracks += addTracks;
    m_nSelectedTracks += selTracks;

    ATH_MSG_DEBUG("Completed " << seedType << " track finding with " << addTracks << " track candidates.");

    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingAlg::storeSeedInfo(const ActsTrk::TrackContainer &tracksContainer,
                                 const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                                 DuplicateSeedDetector &duplicateSeedDetector) const
  {
    for (auto &track : fitResult)
    {
      const auto lastMeasurementIndex = track.tipIndex();
      duplicateSeedDetector.newTrajectory();

      tracksContainer.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&duplicateSeedDetector](const ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {
            // Check there is a source link
            if (not state.hasUncalibratedSourceLink())
              return;

            // Fill the duplicate selector
            auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
            duplicateSeedDetector.addMeasurement(sl);
          }); // end visitBackwards
    }         // end loop on tracks from fitResult

    return StatusCode::SUCCESS;
  }

} // namespace
