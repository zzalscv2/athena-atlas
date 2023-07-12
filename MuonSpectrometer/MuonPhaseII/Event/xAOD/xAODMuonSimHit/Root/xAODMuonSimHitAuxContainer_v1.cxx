/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonSimHit/versions/MuonSimHitAuxContainer_v1.h"

namespace xAOD {
MuonSimHitAuxContainer_v1::MuonSimHitAuxContainer_v1()
    : AuxContainerBase() {
    AUX_MEASUREMENTVAR(localPostion, 3);
    AUX_MEASUREMENTVAR(localDirection, 3);
    AUX_VARIABLE(stepLength);
    AUX_VARIABLE(globalTime);
    AUX_VARIABLE(pdgId);
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(energyDeposit);
    AUX_VARIABLE(kineticEnergy);
}
}  // namespace xAOD