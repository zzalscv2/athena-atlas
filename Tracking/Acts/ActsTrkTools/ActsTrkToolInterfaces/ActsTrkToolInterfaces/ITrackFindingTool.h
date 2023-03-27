/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKTOOLINTERFACES_ITRACKFINDINGTOOL_H
#define ACTSTRKTOOLINTERFACES_ITRACKFINDINGTOOL_H 1

// Base
#include "GaudiKernel/IAlgTool.h"

// Athena
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/EventContext.h"
#include "TrkTrack/TrackCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

// ACTS EDM
#include "ActsTrkEvent/TrackParameters.h"

// OTHER
#include <vector>
#include <variant>

namespace ActsTrk
{
  using UncalibratedMeasurementContainerPtr = std::variant<const xAOD::PixelClusterContainer *, const xAOD::StripClusterContainer *>;

  class ITrackFindingTool
      : virtual public IAlgTool
  {
  public:
    DeclareInterfaceID(ITrackFindingTool, 1, 0);

    virtual StatusCode
    findTracks(const EventContext &ctx,
               const std::vector<std::pair<UncalibratedMeasurementContainerPtr, const InDetDD::SiDetectorElementCollection *>> &measurements,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               ::TrackCollection &tracksContainer) const = 0;
  };

} // namespace

#endif
