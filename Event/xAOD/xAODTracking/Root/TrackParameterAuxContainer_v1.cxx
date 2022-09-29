/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackParameterAuxContainer_v1.h"

namespace xAOD {
  TrackParameterAuxContainer_v1::TrackParameterAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(params);
    AUX_VARIABLE(covMatrix);
  }
}
