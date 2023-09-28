/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODMuonRDO/versions/NRPCRDOAuxContainer_v1.h"
namespace {
   static const std::string preFixStr{"MuonRdo_"};
}
#define RDO_AUXVARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);


namespace xAOD {
    NRPCRDOAuxContainer_v1::NRPCRDOAuxContainer_v1()
    : AuxContainerBase() {
         RDO_AUXVARIABLE(bcid);
         RDO_AUXVARIABLE(time);
         RDO_AUXVARIABLE(subdetector);
         RDO_AUXVARIABLE(tdcsector);
         RDO_AUXVARIABLE(tdc);
         RDO_AUXVARIABLE(channel);
         RDO_AUXVARIABLE(timeoverthr);
    }
}
#undef RDO_AUXVARIABLE
