/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// ROOT include(s):
#include <TBranch.h>
#include <TTree.h>
#include <TError.h>

// EDM include(s):
#include "AthContainers/AuxTypeRegistry.h"

// Local include(s):
#include "xAODRootAccess/tools/TPrimitiveAuxBranchManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/Message.h"

namespace xAOD {

   TPrimitiveAuxBranchManager::TPrimitiveAuxBranchManager( auxid_t auxid,
                                                           ::TBranch* br,
                                                           THolder* holder )
      : m_branch( br ), m_holder( holder ), m_entry( -1 ),
        m_isSet( kTRUE ), m_auxId( auxid ), m_vector( 0 ) {
   }

   TPrimitiveAuxBranchManager::
   TPrimitiveAuxBranchManager( const TPrimitiveAuxBranchManager& parent )
      : TVirtualManager(), m_branch( parent.m_branch ), m_holder( 0 ),
        m_entry( parent.m_entry ), m_isSet( parent.m_isSet ),
        m_auxId( parent.m_auxId ), m_vector( 0 ) {

      if( parent.m_holder ) {
         m_holder = new THolder( *parent.m_holder );
      }
      if( parent.m_vector ) {
         m_vector = SG::AuxTypeRegistry::instance().makeVector( m_auxId, (size_t)0, (size_t)0 ).release();
         m_vector->resize( 1 );
      }
   }

   TPrimitiveAuxBranchManager::~TPrimitiveAuxBranchManager() {

      if( m_holder ) {
         delete m_holder;
      }
      if( m_vector ) {
         delete m_vector;
      }
   }

   TPrimitiveAuxBranchManager&
   TPrimitiveAuxBranchManager::
   operator=( const TPrimitiveAuxBranchManager& rhs ) {

      // Check if anything needs to be done:
      if( this == &rhs ) {
         return *this;
      }

      m_branch = rhs.m_branch;
      if( m_holder ) delete m_holder;
      if( rhs.m_holder ) {
         m_holder = new THolder( *rhs.m_holder );
      } else {
         m_holder = 0;
      }
      m_entry = rhs.m_entry;
      m_isSet = rhs.m_isSet;
      m_auxId = rhs.m_auxId;
      if( m_vector ) delete m_vector;
      if( rhs.m_vector ) {
         m_vector = SG::AuxTypeRegistry::instance().makeVector( m_auxId, (size_t)0, (size_t)0 ).release();
         m_vector->resize( 1 );
      } else {
         m_vector = 0;
      }

      // Return this same object:
      return *this;
   }

   ::TBranch* TPrimitiveAuxBranchManager::branch() {

      return m_branch;
   }

   ::TBranch** TPrimitiveAuxBranchManager::branchPtr() {

      return &m_branch;
   }

   const THolder* TPrimitiveAuxBranchManager::holder() const {

      return m_holder;
   }

   THolder* TPrimitiveAuxBranchManager::holder() {

      return m_holder;
   }

   ::Int_t TPrimitiveAuxBranchManager::getEntry( ::Int_t getall ) {

      // Make sure that the branch is associated to a tree 
      // as the entry to be read is retrieved from the tree
      if (!m_branch->GetTree()){
         Error("xAOD::TPrimitiveAuxBranchManager::getEntry",
           XAOD_MESSAGE("Branch=%s is not associated to any tree while reading of branches within this class relies on that"),
           m_branch->GetName());
         return -1;
      }

      // Get the entry that should be read 
      // The entry to be read is set with TTree::LoadTree()
      // NB: for a branch from a friend tree and if the friend tree has an index built,
      // then the entry to read is found when calling the TTree::LoadTree() function 
      // that matches the major and minor values between the main tree and the friend tree 
      ::Long64_t entry = m_branch->GetTree()->GetReadEntry();

      if ( entry  < 0 ){
         // Raise error as it implies 
         // either that the TTree::LoadTree() function has not been called 
         // or 
         // the entry requested to be read by the user 
         // is not corresponding to any entry for the friend tree 
         Error("xAOD::TPrimitiveAuxBranchManager::getEntry", 
            XAOD_MESSAGE( "Entry to read is not set for branch=%s from tree=%s. " 
            "It is either because TTree::LoadTree(entry) was not called "
            "beforehand in the TEvent class OR "
            "the entry requested to be read for the main tree is not corresponding to an event for the friend tree" ), 
            m_branch->GetName(),
            m_branch->GetTree()->GetName());
         return -1;
      }

      // Check if anything needs to be done:
      if( entry == m_entry ) return 0;

      // Load the entry.
      const ::Int_t nbytes = m_branch->GetEntry( entry, getall );

      // If the load was successful, remember that we loaded this entry.
      if( nbytes >= 0 ) {
         m_entry = entry;
      }

      return nbytes;
   }

   const void* TPrimitiveAuxBranchManager::object() const {

      return std::as_const(*m_holder).get();
   }

   void* TPrimitiveAuxBranchManager::object() {

      return m_holder->get();
   }

   void TPrimitiveAuxBranchManager::setObject( void* obj ) {

      // Make the holder forget about the previous variable, and get hold
      // of this new one:
      m_holder->setOwner( kFALSE );
      m_holder->set( obj );

      // Update the address of the branch:
      m_branch->SetAddress( m_holder->get() );

      // We are now "set":
      m_isSet = kTRUE;
      return;
   }

   ::Bool_t TPrimitiveAuxBranchManager::create() {

      // If we already have it set, let's stop here:
      if( m_isSet ) return kTRUE;

      // Otherwise let's create a default object:
      m_isSet = kTRUE;
      if( ! m_vector ) {
         m_vector = SG::AuxTypeRegistry::instance().makeVector( m_auxId, (size_t)0, (size_t)0 ).release();
         m_vector->resize( 1 );
      }
      // ...and use it to fill the current event:
      m_holder->setOwner( kFALSE );
      m_holder->set( m_vector->toPtr() );

      // And update the branch to use this address:
      m_branch->SetAddress( m_holder->get() );

      // We are now "set":
      return kTRUE;
   }

   ::Bool_t TPrimitiveAuxBranchManager::isSet() const {

     return m_isSet;
   }

   void TPrimitiveAuxBranchManager::reset() {

      m_isSet = kFALSE;
      return;
   }

} // namespace xAOD
