/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_TGCSTRIPAUXCONTAINER_H
#define XAODMUONPREPDATA_TGCSTRIPAUXCONTAINER_H

#include "xAODMuonPrepData/versions/TgcStripAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the TgcStrip
typedef TgcStripAuxContainer_v1 TgcStripAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::TgcStripAuxContainer , 1242132729 , 1 )
#endif  // XAODMUONRDO_NTGCRDO_H
