/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALO_VERSIONS_GFEXTOWERCONTAINER_V1_H
#define XAODTRIGL1CALO_VERSIONS_GFEXTOWERCONTAINER_V1_H

// EDM include(s):
#include "AthContainers/DataVector.h"

// Local include(s):
#include "xAODTrigL1Calo/versions/gFexTower_v1.h"

namespace xAOD {
   /// Define the TriggerTower as a simple DataVector
   typedef DataVector< xAOD::gFexTower_v1 > gFexTowerContainer_v1;
}

#endif // XAODTRIGL1CALO_VERSIONS_GFEXTOWERCONTAINER_V1_H
