/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_SPACEPOINTAUXCONTAINER_H
#define XAODINDETMEASUREMENT_SPACEPOINTAUXCONTAINER_H

// Local include(s):
#include "xAODInDetMeasurement/versions/SpacePointAuxContainer_v1.h"

namespace xAOD {
   /// Definition of the current space point auxiliary container
   ///
   typedef SpacePointAuxContainer_v1 SpacePointAuxContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::SpacePointAuxContainer, 1273411139, 1 )

#endif


