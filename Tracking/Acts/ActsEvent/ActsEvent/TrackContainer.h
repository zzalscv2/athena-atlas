/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINER_H
#define ACTSTRKEVENT_TRACKCONTAINER_H 1

#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

namespace ActsTrk {
using MutableTrackBackend = Acts::VectorTrackContainer;
using TrackBackend = Acts::ConstVectorTrackContainer;
using MutableTrackStateBackend = Acts::VectorMultiTrajectory;
using TrackStateBackend = Acts::ConstVectorMultiTrajectory;

class MutableTrackContainer
    : public Acts::TrackContainer<MutableTrackBackend, MutableTrackStateBackend,
                                  Acts::detail::ValueHolder> {
 public:
  MutableTrackContainer()
      : Acts::TrackContainer<MutableTrackBackend, MutableTrackStateBackend,
                             Acts::detail::ValueHolder>(
            MutableTrackBackend(), MutableTrackStateBackend()) {}
};

using TrackContainer = Acts::TrackContainer<TrackBackend, TrackStateBackend,
                                            Acts::detail::ValueHolder>;
}  // namespace ActsTrk
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::TrackContainer, 1210898253, 1)
#endif
