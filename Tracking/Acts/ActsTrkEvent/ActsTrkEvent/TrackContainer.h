/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINER_H
#define ACTSTRKEVENT_TRACKCONTAINER_H 1

#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

namespace ActsTrk
{
  using TrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail::ValueHolder>;
  using ConstTrackContainer = Acts::TrackContainer<Acts::ConstVectorTrackContainer, Acts::ConstVectorMultiTrajectory, Acts::detail::ValueHolder>;
}

#endif
