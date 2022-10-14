/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ALFADataAuxContainer_v2.cxx 693858 2015-09-09 10:30:15Z
// krasznaa $

// Local include(s):
#include "xAODForward/versions/ZdcModuleAuxContainer_v2.h"

namespace xAOD {

   ZdcModuleAuxContainer_v2::ZdcModuleAuxContainer_v2()
      : AuxContainerBase() {

      // Digit collectoins
      AUX_VARIABLE( zdcId );
      AUX_VARIABLE( zdcSide );
      AUX_VARIABLE( zdcModule );
      AUX_VARIABLE( zdcType );
      AUX_VARIABLE( zdcChannel );


   }

} // namespace xAOD
