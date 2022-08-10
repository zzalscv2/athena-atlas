/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALO_GFEXTOWERCONTAINER_H
#define XAODTRIGL1CALO_GFEXTOWERCONTAINER_H

// Local include(s):
#include "xAODTrigL1Calo/gFexTower.h"
#include "xAODTrigL1Calo/versions/gFexTowerContainer_v1.h"

namespace xAOD {
   /// Define the latest version of the TriggerTower container
   typedef gFexTowerContainer_v1 gFexTowerContainer;
}

#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::gFexTowerContainer , 1149245544 , 1 )

#endif // XAODTRIGL1CALO_GFEXTOWERCONTAINER_H
