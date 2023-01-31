/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
#include "xAODRootAccess/tools/TEventFormatRegistry.h"

#include <TFile.h>

#include <mutex>

namespace xAOD {

   const TEventFormatRegistry& TEventFormatRegistry::instance() {

      static const TEventFormatRegistry instance;
      return instance;
   }

   EventFormat& TEventFormatRegistry::getEventFormat( const TFile* file ) const {

      static std::mutex mutex;
      std::scoped_lock lock(mutex);
      auto this_nc ATLAS_THREAD_SAFE = const_cast<TEventFormatRegistry*>(this);
      return this_nc->m_eventFormats[ file ];
   }

   void TEventFormatRegistry::merge( const TFile* file,
                                     const EventFormat& ef ) {

      // Get the local EventFormat object:
      EventFormat& localEf = getEventFormat( file );

      // Loop over the contents of the new object:
      EventFormat::const_iterator itr = ef.begin();
      EventFormat::const_iterator end = ef.end();
      for( ; itr != end; ++itr ) {
         // If it's already in the output, continue:
         if( localEf.exists( itr->first ) ) continue;
         // If not, let's add it:
         localEf.add( *( ef.get( itr->first ) ) );
      }
   }

   TEventFormatRegistry::TEventFormatRegistry()
      : m_eventFormats() {

   }

} // namespace xAOD
