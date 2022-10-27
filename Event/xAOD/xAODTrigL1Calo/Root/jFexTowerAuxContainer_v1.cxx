/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL1Calo/versions/jFexTowerAuxContainer_v1.h"

namespace xAOD {

jFexTowerAuxContainer_v1::jFexTowerAuxContainer_v1() :
    AuxContainerBase()
{

    AUX_VARIABLE( eta         );
    AUX_VARIABLE( phi         );
    AUX_VARIABLE( globalEta   );
    AUX_VARIABLE( globalPhi   );
    AUX_VARIABLE( module      );
    AUX_VARIABLE( fpga        );
    AUX_VARIABLE( channel     );
    AUX_VARIABLE( jFEXdataID  );
    AUX_VARIABLE( isjTowerSat );
    AUX_VARIABLE( jFEXtowerID );
    AUX_VARIABLE( Calosource  );
    AUX_VARIABLE( et_count    );
}

} // namespace xAOD
