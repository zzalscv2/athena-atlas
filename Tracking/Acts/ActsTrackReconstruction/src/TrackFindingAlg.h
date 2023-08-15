/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGALG_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "src/ITrackStatePrinter.h"

// ACTS
#include "ActsEvent/Seed.h"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometry/ATLASSourceLink.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/EventContext.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

// Other
#include <limits>
#include <string>
#include <vector>
#include <memory>
#include <atomic>

// Handle Keys
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace
{
  // Forward-declare internal classes defined in TrackFindingData.h and used only in TrackFindingAlg.cxx.
  // Define in the anonymous namespace to prevent unneccessary external linkage.
  class TrackFindingMeasurements;
  class DuplicateSeedDetector;
}

namespace ActsTrk
{

  class TrackFindingAlg : public AthReentrantAlgorithm
  {
  public:
    TrackFindingAlg(const std::string &name,
                    ISvcLocator *pSvcLocator);
    virtual ~TrackFindingAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    // Tool Handles
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
    ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", "ActsExtrapolationTool"};
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
    ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", "ActsToTrkConverterTool"};
    ToolHandle<ActsTrk::ITrackStatePrinter> m_trackStatePrinter{this, "TrackStatePrinter", "", "optional track state printer"};

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

    SG::WriteHandleKey<std::vector<ATLASUncalibSourceLink::ElementsType>> m_sourceLinksOutKey{this, "ATLASUncalibSourceLinkElementsName", "" /*"UncalibratedMeasurementSourceLinkElements"*/, ""};
    SG::WriteHandleKey<ActsTrk::ConstTrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "SiSPSeededActsTrackContainer", "Output track collection (ActsTrk variant)"};

    // Configuration
    Gaudi::Property<unsigned int> m_maxPropagationStep{this, "maxPropagationStep", 1000, "Maximum number of steps for one propagate call"};
    Gaudi::Property<bool> m_skipDuplicateSeeds{this, "skipDuplicateSeeds", true, "skip duplicate seeds before calling CKF"};
    // Selection cuts for associating measurements with predicted track parameters on a surface.
    Gaudi::Property<std::vector<double>> m_etaBins{this, "etaBins", {}, "MeasurementSelector: bins in |eta| to specify variable selections"};
    Gaudi::Property<std::vector<double>> m_chi2CutOff{this, "chi2CutOff", {std::numeric_limits<double>::max()}, "MeasurementSelector: maximum local chi2 contribution"};
    Gaudi::Property<std::vector<size_t>> m_numMeasurementsCutOff{this, "numMeasurementsCutOff", {1}, "MeasurementSelector: maximum number of associated measurements on a single surface"};

    /**
     * @brief invoke track finding procedure
     *
     * @param ctx - event context
     * @param measurements - measurements container
     * @param estimatedTrackParameters - estimates
     * @param seeds - spacepoint triplet seeds
     * @param tracksContainer - output tracks
     * @param tracksCollection - auxiliary output for downstream tools compatibility (to be removed in the future)
     * @param seedCollectionIndex - index of seeds in measurements
     * @param seedType name of type of seeds (strip or pixel) - only used for messages
     */
    StatusCode
    findTracks(const EventContext &ctx,
               const TrackFindingMeasurements &measurements,
               DuplicateSeedDetector &duplicateSeedDetector,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               const ActsTrk::SeedContainer *seeds,
               ActsTrk::TrackContainer &tracksContainer,
               size_t seedCollectionIndex,
               const char *seedType) const;

    // Create tracks from one seed's CKF result, appending to tracksContainer
    StatusCode storeSeedInfo(const ActsTrk::TrackContainer &tracksContainer,
                             const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                             DuplicateSeedDetector &duplicateSeedDetector) const;

    // Access Acts::CombinatorialKalmanFilter etc using "pointer to implementation"
    // so we don't have to instantiate the heavily templated classes in the header.
    // To maintain const-correctness, only use this via the accessor functions.
    struct CKF_pimpl;
    CKF_pimpl& trackFinder();
    const CKF_pimpl& trackFinder() const;

    std::unique_ptr<CKF_pimpl> m_trackFinder;

    // statistics
    mutable std::atomic<size_t> m_nTotalSeeds{0};
    mutable std::atomic<size_t> m_nFailedSeeds{0};
    mutable std::atomic<size_t> m_nDuplicateSeeds{0};
    mutable std::atomic<size_t> m_nOutputTracks{0};

    /// Private access to the logger
    const Acts::Logger &logger() const
    {
      return *m_logger;
    }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;
  };

} // namespace

#endif
