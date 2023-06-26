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
    ATH_CHECK(m_estimatedTrackParametersKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());

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
    ATH_MSG_DEBUG("Reading input collection with key " << m_estimatedTrackParametersKey.key());
    SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> estimatedTrackParametersHandle = SG::makeHandle(m_estimatedTrackParametersKey, ctx);
    ATH_CHECK(estimatedTrackParametersHandle.isValid());
    const ActsTrk::BoundTrackParametersContainer *estimatedTrackParameters = estimatedTrackParametersHandle.get();
    ATH_MSG_DEBUG("Retrieved " << estimatedTrackParameters->size() << " input elements from key " << m_estimatedTrackParametersKey.key());

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

    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //

    SG::WriteHandle<::TrackCollection> trackHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_tracksKey.key() << "` created ...");
    auto trackPtrs = std::make_unique<::TrackCollection>();

    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << estimatedTrackParameters->size() << " seeds.");

    ATH_CHECK(m_trackFindingTool->findTracks(ctx,
                                             {{pixelClusterContainer, pixelDetEleColl},
                                              {stripClusterContainer, stripDetEleColl}},
                                             *estimatedTrackParameters,
                                             *trackPtrs));
    ATH_MSG_DEBUG("    \\__ Created " << trackPtrs->size() << " tracks");

    // ================================================== //
    // ===================== STORE OUTPUT =============== //
    // ================================================== //

    ATH_MSG_DEBUG("Storing Track Collection " << m_tracksKey.key());
    ATH_CHECK(trackHandle.record(std::move(trackPtrs)));

    return StatusCode::SUCCESS;
  }

} // namespace
