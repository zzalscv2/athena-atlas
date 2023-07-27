/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHIT_DICT_H
#define XAODMUONSIMHIT_DICT_H

#include <xAODCore/tools/DictHelpers.h>
#include <xAODMuonSimHit/MuonSimHit.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <xAODMuonSimHit/versions/MuonSimHitContainer_v1.h>
#include <xAODMuonSimHit/versions/MuonSimHitAuxContainer_v1.h>
#include <xAODMuonSimHit/versions/MuonSimHit_v1.h>

// Instantiate all necessary types for the dictionary.
namespace {
struct GCCXML_DUMMY_INSTANTIATION_XAODMUONRDO {
    // Type(s) needed for the dictionary generation to succeed.
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, MuonSimHitContainer_v1);
};
}  // namespace

#endif  // XAODMUONRDO_DICT_H
