/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: xAODPhotonContainerCnv.cxx 596436 2014-05-11 17:34:00Z krasznaa $

// System include(s):
#include <exception>

// Local include(s):
#include "xAODPhotonContainerCnv.h"


namespace { 
  /// Helper function setting up the container's link to its auxiliary store 
  void setStoreLink( SG::AuxVectorBase* cont, const std::string& key ) { 
    
    // The link to set up: 
    DataLink< SG::IConstAuxStore > link( key + "Aux." ); 
    
    // Give it to the container: 
    cont->setStore( link ); 
    
    return; 
  } 
  
} // private namespace 

xAODPhotonContainerCnv::xAODPhotonContainerCnv( ISvcLocator* svcLoc )
   : xAODPhotonContainerCnvBase( svcLoc ) {

}

/** 
 * This function needs to be re-implemented in order to figure out the StoreGate 
 * key of the container that's being created. After that's done, it lets the 
 * base class do its normal task. 
 */

StatusCode xAODPhotonContainerCnv::createObj( IOpaqueAddress* pAddr, 
					   DataObject*& pObj ) { 
  
  // Get the key of the container that we'll be creating: 
  m_key = *( pAddr->par() + 1 ); 
  ATH_MSG_VERBOSE( "Key of xAOD::PhotonContainer: " << m_key ); 
  
  // Let the base class do its thing now: 
  return AthenaPoolConverter::createObj( pAddr, pObj ); 
} 


xAOD::PhotonContainer*
xAODPhotonContainerCnv::createPersistent( xAOD::PhotonContainer* trans ) {

   // Create a view copy of the container:
   xAOD::PhotonContainer* result =
      new xAOD::PhotonContainer( trans->begin(), trans->end(),
                              SG::VIEW_ELEMENTS );

   // Prepare the objects to be written out:
   xAOD::PhotonContainer::iterator itr = result->begin();
   xAOD::PhotonContainer::iterator end = result->end();
   for( ; itr != end; ++itr ) {
      toPersistent( *itr );
   }

   // Return the new container:
   return result;
}

xAOD::PhotonContainer* xAODPhotonContainerCnv::createTransient() {

   // The known ID(s) for this container:
   static pool::Guid v1_guid( "5F045AAE-DBD8-47E4-90AC-9162530A9565" );

   // Check if we're reading the most up to date type:
   if( compareClassGuid( v1_guid ) ) {
     xAOD::PhotonContainer* c = poolReadObject< xAOD::PhotonContainer >(); 
     setStoreLink( c, m_key ); 
     return c; 
   }

   // If we didn't recognise the ID, let's complain:
   throw std::runtime_error( "Unsupported version of "
                             "xAOD::PhotonContainer found" );
   return 0;
}

void xAODPhotonContainerCnv::toPersistent( xAOD::Photon* photon ) const {


  size_t nClusters= photon->nCaloClusters();
  for (size_t i=0; i<nClusters;++i){
    ElementLink<xAOD::CaloClusterContainer>& link = const_cast<ElementLink<xAOD::CaloClusterContainer>& > (photon->caloClusterLink(i));
    link.toPersistent();
  }

  size_t nVertexes= photon->nVertices();
  for (size_t i=0; i<nVertexes;++i){
    ElementLink<xAOD::VertexContainer>& link = const_cast<ElementLink<xAOD::VertexContainer>& > (photon->vertexLink(i));
    link.toPersistent();
  }

   return;
}
