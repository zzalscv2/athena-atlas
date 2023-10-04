/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_DICT_H
#define XAODMUONPREPDATA_DICT_H

#include "xAODCore/tools/DictHelpers.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/versions/MdtDriftCircleAuxContainer_v1.h"
#include "xAODMuonPrepData/versions/MdtDriftCircleContainer_v1.h"
#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/RpcStripAuxContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonPrepData/versions/TgcStripAuxContainer_v1.h"
#include "xAODMuonPrepData/versions/TgcStripContainer_v1.h"
#include "xAODMuonPrepData/versions/TgcStrip_v1.h"
#include "xAODMuonPrepData/versions/MMClusterAuxContainer_v1.h"
#include "xAODMuonPrepData/versions/MMClusterContainer_v1.h"
#include "xAODMuonPrepData/versions/MMCluster_v1.h"
// Instantiate all necessary types for the dictionary.
namespace {
struct GCCXML_DUMMY_INSTANTIATION_XAODMUONRDO {
    // Type(s) needed for the dictionary generation to succeed.
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, MdtDriftCircleContainer_v1);
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, RpcStripContainer_v1);
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, TgcStripContainer_v1);
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, MMClusterContainer_v1);
};
}  // namespace

#endif  // XAODMUONRDO_DICT_H
