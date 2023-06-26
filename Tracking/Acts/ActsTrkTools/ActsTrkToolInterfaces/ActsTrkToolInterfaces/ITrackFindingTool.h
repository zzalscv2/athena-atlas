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
#include "ActsTrkEvent/TrackContainer.h"
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
    // Class to hold the measurement source links. This is created by TrackFindingTool::initMeasurements().
    struct Measurements
    {
      virtual ~Measurements() = default;
      virtual void addMeasurements(const EventContext &ctx, const UncalibratedMeasurementContainerPtr &clusterContainer, const InDetDD::SiDetectorElementCollection *detElems) = 0;
    };

    DeclareInterfaceID(ITrackFindingTool, 1, 0);


    /**
     * @brief invoke track finding procedure
     * 
     * @param ctx - event context
     * @param measurements - measurements container
     * @param estimatedTrackParameters - estimates
     * @param tracksContainer - output tracks
     * @param tracksCollection - auxiliary output for downstream tools compatibility (to be removed in the future)
     * @param hint on which tracks (strip or pixel) are to be looked for
     */
    virtual StatusCode
    findTracks(const EventContext &ctx,
               const Measurements &measurements,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               ActsTrk::TrackContainer &tracksContainer,
               ::TrackCollection & tracksCollection,
               const char *seedType = "") const = 0;

    virtual std::unique_ptr<Measurements> initMeasurements(size_t numMeasurements) const = 0;
  };

} // namespace

#endif
