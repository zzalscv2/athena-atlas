/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: IParticleHelpers.cxx 618909 2014-09-29 10:16:52Z krasznaa $

// System include(s):
#include <iostream>

// EDM include(s):
#include "AthLinks/ElementLink.h"

// Local include(s):
#include "xAODBase/IParticleHelpers.h"

namespace xAOD {

   /// Object used for setting/getting the dynamic decoration in question
   static SG::AuxElement::Accessor< ElementLink< IParticleContainer > >
      acc( "originalObjectLink" );

   /// This function should be used by CP tools when they make a deep copy
   /// of an object in their correctedCopy(...) function. So that the deep
   /// copy would hold an ElementLink pointing back to its original, which
   /// the MET calculations can use later on.
   ///
   /// @param original Reference to the original object
   /// @param copy Reference to the (deep/shallow) copy of the original object
   /// @returns <code>true</code> if the link setting was successful,
   ///          <code>false</code> if it wasn't
   ///
   bool setOriginalObjectLink( const IParticle& original,
                               IParticle& copy ) {

      // We mustn't check for the availability of the decoration on the copied
      // object here. Since if the copy is already part of a container, the
      // decoration may well exist on it already. But it will not be set to
      // anything meaningful yet.

      // Check if the original is part of a container. If not, we can't set
      // up a link.
      const IParticleContainer* container =
         dynamic_cast< const IParticleContainer* >( original.container() );
      if( ! container ) {
         std::cerr << "xAOD::setOriginalObjectLink   ERROR Original object is "
                   << "not part of a container" << std::endl;
         return false;
      }

      // Construct the ElementLink that points back to the original.
      // We have to be tricky here. The original itself may already be a
      // copy. In which case the new object should point back to the same
      // original that "our original" is pointing to.
      const ElementLink< IParticleContainer > link =
         ( acc.isAvailable( original ) ?
           acc( original ) :
           ElementLink< IParticleContainer >( *container, original.index() ) );

      // Now set this link on the copy:
      acc( copy ) = link;

      // We were successful:
      return true;
   }

   /// This function should be used by the users when they make deep/shallow
   /// copies of an entire container. It sets up links from all the copied
   /// object to their originals. So in later stages of the analysis one can
   /// navigate back to them. (This is mostly necessary for proper MET
   /// handling.)
   ///
   /// @param original Reference to the original container
   /// @param copy Reference to the (deep/shallow) copy of the original
   ///             container
   /// @returns <code>true</code> if the link setting was successful,
   ///          <code>false</code> if it wasn't
   ///
   bool setOriginalObjectLink( const IParticleContainer& original,
                               IParticleContainer& copy ) {

      // Check that the containers are of the same size:
      if( original.size() != copy.size() ) {
         std::cerr << "xAOD::setOriginalObjectLink   ERROR Size of original "
                   << "and copy containers differs" << std::endl;
         return false;
      }

      // If the containers are empty, we're done:
      if( ! copy.size() ) {
         return true;
      }

      // Loop over the copied container:
      IParticleContainer::const_iterator orig_itr = original.begin();
      IParticleContainer::const_iterator orig_end = original.end();
      IParticleContainer::iterator copy_itr = copy.begin();
      IParticleContainer::iterator copy_end = copy.end();
      for( ; orig_itr != orig_end; ++orig_itr, ++copy_itr ) {
         // Construct the ElementLink with the same logic as in the previous
         // function:
         const ElementLink< IParticleContainer > link =
            ( acc.isAvailable( **orig_itr ) ?
              acc( **orig_itr ) :
              ElementLink< IParticleContainer >( original,
                                                 ( *orig_itr )->index() ) );
         // Set the link on the object:
         acc( **copy_itr ) = link;
      }

      // We were successful:
      return true;
   }

   /// This function can be used to conveniently get a pointer back to the
   /// original object from which a copy was created. If there is no such
   /// parent, the function silently returns a null pointer.
   ///
   /// @param copy The object that should have a parent set on it
   /// @returns A pointer to the objects parent if it exists and its available,
   ///          a null pointer otherwise
   ///
   const IParticle* getOriginalObject( const IParticle& copy ) {

      // Check if the decoration is available on the object:
      if( ! acc.isAvailable( copy ) ) {
         return 0;
      }

      // Get the link:
      const ElementLink< IParticleContainer >& link = acc( copy );

      // Check if the link is valid:
      if( ! link.isValid() ) {
         return 0;
      }

      // Apparently all is fine:
      return *link;
   }

} // namespace xAOD
