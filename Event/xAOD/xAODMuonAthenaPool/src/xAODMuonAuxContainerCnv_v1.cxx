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
#include "xAODMuonAuxContainerCnv_v1.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/versions/MuonContainer_v1.h"


xAODMuonAuxContainerCnv_v1::xAODMuonAuxContainerCnv_v1()
{
}

void xAODMuonAuxContainerCnv_v1::
persToTrans( const xAOD::MuonAuxContainer_v1* oldObj,
             xAOD::MuonAuxContainer* newObj,
             MsgStream& /*log*/ ) const {

   // Clear the transient object:
   newObj->resize( 0 );

   // Copy the payload of the v1 object into the latest one by misusing
   // the thinning code a bit...
   SG::copyAuxStoreThinned( *oldObj, *newObj, nullptr );

   // Set up interface containers on top of them:

   //The old  uses v_
   // xAOD::MuonContainer_v1 oldInt;
   // for( size_t i = 0; i < oldObj->size(); ++i ) {
   //   oldInt.push_back( new xAOD::Muon_v1() );
   // }
   // oldInt.setStore( oldObj );
   //
   // xAOD::MuonContainer newInt;
   // for( size_t i = 0; i < newObj->size(); ++i ) {
   //   newInt.push_back( new xAOD::Muon() );
   // }
   // newInt.setStore( newObj );
   //
   // for( size_t i = 0; i < oldInt.size(); ++i ) {
   // }

   return;
}

/// This function should never be called, as we are not supposed to convert
/// object before writing.
///
void xAODMuonAuxContainerCnv_v1::
transToPers( const xAOD::MuonAuxContainer*,
             xAOD::MuonAuxContainer_v1*,
             MsgStream& log ) const {

   log << MSG::ERROR
       << "Somebody called xAODMuonAuxContainerCnv_v1::transToPers"
       << endmsg;
   throw std::runtime_error( "Somebody called xAODMuonAuxContainerCnv_v1::"
                             "transToPers" );

   return;
}
