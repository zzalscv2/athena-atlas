/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ByteStreamAuxContainer_v1.cxx 611309 2014-08-11 16:51:58Z ssnyder $

// System include(s):
#include <iostream>

// EDM include(s):
#include "xAODCore/tools/AuxPersVector.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"
#include "AthContainers/tools/foreach.h"

// Local include(s):
#include "xAODTrigger/versions/ByteStreamAuxContainer_v1.h"

namespace xAOD {

   ByteStreamAuxContainer_v1::ByteStreamAuxContainer_v1()
      : SG::IAuxStore(),
        m_int(), m_float(), m_vecInt(), m_vecFloat(),
        m_staticVecs(), m_dynamicVecs(),
        m_locked( false ),
        m_tick( 0 ) {

   }

   ByteStreamAuxContainer_v1::
   ByteStreamAuxContainer_v1( const ByteStreamAuxContainer_v1& parent )
      : SG::IAuxStore( parent ),
        m_int( parent.m_int ), m_float( parent.m_float ),
        m_vecInt( parent.m_vecInt ), m_vecFloat( parent.m_vecFloat ),
        // The auxiliary IDs are copied:
        m_auxids( parent.m_auxids ),
        // But the internal pointers are not:
        m_staticVecs(),
        m_dynamicVecs(), m_locked( false ),
        m_tick( 1 ) {

   }

   ByteStreamAuxContainer_v1::~ByteStreamAuxContainer_v1() {

      // Clean up the helper objects:
      std::vector< SG::IAuxTypeVector* >::iterator itr = m_dynamicVecs.begin();
      std::vector< SG::IAuxTypeVector* >::iterator end = m_dynamicVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) delete *itr;
      }
      itr = m_staticVecs.begin();
      end = m_staticVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) delete *itr;
      }
   }

   ByteStreamAuxContainer_v1&
   ByteStreamAuxContainer_v1::
   operator=( const ByteStreamAuxContainer_v1& rhs ) {

      if (this != &rhs) {
        // Copy the persistent variables:
        m_int      = rhs.m_int;
        m_float    = rhs.m_float;
        m_vecInt   = rhs.m_vecInt;
        m_vecFloat = rhs.m_vecFloat;

        // Also copy the list of auxiliary IDs handled:
        m_auxids        = rhs.m_auxids;
        ++m_tick;

        // But then, reset the internal pointers:
        m_staticVecs.clear();
        m_dynamicVecs.clear();
      }

      return *this;
   }

   const void* ByteStreamAuxContainer_v1::getData( auxid_t auxid ) const {

      guard_t guard (m_mutex);

      // If it's a static auxiliary property:
      if( ( auxid < m_staticVecs.size() ) && ( m_staticVecs[ auxid ] ) ) {
         // We're done already:
         return m_staticVecs[ auxid ]->toPtr();
      }
      // If it's a dynamic one:
      else if( ( auxid < m_dynamicVecs.size() ) &&
               ( m_dynamicVecs[ auxid ] ) ) {
         // We're done already:
         return m_dynamicVecs[ auxid ]->toPtr();
      }

      // Try to retrieve the dynamic variable:
      // quiet on because this method may be called from isAccessible
      // and that shouldn't generate any output.
      return getData1 (auxid, 0, 0, true, false);
   }

   const ByteStreamAuxContainer_v1::auxid_set_t&
   ByteStreamAuxContainer_v1::getAuxIDs() const {

      // Return the full list of IDs:
      return getWritableAuxIDs();
   }

   void* ByteStreamAuxContainer_v1::getData( auxid_t auxid, size_t size,
                                             size_t capacity ) {

      guard_t guard (m_mutex);

      // Check if we have such a static variable:
      if( ( auxid < m_staticVecs.size() ) && ( m_staticVecs[ auxid ] ) ) {
         // Set it to the right size:
         m_staticVecs[ auxid ]->reserve( capacity );
         m_staticVecs[ auxid ]->resize( size );
         // We're done already:
         return m_staticVecs[ auxid ]->toPtr();
      }
      // Check if we already know about this dynamic variable:
      else if( ( auxid < m_dynamicVecs.size() ) &&
               ( m_dynamicVecs[ auxid ] ) ) {
         // Set it to the right size:
         m_dynamicVecs[ auxid ]->reserve( capacity );
         m_dynamicVecs[ auxid ]->resize( size );
         // We're done already:
         return m_dynamicVecs[ auxid ]->toPtr();
      }

      return getData1 (auxid, size, capacity, false, false);
   }

   const ByteStreamAuxContainer_v1::auxid_set_t&
   ByteStreamAuxContainer_v1::getWritableAuxIDs() const {

      // Return the full list of IDs:
      guard_t guard (m_mutex);
      if (m_tsAuxids.get() == 0) {
        m_tsAuxids.reset (new TSAuxidSet (m_tick, m_auxids));
      }
      else if (m_tsAuxids->m_tick != m_tick) {
        m_tsAuxids->m_set = m_auxids;  // May need to optimize this!
        m_tsAuxids->m_tick = m_tick;
      }
      return m_tsAuxids->m_set;
   }

   void ByteStreamAuxContainer_v1::resize( size_t size ) {

      guard_t guard (m_mutex);

      if (m_locked)
        throw SG::ExcStoreLocked ("resize");

      std::vector< SG::IAuxTypeVector* >::iterator itr = m_dynamicVecs.begin();
      std::vector< SG::IAuxTypeVector* >::iterator end = m_dynamicVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) ( *itr )->resize( size );
      }
      itr = m_staticVecs.begin();
      end = m_staticVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) ( *itr )->resize( size );
      }

      return;
   }

   void ByteStreamAuxContainer_v1::reserve( size_t size ) {

      guard_t guard (m_mutex);

      if (m_locked)
        throw SG::ExcStoreLocked ("reserve");

      std::vector< SG::IAuxTypeVector* >::iterator itr = m_dynamicVecs.begin();
      std::vector< SG::IAuxTypeVector* >::iterator end = m_dynamicVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) ( *itr )->reserve( size );
      }
      itr = m_staticVecs.begin();
      end = m_staticVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) ( *itr )->reserve( size );
      }

      return;
   }

   void ByteStreamAuxContainer_v1::shift( size_t pos, ptrdiff_t offs ) {

      guard_t guard (m_mutex);

      if (m_locked)
        throw SG::ExcStoreLocked ("shift");

      std::vector< SG::IAuxTypeVector* >::iterator itr = m_dynamicVecs.begin();
      std::vector< SG::IAuxTypeVector* >::iterator end = m_dynamicVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) ( *itr )->shift( pos, offs );
      }
      itr = m_staticVecs.begin();
      end = m_staticVecs.end();
      for( ; itr != end; ++itr ) {
         if( *itr ) ( *itr )->shift( pos, offs );
      }

      return;
   }

   void ByteStreamAuxContainer_v1::reset() {

      guard_t guard (m_mutex);

      ATHCONTAINERS_FOREACH (SG::IAuxTypeVector* p, m_dynamicVecs)
         delete p;
      m_dynamicVecs.clear();

      SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
#define ADD_IDS(VAR, TYP) \
      do { typedef std::map< std::string, std::vector< TYP > > CONT; \
           ATHCONTAINERS_FOREACH (CONT::value_type& p, VAR) \
             m_auxids.insert (r.getAuxID< TYP > (p.first)); } while(0)
      ADD_IDS(m_int, int);
      ADD_IDS(m_float, float);
      ADD_IDS(m_vecInt, std::vector<int>);
      ADD_IDS(m_vecFloat, std::vector<float>);
#undef ADD_IDS
      ++m_tick;

      return;
   }

   /// Look for variable auxid in pers.
   /// If not found, create it if capacity != 0.
   template< typename T >
   void*
   ByteStreamAuxContainer_v1::getData1( auxid_t auxid,
                                        std::map< std::string,
                                                  std::vector< T > >& pers,
                                        size_t size, size_t capacity,
                                        bool quiet,
                                        bool forDecor) const {

      // Private method --- should hold lock before calling this.

      // Look up the name of the variable:
      const std::string name = SG::AuxTypeRegistry::instance().getName( auxid );

      // Try to find the variable:
      typename std::map< std::string, std::vector< T > >::iterator itr =
         pers.find( name );
      if( itr == pers.end() ) {
         // Variable isn't there.  Fail if capacity==0.
         if (capacity == 0) {
           if (!quiet) {
             std::cerr << "ERROR xAOD::ByteStreamAuxContainer_v1::getData (const) "
                       << "Variable with unknown name (" << name << ") requested"
                       << std::endl;
           }
           return 0;
         }

         if (m_locked && !forDecor)
           throw SG::ExcStoreLocked ("getData");

         // Create the variable.
         itr = pers.insert (std::make_pair (name, std::vector<T>())).first;
      }

      // Get hold of the variable.
      std::vector< T >& var = itr->second;

      // Make sure that the internal vector is big enough:
      if( m_dynamicVecs.size() <= auxid ) {
         m_dynamicVecs.resize( auxid + 1 );
      }

      // Just an internal check...
      if( m_dynamicVecs[ auxid ] ) {
         std::cerr << "ERROR xAOD::ByteStreamAuxContainer_v1::getData "
                   << "Internal inconsistency detected!" << std::endl;
         return m_dynamicVecs[ auxid ]->toPtr();
      }

      // Register the variable:
      m_dynamicVecs[ auxid ] = new AuxPersVector< T >( var );

      if (capacity > 0) {
        // Set it to the right size:
        m_dynamicVecs[ auxid ]->reserve( capacity );
        m_dynamicVecs[ auxid ]->resize( size );
      }

      // Remember that we are now handling this variable:
      m_auxids.insert( auxid );
      ++m_tick;

      // Return the pointer to the array:
      return m_dynamicVecs[ auxid ]->toPtr();
   }

   void*
   ByteStreamAuxContainer_v1::getData1( auxid_t auxid,
                                        size_t size, size_t capacity,
                                        bool quiet,
                                        bool forDecor) const {

      // Private method --- should hold lock before calling this.

      if( *( SG::AuxTypeRegistry::instance().getType( auxid ) ) ==
          typeid( int ) ) {

         return getData1( auxid, m_int, size, capacity, quiet, forDecor );
      }
      else if( *( SG::AuxTypeRegistry::instance().getType( auxid ) ) ==
               typeid( float ) ) {

         return getData1( auxid, m_float, size, capacity, quiet, forDecor );
      }
      else if( *( SG::AuxTypeRegistry::instance().getType( auxid ) ) ==
               typeid( std::vector< int > ) ) {

         return getData1( auxid, m_vecInt, size, capacity, quiet, forDecor );
      }
      else if( *( SG::AuxTypeRegistry::instance().getType( auxid ) ) ==
               typeid( std::vector< float > ) ) {

         return getData1( auxid, m_vecFloat, size, capacity, quiet, forDecor );
      }

      // The object can't handle this variable type...
      std::cerr << "ERROR xAOD::ByteStreamAuxContainer_v1::getData "
                << "Unknown variable type ("
                << SG::AuxTypeRegistry::instance().getType( auxid )->name()
                << ") requested" << std::endl;

      return 0;
   }

   /// Get a pointer to a given array, as a decoration.
   void* ByteStreamAuxContainer_v1::getDecoration (auxid_t auxid,
                                                   size_t size,
                                                   size_t capacity)
   {
      guard_t guard (m_mutex);

      // Does this variable already exist?
      void* ret = 0;
      if( ( auxid < m_staticVecs.size() ) && ( m_staticVecs[ auxid ] ) ) {
        ret = m_staticVecs[ auxid ]->toPtr();
      }
      // If it's a dynamic one:
      else if( ( auxid < m_dynamicVecs.size() ) &&
               ( m_dynamicVecs[ auxid ] ) ) {
        ret = m_dynamicVecs[ auxid ]->toPtr();
      }

      if (!ret)
        ret = getData1 (auxid, 0, 0, true, true);

      if (ret) {
        // Raise exception if locked and not a decoration.
        if (m_locked) {
          if ( ! (auxid < m_isDecoration.size() && m_isDecoration[auxid]) )
            throw SG::ExcStoreLocked (auxid);
        }
        return ret;
      }

      // Make a new variable.
      ret = getData1 (auxid, size, capacity, false, true);

      // If locked, mark as a decoration.
      if (m_locked) {
        if (m_isDecoration.size() <= auxid)
          m_isDecoration.resize (auxid+1);
        m_isDecoration[auxid] = true;
      }

      return ret;
   }


   void ByteStreamAuxContainer_v1::lock()
   {
       guard_t guard (m_mutex);
       m_locked = true;
   }

   void ByteStreamAuxContainer_v1::clearDecorations()
   {
     guard_t guard (m_mutex);

     const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

     size_t sz = m_isDecoration.size();
     for (auxid_t auxid = 0; auxid < sz; auxid++) {
       if (m_isDecoration[auxid]) {
         if (m_dynamicVecs[auxid]) {
           delete m_dynamicVecs[auxid];
           m_dynamicVecs[auxid] = 0;
           m_auxids.erase( auxid );
           ++m_tick;

           const std::string name = r.getName( auxid );
           const std::type_info* ti = r.getType( auxid );
           if (ti == &typeid(int))
             m_int.erase (name);
           else if (ti == &typeid(float))
             m_float.erase (name);
           else if (ti == &typeid(std::vector<int>))
             m_vecInt.erase (name);
           else if (ti == &typeid(std::vector<float>))
             m_vecFloat.erase (name);
         }
       }
     }
     m_isDecoration.clear();
   }


   size_t ByteStreamAuxContainer_v1::size() const
   {
     guard_t guard (m_mutex);
     auxid_set_t::const_iterator i = m_auxids.begin();
     auxid_set_t::const_iterator end = m_auxids.end();
     for (; i != end; ++i) {
       if (*i < m_staticVecs.size() && m_staticVecs[*i]) {
         size_t sz = m_staticVecs[*i]->size();
         if (sz > 0)
           return sz;
       }
       if (*i < m_dynamicVecs.size() && m_dynamicVecs[*i]) {
         size_t sz = m_dynamicVecs[*i]->size();
         if (sz > 0)
           return sz;
       }
     }

     return 0;
   }


} // namespace xAOD
