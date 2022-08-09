/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALO_GFEXTOWER_H
#define XAODTRIGL1CALO_GFEXTOWER_H

// Local include(s):
#include "xAODTrigL1Calo/versions/gFexTower_v1.h"
#include <map>

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Define the latest version of the TriggerTower class
   typedef gFexTower_v1 gFexTower;

}

#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::gFexTower , 49885072 , 1 )

#endif // XAODTRIGL1CALO_GFEXTOWER_H
