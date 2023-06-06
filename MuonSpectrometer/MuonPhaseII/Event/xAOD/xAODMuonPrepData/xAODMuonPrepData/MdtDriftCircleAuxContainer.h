/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTDRIFTCIRCLEAUXCONTAINER_H
#define XAODMUONPREPDATA_MDTDRIFTCIRCLEAUXCONTAINER_H

#include "xAODMuonPrepData/versions/MdtDriftCircleAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MdtDriftCircle
typedef MdtDriftCircleAuxContainer_v1 MdtDriftCircleAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MdtDriftCircleAuxContainer, 1108747397, 1)
#endif  // XAODMUONRDO_NRPCRDO_H
