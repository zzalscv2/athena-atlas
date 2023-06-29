/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDINGTOOL_ITRACKSTATEPRINTER_H
#define ACTSTRKFINDINGTOOL_ITRACKSTATEPRINTER_H 1

// Base
#include "GaudiKernel/IAlgTool.h"

// ATHENA
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/EventContext.h"

// ACTS CORE
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/EventData/TrackParameters.hpp"

// PACKAGE
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsEvent/Seed.h"
#include "ActsEvent/TrackContainer.h"

// Other
#include <vector>

namespace ActsTrk
{
  class ITrackStatePrinter
      : virtual public IAlgTool
  {
  public:
    DeclareInterfaceID(ITrackStatePrinter, 1, 0);

    virtual void
    printSourceLinks(const EventContext &ctx,
                     const std::vector<ATLASUncalibSourceLink> &sourceLinks,
                     size_t type,
                     size_t offset) const = 0;

    virtual void
    printSeed(const Acts::GeometryContext &tgContext,
              const ActsTrk::Seed &seed,
              const Acts::BoundTrackParameters &initialParameters,
              size_t measurementOffset,
              size_t iseed,
              size_t head,
              const char *seedType) const = 0;

    virtual void
    printTracks(const Acts::GeometryContext &tgContext,
                const ActsTrk::TrackContainer &tracks,
                const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                const std::vector<ATLASUncalibSourceLink> &measurements,
                size_t measurementOffset) const = 0;
  };

} // namespace

#endif
