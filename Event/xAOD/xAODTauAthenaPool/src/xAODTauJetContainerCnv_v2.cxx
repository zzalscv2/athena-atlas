/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <stdexcept>

// Gaudi/Athena include(s):
#include "GaudiKernel/MsgStream.h"

// Local include(s):
#include "xAODTauJetContainerCnv_v2.h"


xAODTauJetContainerCnv_v2::xAODTauJetContainerCnv_v2()
{
}

void xAODTauJetContainerCnv_v2::
persToTrans( const xAOD::TauJetContainer_v2* oldObj,
             xAOD::TauJetContainer* newObj,
             MsgStream& /*log*/ ) const {

   // Clear the transient object:
   newObj->clear();

   // Simply fill the transient object with as many new objects as many we have
   // in the persistent object:
   for( size_t i = 0; i < oldObj->size(); ++i ) {
      newObj->push_back( new xAOD::TauJet() );
   }

   return;
}

/// This function should never be called, as we are not supposed to convert
/// object before writing.
///
void xAODTauJetContainerCnv_v2::transToPers( const xAOD::TauJetContainer*,
                                             xAOD::TauJetContainer_v2*,
                                             MsgStream& log ) const {

   log << MSG::ERROR
       << "Somebody called xAODTauJetContainerCnv_v2::transToPers"
       << endmsg;
   throw std::runtime_error( "Somebody called xAODTauJetContainerCnv_v2::"
                             "transToPers" );

   return;
}
