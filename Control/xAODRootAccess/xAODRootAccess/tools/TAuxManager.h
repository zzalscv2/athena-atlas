// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_TAUXMANAGER_H
#define XAODROOTACCESS_TOOLS_TAUXMANAGER_H

// System include(s):
#include <memory>

// Local include(s):
#include "TVirtualManager.h"

// Forward declaration(s):
namespace SG {
   class IConstAuxStore;
}

namespace xAOD {

   // Forward declaration(s):
   class TAuxStore;

   /// @short Manager for TAuxStore objects
   ///
   /// This class is used when connecting TAuxStore objects to the
   /// input tree as the auxiliary store of a DV container.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TAuxManager : public TVirtualManager {

   public:
      /// Constructor getting hold of an auxiliary store object
      TAuxManager( TAuxStore* store, ::Bool_t sharedOwner = kTRUE );
      /// Copy constructor
      TAuxManager( const TAuxManager& parent );

      /// Assignment operator
      TAuxManager& operator= ( const TAuxManager& rhs );

      /// Function for updating the object in memory if needed
      virtual ::Int_t getEntry( ::Int_t getall = 0 ) override;

      /// Function getting a const pointer to the object being handled
      virtual const void* object() const override;
      /// Function getting a pointer to the object being handled
      virtual void* object() override;
      /// Function replacing the object being handled
      virtual void setObject( void* obj ) override;

      /// Create the object for the current event
      virtual ::Bool_t create() override;
      /// Check if the object was set for the current event
      virtual ::Bool_t isSet() const override;
      /// Reset the object at the end of processing of an event
      virtual void reset() override;

      /// Get a type-specific pointer to the managed object
      TAuxStore* getStore();
      /// Get a convenience pointer to the managed object
      const SG::IConstAuxStore* getConstStore() const;

   private:
      /// The auxiliary store object
      std::shared_ptr< TAuxStore > m_store;
      /// Pointer to the auxiliary store object
      TAuxStore* m_storePtr;

   }; // class TAuxManager

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TAUXMANAGER_H
