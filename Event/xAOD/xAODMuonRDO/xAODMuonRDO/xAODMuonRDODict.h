/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_DICT_H
#define XAODMUONRDO_DICT_H

#include "xAODMuonRDO/NRPCRDO.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "xAODMuonRDO/versions/NRPCRDO_v1.h"
#include "xAODMuonRDO/versions/NRPCRDOContainer_v1.h"
#include "xAODMuonRDO/versions/NRPCRDOAuxContainer_v1.h"

#include "xAODMuonRDO/versions/NSWTPRDOContainer_v1.h"
#include "xAODMuonRDO/versions/NSWTPRDOAuxContainer_v1.h"

#include "xAODCore/tools/DictHelpers.h"

// Instantiate all necessary types for the dictionary.
namespace {
    struct GCCXML_DUMMY_INSTANTIATION_XAODMUONRDO {
        // Type(s) needed for the dictionary generation to succeed.
        XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, NRPCRDOContainer_v1 );
        XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, NSWTPRDOContainer_v1 );
    };
}


#endif // XAODMUONRDO_DICT_H

