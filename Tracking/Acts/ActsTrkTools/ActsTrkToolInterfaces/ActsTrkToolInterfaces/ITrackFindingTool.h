/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKTOOLINTERFACES_ITRACKFINDINGTOOL_H
#define ACTSTRKTOOLINTERFACES_ITRACKFINDINGTOOL_H 1

// Athena
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "TrkTrack/TrackCollection.h"

// ACTS EDM
#include "ActsTrkEvent/TrackParameters.h"

// OTHER
#include <vector>
namespace ActsTrk
{
  class ITrackFindingTool
      : virtual public IAlgTool
  {
  public:
    DeclareInterfaceID(ITrackFindingTool, 1, 0);

    virtual StatusCode
    findTracks(const EventContext &ctx,
               const std::vector<ATLASUncalibSourceLink> &uncalibSourceLinks,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               ::TrackCollection &tracksContainer) const = 0;
  };

} // namespace

#endif
