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
    ATH_CHECK(m_pixelClusterContainerKey.initialize());
    ATH_CHECK(m_stripClusterContainerKey.initialize());
    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    ATH_CHECK(m_stripDetEleCollKey.initialize());
    ATH_CHECK(m_pixelEstimatedTrackParametersKey.initialize());
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

    // SEEDS
    ATH_MSG_DEBUG("Reading input collection with key " << m_pixelEstimatedTrackParametersKey.key());
    SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> pixelEstimatedTrackParametersHandle = SG::makeHandle(m_pixelEstimatedTrackParametersKey, ctx);
    ATH_CHECK(pixelEstimatedTrackParametersHandle.isValid());
    const ActsTrk::BoundTrackParametersContainer *pixelEstimatedTrackParameters = pixelEstimatedTrackParametersHandle.get();
    ATH_MSG_DEBUG("Retrieved " << pixelEstimatedTrackParameters->size() << " input elements from key " << m_pixelEstimatedTrackParametersKey.key());

    ATH_MSG_DEBUG("Reading input collection with key " << m_stripEstimatedTrackParametersKey.key());
    SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> stripEstimatedTrackParametersHandle = SG::makeHandle(m_stripEstimatedTrackParametersKey, ctx);
    ATH_CHECK(stripEstimatedTrackParametersHandle.isValid());
    const ActsTrk::BoundTrackParametersContainer *stripEstimatedTrackParameters = stripEstimatedTrackParametersHandle.get();
    ATH_MSG_DEBUG("Retrieved " << stripEstimatedTrackParameters->size() << " input elements from key " << m_stripEstimatedTrackParametersKey.key());

    // MEASUREMENTS
    ATH_MSG_DEBUG("Reading input collection with key " << m_pixelClusterContainerKey.key());
    SG::ReadHandle<xAOD::PixelClusterContainer> pixelClusterContainerHandle = SG::makeHandle(m_pixelClusterContainerKey, ctx);
    ATH_CHECK(pixelClusterContainerHandle.isValid());
    const xAOD::PixelClusterContainer *pixelClusterContainer = pixelClusterContainerHandle.get();
    ATH_MSG_DEBUG("Retrieved " << pixelClusterContainer->size() << " input elements from key " << m_pixelClusterContainerKey.key());

    ATH_MSG_DEBUG("Reading input collection with key " << m_stripClusterContainerKey.key());
    SG::ReadHandle<xAOD::StripClusterContainer> stripClusterContainerHandle = SG::makeHandle(m_stripClusterContainerKey, ctx);
    ATH_CHECK(stripClusterContainerHandle.isValid());
    const xAOD::StripClusterContainer *stripClusterContainer = stripClusterContainerHandle.get();
    ATH_MSG_DEBUG("Retrieved " << stripClusterContainer->size() << " input elements from key " << m_stripClusterContainerKey.key());

    ATH_MSG_DEBUG("Reading input condition data with key " << m_pixelDetEleCollKey.key());
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleCollHandle(m_pixelDetEleCollKey, ctx);
    ATH_CHECK(pixelDetEleCollHandle.isValid());
    const InDetDD::SiDetectorElementCollection *pixelDetEleColl = pixelDetEleCollHandle.retrieve();
    if (pixelDetEleColl == nullptr)
    {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved " << pixelDetEleColl->size() << " input condition elements from key " << m_pixelDetEleCollKey.key());

    ATH_MSG_DEBUG("Reading input condition data with key " << m_stripDetEleCollKey.key());
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleCollHandle(m_stripDetEleCollKey, ctx);
    ATH_CHECK(stripDetEleCollHandle.isValid());
    const InDetDD::SiDetectorElementCollection *stripDetEleColl = stripDetEleCollHandle.retrieve();
    if (stripDetEleColl == nullptr)
    {
      ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved " << stripDetEleColl->size() << " input condition elements from key " << m_stripDetEleCollKey.key());

    // convert all measurements to source links, so the CKF can find them from either strip or pixel seeds.

    auto measurements = m_trackFindingTool->initMeasurements(pixelClusterContainer->size() + stripClusterContainer->size());
    ATH_MSG_DEBUG("Create " << pixelClusterContainer->size() << " source links from pixel measurements");
    measurements->addMeasurements(ctx, pixelClusterContainer, pixelDetEleColl);
    ATH_MSG_DEBUG("Create " << stripClusterContainer->size() << " source links from strip measurements");
    measurements->addMeasurements(ctx, stripClusterContainer, stripDetEleColl);

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
    // Start with strips where the occupancy is lower (will become relevant when we add duplicate removal)
    if (!stripEstimatedTrackParameters->empty())
    {
      ATH_CHECK(m_trackFindingTool->findTracks(ctx,
                                               *measurements,
                                               *stripEstimatedTrackParameters,
                                               trackContainer,
                                               *trackCollection.get(),
                                               "strip"));
    }

    if (!pixelEstimatedTrackParameters->empty())
    {
      ATH_CHECK(m_trackFindingTool->findTracks(ctx,
                                               *measurements,
                                               *pixelEstimatedTrackParameters,
                                               trackContainer,
                                               *trackCollection.get(),
                                               "pixel"));
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
