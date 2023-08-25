/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/versions/SpacePointAuxContainer_v1.h"

namespace xAOD {
  SpacePointAuxContainer_v1::SpacePointAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(elementIdList);
    AUX_VARIABLE(globalPosition);
    AUX_VARIABLE(radius);
    AUX_VARIABLE(varianceR);
    AUX_VARIABLE(varianceZ);
    AUX_VARIABLE(measurements);
  }
}

