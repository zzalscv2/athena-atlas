/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODMuonRDO/versions/NRPCRDOAuxContainer_v1.h"

namespace xAOD {
    NRPCRDOAuxContainer_v1::NRPCRDOAuxContainer_v1()
    : AuxContainerBase() {
         AUX_VARIABLE(bcid);
         AUX_VARIABLE(time);
         AUX_VARIABLE(subdetector);
         AUX_VARIABLE(tdcsector);
         AUX_VARIABLE(tdc);
         AUX_VARIABLE(channel);
         AUX_VARIABLE(timeoverthr);
    }
}
