/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ITRACKFINDINGTOOL_H
#define ACTSTOOLINTERFACES_ITRACKFINDINGTOOL_H

// Base
#include "GaudiKernel/IAlgTool.h"

// Athena
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/EventContext.h"
#include "ActsEvent/TrackContainer.h"
#include "TrkTrack/TrackCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

// ACTS EDM
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/Seed.h"

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
      virtual void addMeasurements(size_t type, const EventContext &ctx, const UncalibratedMeasurementContainerPtr &clusterContainer, const InDetDD::SiDetectorElementCollection *detElems) = 0;
    };

    DeclareInterfaceID(ITrackFindingTool, 1, 0);

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
    virtual StatusCode
    findTracks(const EventContext &ctx,
               const Measurements &measurements,
               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
               const ActsTrk::SeedContainer *seeds,
               ActsTrk::TrackContainer &tracksContainer,
               ::TrackCollection &tracksCollection,
               size_t seedCollectionIndex,
               const char *seedType) const = 0;

    virtual std::unique_ptr<Measurements> initMeasurements(size_t numMeasurements) const = 0;
  };

} // namespace

#endif
