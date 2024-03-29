/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AmbiguityResolutionAlg.h"

// Athena
#include "AthenaMonitoringKernel/Monitored.h"

// ACTS
#include "Acts/Definitions/Units.hpp"

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Utilities/Logger.hpp"

#include "ActsInterop/Logger.h"
#include "ActsGeometry/ATLASSourceLink.h"

namespace {
   std::size_t sourceLinkHash(const Acts::SourceLink& slink) {
      const ActsTrk::ATLASUncalibSourceLink &atlasSourceLink = slink.get<ActsTrk::ATLASUncalibSourceLink>();
      return (*atlasSourceLink)->identifierHash();
   }

   bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
      return    (*a.get<ActsTrk::ATLASUncalibSourceLink>())->identifierHash()
             == (*b.get<ActsTrk::ATLASUncalibSourceLink>())->identifierHash();
   }
}

namespace ActsTrk
{

  AmbiguityResolutionAlg::AmbiguityResolutionAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode AmbiguityResolutionAlg::initialize()
  {
     {
        Acts::GreedyAmbiguityResolution::Config cfg;
        cfg.maximumSharedHits = m_maximumSharedHits;
        cfg.maximumIterations = m_maximumIterations;
        cfg.nMeasurementsMin = m_nMeasurementsMin;
        m_ambi = std::make_unique<Acts::GreedyAmbiguityResolution>(std::move(cfg), makeActsAthenaLogger(this, "Acts")  ) ;
        assert( m_ambi );
     }

     ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
     ATH_CHECK(m_resolvedTracksBackendHandle.initialize());
     ATH_CHECK(m_tracksKey.initialize());
     ATH_CHECK(m_resolvedTracksKey.initialize());
     return StatusCode::SUCCESS;
  }

  StatusCode AmbiguityResolutionAlg::execute(const EventContext &ctx) const
  {
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto mon = Monitored::Group( m_monTool, timer );

    SG::ReadHandle<ActsTrk::TrackContainer> trackHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(trackHandle.isValid());

    Acts::GreedyAmbiguityResolution::State state;
    m_ambi->computeInitialState(*trackHandle, state, &sourceLinkHash,
                                &sourceLinkEquality);

    m_ambi->resolve(state);

    ATH_MSG_DEBUG("Resolved to " << state.selectedTracks.size() << " tracks from "
                  << trackHandle->size());

    ActsTrk::MutableTrackContainer solvedTracks;
    solvedTracks.ensureDynamicColumns(*trackHandle);

    for (auto iTrack : state.selectedTracks) {
       auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
       destProxy.copyFrom(trackHandle->getTrack(state.trackTips.at(iTrack)));
    }
    std::unique_ptr<ActsTrk::TrackContainer> outputTracks = m_resolvedTracksBackendHandle.moveToConst( std::move(solvedTracks), ctx);
    SG::WriteHandle<ActsTrk::TrackContainer> resolvedTrackHandle(m_resolvedTracksKey, ctx);

    if (resolvedTrackHandle.record( std::move(outputTracks)).isFailure()) {
       ATH_MSG_ERROR("Failed to record resolved ACTS tracks with key " << m_resolvedTracksKey.key() );
       return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

} // namespace
