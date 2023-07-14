/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/MdtDriftCircleAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"MDT_"};
}
#define PRD_AUXVARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);
namespace xAOD {
MdtDriftCircleAuxContainer_v1::MdtDriftCircleAuxContainer_v1()
    : AuxContainerBase() {
    /// Identifier variable hopefully unique
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);
  
    AUX_MEASUREMENTVAR(localPosition, 1)
    AUX_MEASUREMENTVAR(localCovariance, 1)
    
    /// Names may be shared across different subdetectors
    PRD_AUXVARIABLE(tdc);
    PRD_AUXVARIABLE(adc);
    PRD_AUXVARIABLE(driftTube);
    PRD_AUXVARIABLE(tubeLayer);
    PRD_AUXVARIABLE(status);
}
}  // namespace xAOD
#undef PRD_AUXVARIABLE