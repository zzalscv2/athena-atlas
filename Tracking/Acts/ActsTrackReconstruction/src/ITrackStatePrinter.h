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
namespace InDetDD {
   class SiDetectorElementCollection;
}
namespace ActsTrk
{
  class ITrackStatePrinter
      : virtual public IAlgTool
  {
  public:
    DeclareInterfaceID(ITrackStatePrinter, 1, 0);

    virtual void
    printMeasurements(const EventContext &ctx,
                      const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                      const InDetDD::SiDetectorElementCollection *detectorElements,
                      size_t typeIndex,
                      size_t offset) const = 0;

    virtual void
    printSeed(const Acts::GeometryContext &tgContext,
              const ActsTrk::Seed &seed,
              const Acts::BoundTrackParameters &initialParameters,
              size_t measurementOffset,
              size_t iseed) const = 0;

    virtual void
    printTracks(const Acts::GeometryContext &tgContext,
                const ActsTrk::TrackContainer &tracks,
                const std::vector<ActsTrk::TrackContainer::TrackProxy> &fitResult,
                const std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t> > &offset) const = 0;
  };

} // namespace

#endif
