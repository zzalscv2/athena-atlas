/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCSTRIPAUXCONTAINER_H
#define XAODMUONPREPDATA_STGCSTRIPAUXCONTAINER_H

#include "xAODMuonPrepData/versions/sTgcStripAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the sTgcStrip
typedef sTgcStripAuxContainer_v1 sTgcStripAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcStripAuxContainer , 1320356011 , 1 )
#endif  // XAODMUONPREPDATA_STGCSTRIPAUXCONTAINER_H
