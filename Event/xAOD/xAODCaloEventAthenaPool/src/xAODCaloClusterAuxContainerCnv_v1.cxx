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
#include "xAODCaloClusterAuxContainerCnv_v1.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/versions/CaloClusterContainer_v1.h"


xAODCaloClusterAuxContainerCnv_v1::xAODCaloClusterAuxContainerCnv_v1()
{
}

void xAODCaloClusterAuxContainerCnv_v1::
persToTrans( const xAOD::CaloClusterAuxContainer_v1* oldObj,
             xAOD::CaloClusterAuxContainer* newObj,
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
void xAODCaloClusterAuxContainerCnv_v1::
transToPers( const xAOD::CaloClusterAuxContainer*,
             xAOD::CaloClusterAuxContainer_v1*,
             MsgStream& log ) const {

   log << MSG::ERROR
       << "Somebody called xAODCaloClusterAuxContainerCnv_v1::transToPers"
       << endmsg;
   throw std::runtime_error( "Somebody called xAODCaloClusterAuxContainerCnv_v1::"
                             "transToPers" );

   return;
}
