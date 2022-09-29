/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackJacobianAuxContainer_v1.h"

namespace xAOD {
  TrackJacobianAuxContainer_v1::TrackJacobianAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(jac);
  }
}
