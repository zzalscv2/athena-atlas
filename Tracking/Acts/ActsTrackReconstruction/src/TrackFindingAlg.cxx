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
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/TrackingSurfaceHelper.h"
#include "ActsInterop/Logger.h"

#include "TableUtils.h"
#include "MeasurementCalibrator.h"
// Other
#include <sstream>
#include <functional>
#include <tuple>
#include <algorithm>

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

  // === initialize ==========================================================

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
    ATH_MSG_DEBUG("   " << m_statEtaBins);
    ATH_MSG_DEBUG("   " << m_seedLabels);
    ATH_MSG_DEBUG("   " << m_dumpAllStatEtaBins);

    // Read and Write handles
    ATH_CHECK(m_seedContainerKeys.initialize());
    ATH_CHECK(m_uncalibratedMeasurementContainerKeys.initialize());
    ATH_CHECK(m_detEleCollKeys.initialize());
    ATH_CHECK(m_estimatedTrackParametersKeys.initialize());
    ATH_CHECK(m_tracksBackendHandle.initialize());
    ATH_CHECK(m_trackContainerKey.initialize());

    if (m_estimatedTrackParametersKeys.size() != m_seedLabels.size())
    {
      ATH_MSG_FATAL("There are " << m_seedLabels.size() << " SeedLabels, but " << m_estimatedTrackParametersKeys.size() << " EstimatedTrackParametersKeys");
      return StatusCode::FAILURE;
    }

    if (m_seedContainerKeys.size() != m_estimatedTrackParametersKeys.size())
    {
      ATH_MSG_FATAL("There are " << m_estimatedTrackParametersKeys.size() << " EstimatedTrackParametersKeys, but " << m_seedContainerKeys.size() << " SeedContainerKeys");
      return StatusCode::FAILURE;
    }

    // @TODO can we remove this requirement to allow PPP seeds with pixel+strip measurements, or even PPS seeds?
    if (m_seedContainerKeys.size() != m_uncalibratedMeasurementContainerKeys.size())
    {
      ATH_MSG_FATAL("There are " << m_uncalibratedMeasurementContainerKeys.size() << " UncalibratedMeasurementContainerKeys, but " << m_seedContainerKeys.size() << " SeedContainerKeys");
      return StatusCode::FAILURE;
    }

    if (m_detEleCollKeys.size() != m_uncalibratedMeasurementContainerKeys.size())
    {
      ATH_MSG_FATAL("There are " << m_uncalibratedMeasurementContainerKeys.size() << " UncalibratedMeasurementContainerKeys, but " << m_detEleCollKeys.size() << " DetEleCollKeys");
      return StatusCode::FAILURE;
    }

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
    Navigator navigator(cfg, logger().cloneWithSuffix("Navigator"));
    Propagator propagator(std::move(stepper), std::move(navigator), logger().cloneWithSuffix("Prop"));

    std::vector<double> etaBins;
    // m_etaBins (from flags.Tracking.ActiveConfig.etaBins) includes a dummy first and last bin, which we ignore
    if (m_etaBins.size() > 2)
      etaBins.assign(m_etaBins.begin() + 1, m_etaBins.end() - 1);
    Acts::MeasurementSelectorCuts measurementSelectorCuts{etaBins};

    if (!m_chi2CutOff.empty())
      measurementSelectorCuts.chi2CutOff = m_chi2CutOff;
    if (!m_numMeasurementsCutOff.empty())
      measurementSelectorCuts.numMeasurementsCutOff = m_numMeasurementsCutOff;

    Acts::MeasurementSelector::Config measurementSelectorCfg{{Acts::GeometryIdentifier(), std::move(measurementSelectorCuts)}};

    std::vector<double> absEtaEdges;
    absEtaEdges.reserve(etaBins.size() + 2);
    if (etaBins.empty())
    {
      absEtaEdges.push_back(0.0);
      absEtaEdges.push_back(std::numeric_limits<double>::infinity());
    }
    else
    {
      absEtaEdges.push_back(m_absEtaMin);
      absEtaEdges.insert(absEtaEdges.end(), etaBins.begin(), etaBins.end());
      absEtaEdges.push_back(m_absEtaMax);
    }

    auto setCut = [](auto &cfgVal, const auto &cuts, size_t ind) -> void
    {
      if (cuts.empty())
        return;
      cfgVal = (ind < cuts.size()) ? cuts[ind] : cuts[cuts.size() - 1];
    };

    Acts::TrackSelector::EtaBinnedConfig trackSelectorCfg{std::move(absEtaEdges)};
    if (etaBins.empty())
    {
      assert(trackSelectorCfg.cutSets.size() == 1);
      trackSelectorCfg.cutSets[0].absEtaMin = m_absEtaMin;
      trackSelectorCfg.cutSets[0].absEtaMax = m_absEtaMax;
    }
    size_t cutIndex = 0;
    for (auto &cfg : trackSelectorCfg.cutSets)
    {
      setCut(cfg.phiMin, m_phiMin, cutIndex);
      setCut(cfg.phiMax, m_phiMax, cutIndex);
      setCut(cfg.etaMin, m_etaMin, cutIndex);
      setCut(cfg.etaMax, m_etaMax, cutIndex);
      setCut(cfg.ptMin, m_ptMin, cutIndex);
      setCut(cfg.ptMax, m_ptMax, cutIndex);
      setCut(cfg.minMeasurements, m_minMeasurements, cutIndex);
      ++cutIndex;
    }

    ATH_MSG_DEBUG(trackSelectorCfg);

    m_trackFinder.reset(new CKF_pimpl{CKF_config{{std::move(propagator), logger().cloneWithSuffix("CKF")}, measurementSelectorCfg, {}, {}, trackSelectorCfg}});

    trackFinder().pOptions.maxSteps = m_maxPropagationStep;

    trackFinder().ckfExtensions.updater.connect<&gainMatrixUpdate>();
    trackFinder().ckfExtensions.smoother.connect<&gainMatrixSmoother>();
    trackFinder().ckfExtensions.measurementSelector.connect<&Acts::MeasurementSelector::select<ActsTrk::MutableTrackStateBackend>>(&trackFinder().measurementSelector);

    initStatTables();

    return StatusCode::SUCCESS;
  }

  // === finalize ============================================================

  StatusCode TrackFindingAlg::finalize()
  {
    printStatTables();
    return StatusCode::SUCCESS;
  }

  // === execute =============================================================

  StatusCode TrackFindingAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
    auto mon = Monitored::Group(m_monTool, timer);

    // ================================================== //
    // ===================== INPUTS ===================== //
    // ================================================== //

    // SEED PARAMETERS
    std::vector<const ActsTrk::BoundTrackParametersContainer *> estimatedTrackParametersContainers;
    estimatedTrackParametersContainers.reserve(m_estimatedTrackParametersKeys.size());
    for (const auto &estimatedTrackParametersKey : m_estimatedTrackParametersKeys)
    {
      ATH_MSG_DEBUG("Reading input collection with key " << estimatedTrackParametersKey.key());
      SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> estimatedTrackParametersHandle = SG::makeHandle(estimatedTrackParametersKey, ctx);
      ATH_CHECK(estimatedTrackParametersHandle.isValid());
      estimatedTrackParametersContainers.push_back(estimatedTrackParametersHandle.cptr());
      ATH_MSG_DEBUG("Retrieved " << estimatedTrackParametersContainers.back()->size() << " input elements from key " << estimatedTrackParametersKey.key());
    }

    // SEED TRIPLETS
    std::vector<const ActsTrk::SeedContainer *> seedContainers;
    seedContainers.reserve(m_seedContainerKeys.size());
    std::size_t total_seeds = 0;
    for (const auto &seedContainerKey : m_seedContainerKeys)
    {
      ATH_MSG_DEBUG("Reading input collection with key " << seedContainerKey.key());
      SG::ReadHandle<ActsTrk::SeedContainer> seedsHandle = SG::makeHandle(seedContainerKey, ctx);
      ATH_CHECK(seedsHandle.isValid());
      seedContainers.push_back(seedsHandle.cptr());
      ATH_MSG_DEBUG("Retrieved " << seedContainers.back()->size() << " input elements from key " << seedContainerKey.key());
      total_seeds += seedContainers.back()->size();
    }

    // MEASUREMENTS
    std::vector<const xAOD::UncalibratedMeasurementContainer *> uncalibratedMeasurementContainers;
    std::vector<xAOD::UncalibMeasType> measType;
    std::array<std::size_t, TrackingSurfaceHelper::s_NMeasTypes> measCount{};
    uncalibratedMeasurementContainers.reserve(m_uncalibratedMeasurementContainerKeys.size());
    measType.reserve(m_uncalibratedMeasurementContainerKeys.size());
    std::size_t measTotal = 0;
    for (const auto &uncalibratedMeasurementContainerKey : m_uncalibratedMeasurementContainerKeys)
    {
      ATH_MSG_DEBUG("Reading input collection with key " << uncalibratedMeasurementContainerKey.key());
      SG::ReadHandle<xAOD::UncalibratedMeasurementContainer> uncalibratedMeasurementContainerHandle = SG::makeHandle(uncalibratedMeasurementContainerKey, ctx);
      ATH_CHECK(uncalibratedMeasurementContainerHandle.isValid());
      uncalibratedMeasurementContainers.push_back(uncalibratedMeasurementContainerHandle.cptr());
      ATH_MSG_DEBUG("Retrieved " << uncalibratedMeasurementContainers.back()->size() << " input elements from key " << uncalibratedMeasurementContainerKey.key());

      if (!checkHashOrder(*uncalibratedMeasurementContainers.back()))
      {
        ATH_MSG_ERROR("Measurements " << uncalibratedMeasurementContainerKey.key() << " not ordered by identifier hash.");
        return StatusCode::FAILURE;
      }

      xAOD::UncalibMeasType typ = !uncalibratedMeasurementContainers.back()->empty()
                                      ? uncalibratedMeasurementContainers.back()->at(0)->type()
                                      : xAOD::UncalibMeasType::Other;
      auto ind = static_cast<std::size_t>(typ);
      if (!(ind < TrackingSurfaceHelper::s_NMeasTypes))
      {
        ATH_MSG_FATAL("Measurements " << uncalibratedMeasurementContainerKey.key() << " type " << ind << " larger than " << TrackingSurfaceHelper::s_NMeasTypes - 1);
        return StatusCode::FAILURE;
      }
      measType.push_back(typ);
      measCount.at(ind) += uncalibratedMeasurementContainers.back()->size();
      measTotal += uncalibratedMeasurementContainers.back()->size();
    }

    std::vector<const InDetDD::SiDetectorElementCollection *> detEleColl;
    detEleColl.reserve(m_detEleCollKeys.size());
    for (const auto &detEleCollKey : m_detEleCollKeys)
    {
      ATH_MSG_DEBUG("Reading input condition data with key " << detEleCollKey.key());
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detEleCollHandle(detEleCollKey, ctx);
      ATH_CHECK(detEleCollHandle.isValid());
      detEleColl.push_back(detEleCollHandle.retrieve());
      if (detEleColl.back() == nullptr)
      {
        ATH_MSG_FATAL(detEleCollKey.fullKey() << " is not available.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Retrieved " << detEleColl.back()->size() << " input condition elements from key " << detEleCollKey.key());
    }

    // @TODO make this condition data
    std::array<std::vector<const Acts::Surface *>, TrackingSurfaceHelper::s_NMeasTypes> acts_surfaces;
    std::vector<Acts::GeometryIdentifier> geo_ids;
    geo_ids.reserve(measTotal);
    for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer)
    {
      auto ind = static_cast<std::size_t>(measType[icontainer]);
      acts_surfaces.at(ind).reserve(measCount[ind]);
      gatherGeoIds(*m_ATLASConverterTool, *detEleColl[icontainer], geo_ids, acts_surfaces.at(ind));
    }
    std::sort(geo_ids.begin(), geo_ids.end());

    TrackingSurfaceHelper tracking_surface_helper(std::move(acts_surfaces));

    TrackFindingMeasurements measurements(geo_ids, !m_trackStatePrinter.empty());

    DuplicateSeedDetector duplicateSeedDetector(total_seeds, m_skipDuplicateSeeds);

    for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer)
    {
      if (measType[icontainer] != xAOD::UncalibMeasType::Other)
      {
        tracking_surface_helper.setSiDetectorElements(measType[icontainer], detEleColl[icontainer]);
      }
      ATH_MSG_DEBUG("Create " << uncalibratedMeasurementContainers[icontainer]->size() << " source links from measurements in " << m_uncalibratedMeasurementContainerKeys[icontainer].key());
      measurements.addMeasurements(icontainer, *uncalibratedMeasurementContainers[icontainer], *detEleColl[icontainer], seedContainers[icontainer],
                                   m_ATLASConverterTool, m_trackStatePrinter, duplicateSeedDetector, ctx);
    }

    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //
    // TODO move closer to the place where it is used
    auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);

    ActsTrk::MutableTrackContainer tracksContainer;
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_trackContainerKey.key() << "` created ...");

    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //
    EventStats event_stat;
    event_stat.resize(m_stat.size());

    // Perform the track finding for all initial parameters.
    // Until the CKF can do a backward search, start with the pixel seeds
    // (will become relevant when we can remove pixel/strip duplicates).
    // Afterwards, we could start with strips where the occupancy is lower.
    for (std::size_t icontainer = 0; icontainer < estimatedTrackParametersContainers.size(); ++icontainer)
    {
      if (estimatedTrackParametersContainers[icontainer]->empty())
        continue;
      ATH_CHECK(findTracks(ctx,
                           measurements,
                           tracking_surface_helper,
                           duplicateSeedDetector,
                           *estimatedTrackParametersContainers[icontainer],
                           seedContainers[icontainer],
                           tracksContainer,
                           icontainer,
                           icontainer < m_seedLabels.size() ? m_seedLabels[icontainer].c_str() : m_seedContainerKeys[icontainer].key().c_str(),
                           event_stat));
    }

    ATH_MSG_DEBUG("    \\__ Created " << tracksContainer.size() << " tracks");

    copyStats(event_stat);

    std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = m_tracksBackendHandle.moveToConst(std::move(tracksContainer), ctx);
    ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));
    if (!trackContainerHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write TrackContainer with key " << m_trackContainerKey.key());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  // === findTracks ==========================================================

  StatusCode
  TrackFindingAlg::findTracks(const EventContext &ctx,
                              const TrackFindingMeasurements &measurements,
                              const TrackingSurfaceHelper &tracking_surface_helper,
                              DuplicateSeedDetector &duplicateSeedDetector,
                              const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
                              const ActsTrk::SeedContainer *seeds,
                              ActsTrk::MutableTrackContainer &tracksContainer,
                              size_t typeIndex,
                              const char *seedType,
                              EventStats &event_stat) const
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

    UncalibSourceLinkAccessor slAccessor(ctx,
                                         measurements.orderedGeoIds(),
                                         measurements.measurementRanges());
    Acts::SourceLinkAccessorDelegate<UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&UncalibSourceLinkAccessor::range>(&slAccessor);

    // Set the CombinatorialKalmanFilter options
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<UncalibSourceLinkAccessor::Iterator, ActsTrk::MutableTrackStateBackend>;
    TrackFinderOptions options(tgContext,
                               mfContext,
                               calContext,
                               slAccessorDelegate,
                               trackFinder().ckfExtensions,
                               trackFinder().pOptions,
                               &(*pSurface));

    ActsTrk::MutableTrackContainer tracksContainerTemp;

    UncalibratedMeasurementCalibrator<ActsTrk::MutableTrackStateBackend> calibrator(*m_ATLASConverterTool, tracking_surface_helper);
    options.extensions.calibrator.connect(calibrator);

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << estimatedTrackParameters.size() << ' ' << seedType << " seeds.");

    // Loop over the track finding results for all initial parameters
    for (std::size_t iseed = 0; iseed < estimatedTrackParameters.size(); ++iseed)
    {
      std::size_t category_i = typeIndex * (m_statEtaBins.size() + 1);
      tracksContainerTemp.clear();

      if (!estimatedTrackParameters[iseed])
      {
        ATH_MSG_WARNING("No " << seedType << " seed " << iseed);
        ++event_stat[category_i][kNoTrackParam];
        continue;
      }

      const Acts::BoundTrackParameters &initialParameters = *estimatedTrackParameters[iseed];

      category_i = getStatCategory(typeIndex, -std::log(std::tan(initialParameters.theta() / 2)));
      ++event_stat[category_i][kNTotalSeeds];

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
        ++event_stat[category_i][kNDuplicateSeeds];
        continue;
      }

      // Get the Acts tracks, given this seed
      // Result here contains a vector of TrackProxy objects
      ++event_stat[category_i][kNUsedSeeds];

      auto result = trackFinder().ckf.findTracks(initialParameters, options, tracksContainerTemp);

      // The result for this seed
      if (not result.ok())
      {
        ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
        continue;
      }
      const auto &tracksForSeed = result.value();

      if (!m_trackStatePrinter.empty())
      {
        m_trackStatePrinter->printTracks(tgContext, tracksContainerTemp, tracksForSeed, measurements.measurementOffsets());
      }

      // Fill the track infos into the duplicate seed detector
      if (m_skipDuplicateSeeds)
      {
        ATH_CHECK(storeSeedInfo(tracksContainerTemp, tracksForSeed, duplicateSeedDetector));
      }

      size_t ntracks = tracksForSeed.size();
      event_stat[category_i][kNOutputTracks] += ntracks;

      if (ntracks == 0)
      {
        ATH_MSG_WARNING("Track finding found no track candidates for " << seedType << " seed " << iseed);
        ++event_stat[category_i][kNoTrack];
      }

      // copy selected tracks into output tracksContainer
      size_t itrack = 0;
      for (auto &track : tracksForSeed)
      {
        if (trackFinder().trackSelector.isValidTrack(track))
        {
          auto destProxy = tracksContainer.getTrack(tracksContainer.addTrack());
          destProxy.copyFrom(track, true); // make sure we copy track states!
          ++event_stat[category_i][kNSelectedTracks];
        }
        else
        {
          ATH_MSG_DEBUG("Track " << itrack << " from " << seedType << " seed " << iseed << " failed track selection");
        }
        itrack++;
      }
    }

    ATH_MSG_DEBUG("Completed " << seedType << " track finding with " << computeStatSum(typeIndex, kNOutputTracks, event_stat) << " track candidates.");

    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingAlg::storeSeedInfo(const ActsTrk::MutableTrackContainer &tracksContainer,
                                 const std::vector<ActsTrk::MutableTrackContainer::TrackProxy> &fitResult,
                                 DuplicateSeedDetector &duplicateSeedDetector) const
  {
    for (auto &track : fitResult)
    {
      const auto lastMeasurementIndex = track.tipIndex();
      duplicateSeedDetector.newTrajectory();

      tracksContainer.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&duplicateSeedDetector](const ActsTrk::MutableTrackStateBackend::ConstTrackStateProxy &state) -> void
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

  // === Statistics printout =================================================

  void TrackFindingAlg::initStatTables()
  {
    if (!m_statEtaBins.empty())
    {
      m_useAbsEtaForStat = (m_statEtaBins[0] > 0.);
      float last_eta = m_statEtaBins[0];
      for (float eta : m_statEtaBins)
      {
        if (eta < last_eta)
        {
          ATH_MSG_FATAL("Eta bins for statistics counter not in ascending order.");
        }
        last_eta = eta;
      }
    }
    m_stat.resize(nSeedCollections() * seedCollectionStride());
  }

  // copy statistics
  void TrackFindingAlg::copyStats(const EventStats &event_stat) const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::size_t category_i = 0;
    for (const std::array<unsigned int, kNStat> &src_stat : event_stat)
    {
      std::array<std::size_t, kNStat> &dest_stat = m_stat[category_i++];
      for (std::size_t i = 0; i < src_stat.size(); ++i)
      {
        assert(i < dest_stat.size());
        dest_stat[i] += src_stat[i];
      }
    }
  }

  // print statistics
  void TrackFindingAlg::printStatTables() const
  {
    if (msgLvl(MSG::INFO))
    {
      std::vector<std::string> stat_labels =
          TableUtils::makeLabelVector(kNStat,
                                      {
                                          std::make_pair(kNTotalSeeds, "Input seeds"),
                                          std::make_pair(kNoTrackParam, "No track parameters"),
                                          std::make_pair(kNUsedSeeds, "Used   seeds"),
                                          std::make_pair(kNoTrack, "Cannot find track"),
                                          std::make_pair(kNDuplicateSeeds, "Duplicate seeds"),
                                          std::make_pair(kNOutputTracks, "CKF tracks"),
                                          std::make_pair(kNSelectedTracks, "selected tracks"),
                                      });
      assert(stat_labels.size() == kNStat);
      std::vector<std::string> categories;
      categories.reserve(m_seedLabels.size() + 1);
      categories.insert(categories.end(), m_seedLabels.begin(), m_seedLabels.end());
      categories.push_back("ALL");

      std::vector<std::string> eta_labels;
      eta_labels.reserve(m_statEtaBins.size() + 2);
      for (std::size_t eta_bin_i = 0; eta_bin_i < m_statEtaBins.size() + 2; ++eta_bin_i)
      {
        eta_labels.push_back(TableUtils::makeEtaBinLabel(m_statEtaBins, eta_bin_i, m_useAbsEtaForStat));
      }

      // vector used as 3D array stat[ eta_bin ][ stat_i ][ seed_type]
      // stat_i = [0, kNStat)
      // eta_bin = [0, m_statEtaBins.size()+2 ); eta_bin == m_statEtaBinsSize()+1 means sum of all etaBins
      // seed_type = [0, nSeedCollections()+1)  seed_type == nSeedCollections() means sum of all seed collections
      std::vector<std::size_t> stat =
          TableUtils::createCounterArrayWithProjections<std::size_t>(nSeedCollections(),
                                                                     m_statEtaBins.size() + 1,
                                                                     m_stat);

      // the extra columns and rows for the projections are addeded internally:
      std::size_t stat_stride =
          TableUtils::counterStride(nSeedCollections(),
                                    m_statEtaBins.size() + 1,
                                    kNStat);
      std::size_t eta_stride =
          TableUtils::subCategoryStride(nSeedCollections(),
                                        m_statEtaBins.size() + 1,
                                        kNStat);
      std::stringstream table_out;

      if (m_dumpAllStatEtaBins.value())
      {
        // dump for each counter a table with one row per eta bin
        std::size_t max_label_width = TableUtils::maxLabelWidth(stat_labels) + TableUtils::maxLabelWidth(eta_labels);
        for (std::size_t stat_i = 0; stat_i < kNStat; ++stat_i)
        {
          std::size_t dest_idx_offset = stat_i * stat_stride;
          table_out << makeTable(stat, dest_idx_offset, eta_stride,
                                 eta_labels,
                                 categories)
                           .columnWidth(10)
                           // only dump the footer for the last eta bin i.e. total
                           .dumpHeader(stat_i == 0)
                           .dumpFooter(stat_i + 1 == kNStat)
                           .separateLastRow(true) // separate the sum of all eta bins
                           .minLabelWidth(max_label_width)
                           .labelPrefix(stat_labels.at(stat_i));
        }
      }
      else
      {
        // dump one table with one row per counter showing the total eta range
        for (std::size_t eta_bin_i = (m_dumpAllStatEtaBins.value() ? 0 : m_statEtaBins.size() + 1);
             eta_bin_i < m_statEtaBins.size() + 2;
             ++eta_bin_i)
        {
          std::size_t dest_idx_offset = eta_bin_i * eta_stride;
          table_out << makeTable(stat, dest_idx_offset, stat_stride,
                                 stat_labels,
                                 categories,
                                 eta_labels.at(eta_bin_i))
                           .columnWidth(10)
                           // only dump the footer for the last eta bin i.e. total
                           .dumpFooter(!m_dumpAllStatEtaBins.value() || eta_bin_i == m_statEtaBins.size() + 1);
        }
      }
      ATH_MSG_INFO("statistics:\n"
                   << table_out.str());
      table_out.str("");

      // define retios first element numerator, second element denominator
      // each element contains a vector of counter and a multiplier e.g. +- 1
      // ratios are computed as  (sum_i stat[stat_i] *  multiplier_i ) / (sum_j stat[stat_j] *  multiplier_j )
      auto [ratio_labels, ratio_def] =
          TableUtils::splitRatioDefinitionsAndLabels({TableUtils::makeRatioDefinition("failed / seeds ",
                                                                                      std::vector<TableUtils::SummandDefinition>{
                                                                                          TableUtils::defineSummand(kNTotalSeeds, 1),
                                                                                          TableUtils::defineSummand(kNUsedSeeds, -1),
                                                                                          TableUtils::defineSummand(kNDuplicateSeeds, -1),
                                                                                          // no track counted  as used but want to include it as failed
                                                                                          TableUtils::defineSummand(kNoTrack, 1),
                                                                                      }, // failed seeds i.e. seeds which are not duplicates but did not produce a track
                                                                                      std::vector<TableUtils::SummandDefinition>{TableUtils::defineSummand(kNTotalSeeds, 1)}),
                                                      TableUtils::defineSimpleRatio("duplication / seeds", kNDuplicateSeeds, kNTotalSeeds),
                                                      TableUtils::defineSimpleRatio("selected / CKF tracks", kNSelectedTracks, kNOutputTracks),
                                                      TableUtils::defineSimpleRatio("selected tracks / used seeds", kNSelectedTracks, kNUsedSeeds)});

      std::vector<float> ratio = TableUtils::computeRatios(ratio_def,
                                                           nSeedCollections() + 1,
                                                           m_statEtaBins.size() + 2,
                                                           stat);

      // the extra columns and rows for the projections are _not_ added internally
      std::size_t ratio_stride = TableUtils::ratioStride(nSeedCollections() + 1,
                                                         m_statEtaBins.size() + 2,
                                                         ratio_def);
      std::size_t ratio_eta_stride = TableUtils::subCategoryStride(nSeedCollections() + 1,
                                                                   m_statEtaBins.size() + 2,
                                                                   ratio_def);

      std::size_t max_label_width = TableUtils::maxLabelWidth(ratio_labels) + TableUtils::maxLabelWidth(eta_labels);
      if (m_dumpAllStatEtaBins.value())
      {
        // show for each ratio a table with one row per eta bin
        for (std::size_t ratio_i = 0; ratio_i < ratio_labels.size(); ++ratio_i)
        {
          table_out << makeTable(ratio,
                                 ratio_i * ratio_stride,
                                 ratio_eta_stride,
                                 eta_labels,
                                 categories)
                           .columnWidth(10)
                           // only dump the footer for the last eta bin i.e. total
                           .dumpHeader(ratio_i == 0)
                           .dumpFooter(ratio_i + 1 == ratio_labels.size())
                           .separateLastRow(true) // separate the sum of las
                           .minLabelWidth(max_label_width)
                           .labelPrefix(ratio_labels.at(ratio_i));
        }
      }
      else
      {
        // dump one table with one row per ratio showing  the total eta range
        table_out << makeTable(ratio,
                               (m_statEtaBins.size() + 1) * ratio_eta_stride + 0 * ratio_stride,
                               ratio_stride,
                               ratio_labels,
                               categories)
                         .columnWidth(10)
                         // only dump the footer for the last eta bin i.e. total
                         .minLabelWidth(max_label_width)
                         .dumpFooter(false);

        // also dump a table for final tracks over seeds (ratio_i==3) showing one row per eta bin
        eta_labels.erase(eta_labels.end() - 1); // drop last line of table which shows again all eta bins summed.
        constexpr std::size_t ratio_i = 3;
        table_out << makeTable(ratio,
                               ratio_i * ratio_stride,
                               ratio_eta_stride,
                               eta_labels,
                               categories)
                         .columnWidth(10)
                         .dumpHeader(false)
                         // only dump the footer for the last eta bin i.e. total
                         .dumpFooter(!m_dumpAllStatEtaBins.value() || ratio_i + 1 == ratio_labels.size())
                         .separateLastRow(false)
                         .minLabelWidth(max_label_width)
                         .labelPrefix(ratio_labels.at(ratio_i));
      }

      ATH_MSG_INFO("Ratios:\n"
                   << table_out.str());
    }
  }

  inline std::size_t TrackFindingAlg::getStatCategory(std::size_t seed_collection, float eta) const
  {
    std::vector<float>::const_iterator bin_iter = std::upper_bound(m_statEtaBins.begin(),
                                                                   m_statEtaBins.end(),
                                                                   m_useAbsEtaForStat ? std::abs(eta) : eta);
    std::size_t category_i = seed_collection * seedCollectionStride() + static_cast<std::size_t>(bin_iter - m_statEtaBins.begin());
    assert(category_i < m_stat.size());
    return category_i;
  }

  inline std::size_t TrackFindingAlg::computeStatSum(std::size_t seed_collection, EStat counter_i, const EventStats &stat) const
  {
    std::size_t out = 0u;
    for (std::size_t category_i = seed_collection * seedCollectionStride() + static_cast<std::size_t>(counter_i);
         category_i < (seed_collection + 1) * seedCollectionStride();
         ++category_i)
    {
      assert(category_i < stat.size());
      out += stat[category_i][counter_i];
    }
    return out;
  }

} // namespace
