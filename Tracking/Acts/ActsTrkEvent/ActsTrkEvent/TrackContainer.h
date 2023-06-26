/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINER_H
#define ACTSTRKEVENT_TRACKCONTAINER_H 1

#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

namespace ActsTrk
{
  using TrackBackend = Acts::VectorTrackContainer;
  using ConstTrackBackend = Acts::ConstVectorTrackContainer;
  using TrackStateBackend =  Acts::VectorMultiTrajectory;
  using ConstTrackStateBackend =  Acts::ConstVectorMultiTrajectory;
  using TrackContainer = Acts::TrackContainer<TrackBackend, TrackStateBackend, Acts::detail::ValueHolder>;
  using ConstTrackContainer = Acts::TrackContainer<ConstTrackBackend, ConstTrackStateBackend, Acts::detail::ValueHolder>;
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::ConstTrackContainer , 1080995907 , 1 )
#endif
