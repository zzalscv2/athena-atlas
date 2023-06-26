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
#include "ActsTrkEvent/TrackContainer.h"

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
                     const Acts::GeometryContext &tgContext,
                     const std::vector<ATLASUncalibSourceLink> &sourceLinks,
                     const std::vector<size_t> &ncoll) const = 0;

    virtual void
    printTracks(const Acts::GeometryContext &tgContext,
                const ActsTrk::TrackContainer &tracks,
                const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                const Acts::BoundTrackParameters &seed,
                size_t iseed,
                size_t ntracks) const = 0;
  };

} // namespace

#endif
