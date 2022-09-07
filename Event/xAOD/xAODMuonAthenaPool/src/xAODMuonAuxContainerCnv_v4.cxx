/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <stdexcept>

// Gaudi/Athena include(s):
#include "GaudiKernel/MsgStream.h"

// Core EDM include(s):
#include "AthContainers/tools/copyAuxStoreThinned.h"

// Local include(s):
#include "xAODMuonAuxContainerCnv_v4.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/versions/MuonContainer_v1.h"


xAODMuonAuxContainerCnv_v4::xAODMuonAuxContainerCnv_v4()
{
}

void xAODMuonAuxContainerCnv_v4::
persToTrans( const xAOD::MuonAuxContainer_v4* oldObj,
             xAOD::MuonAuxContainer* newObj,
             MsgStream& /*log*/ ) const {

   // Clear the transient object:
   newObj->resize( 0 );

   // Copy the payload of the v1 object into the latest one by misusing
   // the thinning code a bit...
   SG::copyAuxStoreThinned( *oldObj, *newObj, nullptr );

   return;
}

/// This function should never be called, as we are not supposed to convert
/// object before writing.
///
void xAODMuonAuxContainerCnv_v4::
transToPers( const xAOD::MuonAuxContainer*,
             xAOD::MuonAuxContainer_v4*,
             MsgStream& log ) const {

   log << MSG::ERROR
       << "Somebody called xAODMuonAuxContainerCnv_v4::transToPers"
       << endmsg;
   throw std::runtime_error( "Somebody called xAODMuonAuxContainerCnv_v4::"
                             "transToPers" );

   return;
}
