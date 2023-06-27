/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingAlg.h"

// ACTS
#include "Acts/Definitions/Units.hpp"

#include "AthenaMonitoringKernel/Monitored.h"
#include "ActsGeometry/ActsDetectorElement.h"

namespace ActsTrk
{

  TrackFindingAlg::TrackFindingAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TrackFindingAlg::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << " ... ");

    // Retrieve seed tool
    ATH_CHECK(m_trackFindingTool.retrieve());

    // Read and Write handles
    if (!m_pixelSeedsKey.empty())
      ATH_CHECK(m_pixelSeedsKey.initialize());
    if (!m_stripSeedsKey.empty())
      ATH_CHECK(m_stripSeedsKey.initialize());
    if (!m_pixelClusterContainerKey.empty())
      ATH_CHECK(m_pixelClusterContainerKey.initialize());
    if (!m_stripClusterContainerKey.empty())
      ATH_CHECK(m_stripClusterContainerKey.initialize());
    if (!m_pixelDetEleCollKey.empty())
      ATH_CHECK(m_pixelDetEleCollKey.initialize());
    if (!m_stripDetEleCollKey.empty())
      ATH_CHECK(m_stripDetEleCollKey.initialize());
    if (!m_pixelEstimatedTrackParametersKey.empty())
      ATH_CHECK(m_pixelEstimatedTrackParametersKey.initialize());
    if (!m_stripEstimatedTrackParametersKey.empty())
      ATH_CHECK(m_stripEstimatedTrackParametersKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());
    ATH_CHECK(m_tracksContainerKey.initialize());

    if (not m_monTool.empty())
      ATH_CHECK(m_monTool.retrieve());

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

    auto measurements = m_trackFindingTool->initMeasurements((pixelClusterContainer ? pixelClusterContainer->size() : 0) +
                                                             (stripClusterContainer ? stripClusterContainer->size() : 0));
    if (pixelClusterContainer && pixelDetEleColl)
    {
      ATH_MSG_DEBUG("Create " << pixelClusterContainer->size() << " source links from pixel measurements");
      measurements->addMeasurements(0, ctx, pixelClusterContainer, pixelDetEleColl);
    }
    if (stripClusterContainer && stripDetEleColl)
    {
      ATH_MSG_DEBUG("Create " << stripClusterContainer->size() << " source links from strip measurements");
      measurements->addMeasurements(1, ctx, stripClusterContainer, stripDetEleColl);
    }

    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //

    SG::WriteHandle<::TrackCollection> trackHandle = SG::makeHandle(m_tracksKey, ctx);
    auto trackCollection = std::make_unique<::TrackCollection>();
    ATH_MSG_DEBUG("    \\__ Tracks Collection `" << m_tracksKey.key() << "` created ...");

    auto trackContainerHandle = SG::makeHandle(m_tracksContainerKey, ctx);
    Acts::TrackContainer trackContainer{Acts::VectorTrackContainer{},
                                        ActsTrk::TrackStateBackend{}};
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_tracksContainerKey.key() << "` created ...");

    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //

    // Perform the track finding for all initial parameters.
    // Until the CKF can do a backward search, start with the pixel seeds
    // (will become relevant when we can remove pixel/strip duplicates).
    // Afterwards, we could start with strips where the occupancy is lower.
    if (pixelEstimatedTrackParameters && !pixelEstimatedTrackParameters->empty())
    {
      ATH_CHECK(m_trackFindingTool->findTracks(ctx,
                                               *measurements,
                                               *pixelEstimatedTrackParameters,
                                               pixelSeeds,
                                               trackContainer,
                                               *trackCollection.get(),
                                               0,
                                               "pixel"));
    }

    if (stripEstimatedTrackParameters && !stripEstimatedTrackParameters->empty())
    {
      ATH_CHECK(m_trackFindingTool->findTracks(ctx,
                                               *measurements,
                                               *stripEstimatedTrackParameters,
                                               stripSeeds,
                                               trackContainer,
                                               *trackCollection.get(),
                                               1,
                                               "strip"));
    }

    ATH_MSG_DEBUG("    \\__ Created " << trackCollection->size() << " tracks");

    // ================================================== //
    // ===================== STORE OUTPUT =============== //
    // ================================================== //
    // TODO once have final version of containers, they need to have movable backends also here
    ActsTrk::ConstTrackStateBackend trackStateBackend(trackContainer.trackStateContainer());
    ActsTrk::ConstTrackBackend trackBackend(trackContainer.container());
    auto constTrackContainer = std::make_unique<ActsTrk::ConstTrackContainer>(std::move(trackBackend), std::move(trackStateBackend));
    ATH_CHECK(trackContainerHandle.record(std::move(constTrackContainer)));

    ATH_MSG_DEBUG("Storing Track Collection " << m_tracksKey.key());
    ATH_CHECK(trackHandle.record(std::move(trackCollection)));

    return StatusCode::SUCCESS;
  }

} // namespace
