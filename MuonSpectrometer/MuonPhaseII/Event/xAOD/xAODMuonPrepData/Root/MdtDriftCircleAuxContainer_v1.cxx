/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/MdtDriftCircleAuxContainer_v1.h"

namespace xAOD {
MdtDriftCircleAuxContainer_v1::MdtDriftCircleAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(tdc);
    AUX_VARIABLE(adc);
    AUX_VARIABLE(driftTube);
    AUX_VARIABLE(tubeLayer);
    AUX_VARIABLE(driftCircleStatus);
}
}  // namespace xAOD