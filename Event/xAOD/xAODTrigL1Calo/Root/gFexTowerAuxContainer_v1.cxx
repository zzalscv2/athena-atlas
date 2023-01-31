/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL1Calo/versions/gFexTowerAuxContainer_v1.h"

namespace xAOD {

  gFexTowerAuxContainer_v1::gFexTowerAuxContainer_v1() :
    AuxContainerBase()
    {
    
    AUX_VARIABLE( eta );
    AUX_VARIABLE( phi );
    AUX_VARIABLE( iEta );           
    AUX_VARIABLE( iPhi );
    AUX_VARIABLE( fpga );
    AUX_VARIABLE( towerEt );
    AUX_VARIABLE( isSaturated );
    AUX_VARIABLE( gFEXtowerID );
    }

} // namespace xAOD
