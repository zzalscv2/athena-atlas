// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_TEVENTFORMATREGISTRY_H
#define XAODROOTACCESS_TOOLS_TEVENTFORMATREGISTRY_H

// EDM include(s):
#include "xAODEventFormat/EventFormat.h"

// Forward declaration(s):
class TFile;

namespace xAOD {

   /// Helper class for managing the event format for output files
   ///
   /// If the user wants to write a single output file with multiple event-level
   /// trees, we need to keep the output EventFormat objects of the multiple
   /// TEvent instances in sync. This singleton class helps out in this.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TEventFormatRegistry {

   public:
      /// Access the only instance of the object in memory
      static const TEventFormatRegistry& instance();

      /// Access the managed EventFormat object
      EventFormat& getEventFormat( const TFile* file ) const;

      /// Merge the contents of another EventFormat object into the managed one
      void merge( const TFile* file, const EventFormat& ef );

   protected:
      /// Hidden constructor
      TEventFormatRegistry();

      /// The process-wide event format object
      std::map< const TFile*, EventFormat > m_eventFormats;

   }; // class TEventFormatRegistry

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TEVENTFORMATREGISTRY_H
