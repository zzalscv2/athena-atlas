/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// ROOT include(s):
#include <TFile.h>

// Local include(s):
#include "xAODRootAccess/tools/TEventFormatRegistry.h"

namespace xAOD {

   TEventFormatRegistry& TEventFormatRegistry::instance() {

      static TEventFormatRegistry instance;
      return instance;
   }

   EventFormat& TEventFormatRegistry::getEventFormat( const TFile* file ) {

      return m_eventFormats[ file ];
   }

   void TEventFormatRegistry::merge( const TFile* file,
                                     const EventFormat& ef ) {

      // Get the local EventFormat object:
      EventFormat& localEf = m_eventFormats[ file ];

      // Loop over the contents of the new object:
      EventFormat::const_iterator itr = ef.begin();
      EventFormat::const_iterator end = ef.end();
      for( ; itr != end; ++itr ) {
         // If it's already in the output, continue:
         if( localEf.exists( itr->first ) ) continue;
         // If not, let's add it:
         localEf.add( *( ef.get( itr->first ) ) );
      }

      return;
   }

   TEventFormatRegistry::TEventFormatRegistry()
      : m_eventFormats() {

   }

} // namespace xAOD
