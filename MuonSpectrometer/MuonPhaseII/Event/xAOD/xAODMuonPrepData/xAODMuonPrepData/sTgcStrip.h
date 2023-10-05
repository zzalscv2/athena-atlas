/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCSTRIP_H
#define XAODMUONPREPDATA_STGCSTRIP_H

#include "xAODMuonPrepData/versions/sTgcStrip_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the sTgcStrip
typedef sTgcStrip_v1 sTgcStrip;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::sTgcStrip, 32809900, 1)

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
