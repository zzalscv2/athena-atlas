/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: HIEventShapeAuxContainer_v1.cxx 594315 2014-04-25 17:34:40Z krasznaa $

// Local include(s):
#include "xAODHIEvent/versions/HIEventShapeAuxContainer_v1.h"

namespace xAOD {

   HIEventShapeAuxContainer_v1::HIEventShapeAuxContainer_v1()
      : AuxContainerBase() {

      // Basic event information:

     AUX_VARIABLE( Et );
     AUX_VARIABLE( area );
     AUX_VARIABLE( rho );
     AUX_VARIABLE( nCells );
     AUX_VARIABLE( Et_cos ); 
     AUX_VARIABLE( Et_sin ); 
     AUX_VARIABLE( etaMin );
     AUX_VARIABLE( etaMax );
     AUX_VARIABLE( layer );


   }

} // namespace xAOD
