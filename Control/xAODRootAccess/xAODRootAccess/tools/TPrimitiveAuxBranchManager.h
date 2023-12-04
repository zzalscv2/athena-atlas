// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_TPRIMITIVEAUXBRANCHMANAGER_H
#define XAODROOTACCESS_TOOLS_TPRIMITIVEAUXBRANCHMANAGER_H

// EDM include(s):
#include "AthContainersInterfaces/AuxTypes.h"

// Local include(s):
#include "TVirtualManager.h"

// Forward declaration(s):
namespace SG {
   class IAuxTypeVector;
}

namespace xAOD {

   // Forward declaration(s):
   class THolder;

   /// @short Manager for primitive auxiliary branches created dynamically
   ///
   /// One has to treat std::vector<T> and primitive auxiliary branches
   /// differently, because TTree handles these things very differently.
   /// This class manages auxiliary branches created from primitive
   /// variables.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TPrimitiveAuxBranchManager : public TVirtualManager {

   public:
      /// Definition of the auxiliary ID type
      typedef SG::auxid_t auxid_t;

      /// Constructor getting hold of a possible branch
      TPrimitiveAuxBranchManager( auxid_t auxid,
                                  ::TBranch* br = 0,
                                  THolder* holder = 0 );
      /// Copy constructor
      TPrimitiveAuxBranchManager( const TPrimitiveAuxBranchManager& parent );
      /// Destructor
      ~TPrimitiveAuxBranchManager();

      /// Assignment operator
      TPrimitiveAuxBranchManager&
      operator=( const TPrimitiveAuxBranchManager& rhs );

      /// Accessor to the branch
      ::TBranch* branch();
      /// Pointer to the branch's pointer
      ::TBranch** branchPtr();
      /// Accessor to the Holder object (constant version)
      const THolder* holder() const;
      /// Accessor to the Holder object
      THolder* holder();

       /// Function for updating the object in memory if needed
      virtual ::Int_t getEntry( ::Int_t getall = 0 ) override;

      /// Function getting a const pointer to the object being handled
      virtual const void* object() const override;
      /// Function getting a pointer to the object being handled
      virtual void* object();
      /// Function replacing the object being handled
      virtual void setObject( void* obj ) override;

      /// Create the object for the current event
      virtual ::Bool_t create() override;
      /// Check if the object was set for the current event
      virtual ::Bool_t isSet() const override;
      /// Reset the object at the end of processing of an event
      virtual void reset() override;

   private:
      /// Pointer keeping track of the branch
      ::TBranch* m_branch;
      /// Holder object for the EDM object
      THolder* m_holder;
      /// The last entry that was loaded for this branch
      ::Long64_t m_entry;
      /// Was the variable set for the current event?
      ::Bool_t m_isSet;

      /// Auxiliary variable type
      auxid_t m_auxId;
      /// Dummy auxiliary variable for the empty events
      SG::IAuxTypeVector* m_vector;

   }; // class TPrimitiveAuxBranchManager

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TPRIMITIVEAUXBRANCHMANAGER_H
