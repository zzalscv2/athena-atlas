/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackSurfaceAuxContainer_v1.h"

namespace xAOD {
  TrackSurfaceAuxContainer_v1::TrackSurfaceAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(translation);
    AUX_VARIABLE(rotation);
    AUX_VARIABLE(boundValues);
    AUX_VARIABLE(SurfaceType); 
  }
}
