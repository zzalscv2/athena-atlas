// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_TVIRTUALMANAGER_H
#define XAODROOTACCESS_TOOLS_TVIRTUALMANAGER_H

// ROOT include(s):
#include <Rtypes.h>

namespace xAOD {

   /// @short Interface class for the "manager classes"
   ///
   /// The TEvent class handles the reading of (a collection of) branches
   /// through "manager objects". Since different kinds of manager
   /// classes are available, they need a common ancestor to make their
   /// usage nicer.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TVirtualManager {

   public:
      /// Virtual destructor, to make vtable happy...
      virtual ~TVirtualManager() {}

      /// Function for updating the object in memory if needed
      virtual ::Int_t getEntry( ::Int_t getall = 0 ) = 0;

      /// Function getting a const pointer to the object being handled
      virtual const void* object() const = 0;
      /// Function getting a pointer to the object being handled
      virtual void* object() = 0;
      /// Function replacing the object being handled
      virtual void setObject( void* obj ) = 0;

      /// Create the object for the current event
      virtual ::Bool_t create() = 0;
      /// Check if the object was set for the current event
      virtual ::Bool_t isSet() const = 0;
      /// Reset the object at the end of processing of an event
      virtual void reset() = 0;

   }; // class TVirtualManager

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TVIRTUALMANAGER_H
