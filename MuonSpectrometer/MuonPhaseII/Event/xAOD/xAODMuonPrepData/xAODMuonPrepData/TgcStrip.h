/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_TGCSTRIP_H
#define XAODMUONPREPDATA_TGCSTRIP_H

#include "xAODMuonPrepData/versions/TgcStrip_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the TgcStrip
typedef TgcStrip_v1 TgcStrip;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::TgcStrip, 46677082, 1)

#endif  // XAODMUONPREPDATA_TGCSTRIP_H
