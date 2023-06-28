/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_TRACKFINDINGALG_H
#define ACTSTRKFINDING_TRACKFINDINGALG_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsToolInterfaces/ITrackFindingTool.h"

// ACTS
#include "ActsTrkEvent/Seed.h"
#include "ActsTrkEvent/TrackParameters.h"
#include "ActsTrkEvent/TrackContainer.h"
#include "ActsTrkEventCnv/IActsToTrkConverterTool.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

// Handle Keys
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace ActsTrk
{

  class TrackFindingAlg : public AthReentrantAlgorithm
  {
  public:
    using Measurement = xAOD::UncalibratedMeasurement;

    TrackFindingAlg(const std::string &name,
                    ISvcLocator *pSvcLocator);
    virtual ~TrackFindingAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    // Tool Handles
    ToolHandle<ActsTrk::ITrackFindingTool> m_trackFindingTool{this, "TrackFindingTool", "", "Track finding tool"};
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};

    // Handle Keys
    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClusterContainerKey{this, "PixelClusterContainerKey", "", "input pixel clusters"};
    SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClusterContainerKey{this, "StripClusterContainerKey", "", "input strip clusters"};
    /// To get detector elements condition data
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetectorElements", "", "input SiDetectorElementCollection for Pixel"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey{this, "StripDetectorElements", "", "input SiDetectorElementCollection for Strip"};

    SG::ReadHandleKey<ActsTrk::SeedContainer> m_pixelSeedsKey{this, "PixelSeeds", "", "Pixel Seeds"};
    SG::ReadHandleKey<ActsTrk::SeedContainer> m_stripSeedsKey{this, "StripSeeds", "", "Strip Seeds"};

    SG::ReadHandleKey<ActsTrk::BoundTrackParametersContainer> m_pixelEstimatedTrackParametersKey{this, "PixelEstimatedTrackParameters", "", "estimated track parameters from pixel seeding"};
    SG::ReadHandleKey<ActsTrk::BoundTrackParametersContainer> m_stripEstimatedTrackParametersKey{this, "StripEstimatedTrackParameters", "", "estimated track parameters from strip seeding"};

    SG::WriteHandleKey<::TrackCollection> m_tracksKey{this, "TracksLocation", "SiSPSeededActsTracks", "Output track collection"};
    SG::WriteHandleKey<ActsTrk::ConstTrackContainer> m_tracksContainerKey{this, "ACTSTracksLocation", "SiSPSeededActsTrackContainer", "Output track collection (ActsTrk variant)"};
  };

} // namespace

#endif
