/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackMeasurementsAuxContainer_v1.h"

namespace xAOD {
  TrackMeasurementsAuxContainer_v1::TrackMeasurementsAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(measurements);
    AUX_VARIABLE(covariance);

  }
}
