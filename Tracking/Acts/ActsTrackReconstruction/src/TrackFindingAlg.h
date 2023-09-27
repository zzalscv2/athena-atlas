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
#include <mutex>

// Handle Keys
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

class TrackingSurfaceHelper;
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

    SG::WriteHandleKey<ActsTrk::ConstTrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "SiSPSeededActsTrackContainer", "Output track collection (ActsTrk variant)"};

    // Configuration
    Gaudi::Property<unsigned int> m_maxPropagationStep{this, "maxPropagationStep", 1000, "Maximum number of steps for one propagate call"};
    Gaudi::Property<bool> m_skipDuplicateSeeds{this, "skipDuplicateSeeds", true, "skip duplicate seeds before calling CKF"};
    // Selection cuts for associating measurements with predicted track parameters on a surface.
    Gaudi::Property<std::vector<double>> m_etaBins{this, "etaBins", {}, "MeasurementSelector: bins in |eta| to specify variable selections"};
    Gaudi::Property<std::vector<double>> m_chi2CutOff{this, "chi2CutOff", {std::numeric_limits<double>::max()}, "MeasurementSelector: maximum local chi2 contribution"};
    Gaudi::Property<std::vector<size_t>> m_numMeasurementsCutOff{this, "numMeasurementsCutOff", {1}, "MeasurementSelector: maximum number of associated measurements on a single surface"};

    // Acts::TrackSelector cuts
    // Use max double, because mergeConfdb2.py doesn't like std::numeric_limits<double>::infinity() (produces bad Python "inf.0")
    Gaudi::Property<double> m_phiMin{this, "phiMin", -std::numeric_limits<double>::max(), "TrackSelector: phiMin"};
    Gaudi::Property<double> m_phiMax{this, "phiMax", std::numeric_limits<double>::max(), "TrackSelector: phiMax"};
    Gaudi::Property<double> m_etaMin{this, "etaMin", -std::numeric_limits<double>::max(), "TrackSelector: etaMin"};
    Gaudi::Property<double> m_etaMax{this, "etaMax", std::numeric_limits<double>::max(), "TrackSelector: etaMax"};
    Gaudi::Property<double> m_absEtaMin{this, "absEtaMin", 0.0, "TrackSelector: absEtaMin"};
    Gaudi::Property<double> m_absEtaMax{this, "absEtaMax", std::numeric_limits<double>::max(), "TrackSelector: absEtaMax"};
    Gaudi::Property<double> m_ptMin{this, "ptMin", 0.0, "TrackSelector: ptMin"};
    Gaudi::Property<double> m_ptMax{this, "ptMax", std::numeric_limits<double>::max(), "TrackSelector: ptMax"};
    Gaudi::Property<std::size_t> m_minMeasurements{this, "minMeasurements", 0, "TrackSelector: minMeasurements"};

    // configuration of statistics tables
    Gaudi::Property<std::vector<float> > m_statEtaBins
      {this,"StatisticEtaBins",{-4,-2.6,-2,0,2.,2.6,4},"Gather statistics separately for these bins."};
    Gaudi::Property<std::vector<std::string> > m_seedLables
      {this,"SeedLabels",{"Pixel","Strip"},"Empty or one label per seed key used in outputs"};
    Gaudi::Property<bool> m_dumpAllStatEtaBins
      {this,"DumpEtaBinsForAll",false,"Dump eta bins of all statistics counter."};

    enum EStat {
       kNTotalSeeds,
       kNoTrackParam,
       kNUsedSeeds,
       kNoTrack,
       kNDuplicateSeeds,
       kNOutputTracks,
       kNSelectedTracks,
       kNStat
    };

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
               const TrackingSurfaceHelper &tracking_surface_helper,
               DuplicateSeedDetector &duplicateSeedDetector,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               const ActsTrk::SeedContainer *seeds,
               ActsTrk::TrackContainer &tracksContainer,
               size_t seedCollectionIndex,
               const char *seedType,
               std::vector< std::array<unsigned int, kNStat> > &event_stat) const;

    // Create tracks from one seed's CKF result, appending to tracksContainer
    StatusCode storeSeedInfo(const ActsTrk::TrackContainer &tracksContainer,
                             const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                             DuplicateSeedDetector &duplicateSeedDetector) const;

    // Access Acts::CombinatorialKalmanFilter etc using "pointer to implementation"
    // so we don't have to instantiate the heavily templated classes in the header.
    // To maintain const-correctness, only use this via the accessor functions.
    struct CKF_pimpl;
    CKF_pimpl &trackFinder();
    const CKF_pimpl &trackFinder() const;

    std::unique_ptr<CKF_pimpl> m_trackFinder;

    // statistics
    enum ECategories {
       kPixelSeeds,
       kStripSeeds,
       kNCategories
    };

    static unsigned int nSeedCollections() {
       return kNCategories;
    }
    unsigned int seedCollectionStride( ) const {
       return m_statEtaBins.size()+1;
    }
    unsigned int getStatCategory( unsigned int seed_collection, float eta) const;

    std::size_t computeStatSum( unsigned int seed_collection,
                                EStat counter_i,
                                const std::vector< std::array<unsigned int, kNStat> > &stat) const;

    mutable std::mutex m_mutex ATLAS_THREAD_SAFE;
    mutable std::vector< std::array< std::size_t, kNStat > > m_stat ATLAS_THREAD_SAFE {};

    /// Private access to the logger
    const Acts::Logger &logger() const
    {
      return *m_logger;
    }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;

    bool m_useAbsEtaForStat=false;
  };

  inline unsigned int TrackFindingAlg::getStatCategory( unsigned int seed_collection, float eta) const {
     std::vector<float>::const_iterator bin_iter = std::upper_bound(m_statEtaBins.begin(),
                                                                    m_statEtaBins.end(),
                                                                    m_useAbsEtaForStat ? std::abs(eta) : eta);
     unsigned int category_i = seed_collection*seedCollectionStride()
                             + static_cast<unsigned int>(bin_iter - m_statEtaBins.begin());
     assert (category_i<m_stat.size());
     return category_i;
  }

  inline std::size_t TrackFindingAlg::computeStatSum( unsigned int seed_collection,
                                                      EStat counter_i,
                                                      const std::vector< std::array<unsigned int, kNStat> > &stat) const {
     std::size_t out=0u;
     for (unsigned int category_i =seed_collection * seedCollectionStride() + static_cast<unsigned int>(counter_i);
          category_i < (seed_collection+1) * seedCollectionStride();
          ++category_i) {
        assert( category_i<stat.size());
        out += stat[category_i][counter_i];
     }
     return out;
  }

} // namespace

#endif
