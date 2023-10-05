/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/sTgcStripAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"STGC_"};
}
#define PRD_AUXVARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);
namespace xAOD {
sTgcStripAuxContainer_v1::sTgcStripAuxContainer_v1()
    : AuxContainerBase() {
    /// Identifier variable hopefully unique
    AUX_VARIABLE(m_identifier);
    AUX_VARIABLE(m_identifierHash);
  
    AUX_MEASUREMENTVAR(m_localPosition, 1)
    AUX_MEASUREMENTVAR(m_localCovariance, 1)
    
    /// Names may be shared across different subdetectors
    PRD_AUXVARIABLE(m_bcBitMap);
    PRD_AUXVARIABLE(m_time);
    PRD_AUXVARIABLE(m_charge);
}
}  // namespace xAOD
#undef PRD_AUXVARIABLE