/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDINGTOOL_TRACKFINDINGTOOL_H
#define ACTSTRKFINDINGTOOL_TRACKFINDINGTOOL_H 1

// BASE
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/ITrackFindingTool.h"

// ATHENA
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

// ACTS
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsTrkEvent/TrackContainer.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

// PACKAGE
#include "src/ITrackStatePrinter.h"

// Other
#include <memory>
#include <atomic>

namespace ActsTrk
{
  class TrackFindingTool : public extends<AthAlgTool, ActsTrk::ITrackFindingTool>
  {

  public:
    TrackFindingTool(const std::string &type, const std::string &name,
                     const IInterface *parent);
    virtual ~TrackFindingTool(); // define in .cxx so std::unique_ptr can delete incomplete type (pimpl)

    // standard Athena methods
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Interface
    virtual StatusCode
    findTracks(const EventContext &ctx,
               const Measurements &measurements,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               const ActsTrk::SeedContainer *seeds,
               ActsTrk::TrackContainer &tracksContainer, 
               ::TrackCollection &tracksCollection,
               size_t seedCollectionIndex,
               const char *seedType) const override;

    virtual std::unique_ptr<ActsTrk::ITrackFindingTool::Measurements> initMeasurements(size_t numMeasurements) const override;

  private:
    // Tools
    ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", "ActsExtrapolationTool"};
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
    ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool{this, "SummaryTool", "ToolHandle for track summary tool"};
    ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", "ActsToTrkConverterTool"};
    ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool{this, "BoundaryCheckTool", "InDet::InDetBoundaryCheckTool", "Boundary checking tool for detector sensitivities"};
    ToolHandle<Trk::IRIO_OnTrackCreator> m_RotCreatorTool{this, "RotCreatorTool", "", "optional RIO_OnTrack creator tool"};
    ToolHandle<ActsTrk::ITrackStatePrinter> m_trackStatePrinter{this, "TrackStatePrinter", "", "optional track state printer"};

    // Configuration
    Gaudi::Property<unsigned int> m_maxPropagationStep{this, "maxPropagationStep", 1000, "Maximum number of steps for one propagate call"};
    Gaudi::Property<bool> m_skipDuplicateSeeds{this, "skipDuplicateSeeds", true, "skip duplicate seeds before calling CKF"};
    // Selection cuts for associating measurements with predicted track parameters on a surface.
    Gaudi::Property<std::vector<double>> m_etaBins{this, "etaBins", {}, "MeasurementSelector: bins in |eta| to specify variable selections"};
    Gaudi::Property<std::vector<double>> m_chi2CutOff{this, "chi2CutOff", {std::numeric_limits<double>::max()}, "MeasurementSelector: maximum local chi2 contribution"};
    Gaudi::Property<std::vector<size_t>> m_numMeasurementsCutOff{this, "numMeasurementsCutOff", {1}, "MeasurementSelector: maximum number of associated measurements on a single surface"};

    // Create tracks from one seed's CKF result, appending to tracksContainer
    class DuplicateSeedDetector;
    size_t
    makeTracks(const EventContext &ctx,
               const Acts::GeometryContext &tgContext,
               const ActsTrk::TrackContainer &tracks,
               const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitOutput,
               ::TrackCollection &tracksContainer,
               DuplicateSeedDetector &duplicateSeedDetector) const;

    std::unique_ptr<const Trk::MeasurementBase>
    makeRIO_OnTrack(const xAOD::UncalibratedMeasurement &uncalibMeas,
                    const Trk::TrackParameters *parm) const;

    // Access Acts::CombinatorialKalmanFilter etc using "pointer to implementation"
    // so we don't have to instantiate the heavily templated classes in the header.
    struct CKF_pimpl;
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
