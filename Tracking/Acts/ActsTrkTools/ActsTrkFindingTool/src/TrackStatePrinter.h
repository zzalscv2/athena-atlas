/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDINGTOOL_TRACKSTATEPRINTER_H
#define ACTSTRKFINDINGTOOL_TRACKSTATEPRINTER_H 1

// Base
#include "src/ITrackStatePrinter.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ATHENA
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODInDetMeasurement/SpacePoint.h"

// ACTS CORE
#include "Acts/Geometry/TrackingGeometry.hpp"

// PACKAGE
#include "ActsGeometry/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

// Other
#include <memory>
#include <tuple>

namespace ActsTrk
{
  class TrackStatePrinter : public extends<AthAlgTool, ActsTrk::ITrackStatePrinter>
  {
  public:
    TrackStatePrinter(const std::string &type, const std::string &name,
                      const IInterface *parent);
    virtual ~TrackStatePrinter() = default;

    // standard Athena methods
    virtual StatusCode initialize() override;

    void
    printSourceLinks(const EventContext &ctx,
                     const std::vector<ATLASUncalibSourceLink> &sourceLinks,
                     size_t type,
                     size_t offset) const override;

    void
    printTracks(const Acts::GeometryContext &tgContext,
                const ActsTrk::TrackContainer &tracks,
                const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                const Acts::BoundTrackParameters &seed,
                size_t iseed,
                size_t ntracks,
                size_t head,
                const char *seedType = "") const override;

    using MeasurementInfo = std::tuple<size_t,
                                       const ATLASUncalibSourceLink *,
                                       std::vector<const xAOD::SpacePoint *>>;

  private:
    // Handles
    SG::ReadHandleKeyArray<xAOD::SpacePointContainer> m_spacePointKey{this, "InputSpacePoints", {}, "Input Space Points for debugging"};

    // Tools
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

    // Configuration
    Gaudi::Property<std::vector<size_t>> m_spacePointType{this, "spacePointType", {}, "Type of each InputSpacePoints collection used for debugging"};

    void addSpacePoints(const EventContext &ctx, std::vector<MeasurementInfo> &measurementIndex, size_t type) const;
  };

} // namespace

#endif
