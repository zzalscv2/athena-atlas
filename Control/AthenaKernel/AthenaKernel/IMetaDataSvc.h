/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IMETADATASVC_H
#define ATHENAKERNEL_IMETADATASVC_H

/** @file IMetaDataSvc.h
 *  @brief This file contains the class definition for the IMetaDataSvc class.
 *  @author Marcin Nowak
 *  @author Frank Berghaus
 **/

#include "GaudiKernel/INamedInterface.h"
#include "AthenaKernel/MetaCont.h"
#include "StoreGate/StoreGateSvc.h"

#include <string>
#include <set>
#include <mutex>
#include <typeinfo>

/** @class IMetaDataSvc
 *  @brief This class provides the interface for MetaDataSvc
 **/
class IMetaDataSvc : virtual public ::INamedInterface {

public: // Non-static members

   
   /// used by AthenaPoolCnvSvc
   virtual StatusCode shmProxy(const std::string& filename) = 0;

   /// Get all per-stream Key variants created for in-file metadata object with original key - if none, return key
   virtual std::set<std::string> getPerStreamKeysFor(const std::string& key ) const;

   // =======  Methods for handling metadata objects stored in MetaContainers (EventService)
   template <typename T, class TKEY>
   T* tryRetrieve (const TKEY& key) const;

   template <typename T, class TKEY>
   const T* tryConstRetrieve(const TKEY& key) const;

   /// Record an object with a key.
   template <typename T, typename TKEY> 
   StatusCode record(T* p2BRegistered, const TKEY& key);

   /// Record an object with a key, take ownership of the unique_ptr obj
   template <typename T, typename TKEY> 
   StatusCode record(std::unique_ptr<T> pUnique, const TKEY& key);

   /// Remove object with this type+key
   template <typename T, typename TKEY> 
   StatusCode remove(const TKEY& key, bool ignoreIfAbsent=false);

   /// Check if object is already is already in store
   template <typename T, typename TKEY>
   bool contains(const TKEY& key);

   /// The output MetaData Store
   virtual StoreGateSvc* outputDataStore() const = 0;

   /// rangeID for the current EventContext - used to index MetaContainers - 
   virtual const std::string currentRangeID() const = 0;

   /// Gaudi boilerplate
   static const InterfaceID& interfaceID();

   /// Hook for implementation to react to recording an object
   virtual void recordHook(const std::type_info&) {}

   /// Hook for implementation to react to removing an object
   virtual void removeHook(const std::type_info&) {}

private: // Data
   std::mutex    m_mutex;
};

 
inline const InterfaceID& IMetaDataSvc::interfaceID() {
   static const InterfaceID IID("IMetaDataSvc", 1, 0);
   return(IID);
}

/// default implementation that maps a key to itself - overwritten in MetaDataSvc
inline std::set<std::string>
IMetaDataSvc::getPerStreamKeysFor(const std::string& key ) const {
   return std::set<std::string>( {key} );
}

/**
 * @brief Retrieve an object of type @c T from MetaDataStore 
 *        Return 0 if not found. Don't print any WARNINGs
 * @param key The key to use for the lookup.
 **/
template <typename T, class TKEY>
T* IMetaDataSvc::tryRetrieve (const TKEY& key) const
{
   const MetaCont<T>* container = outputDataStore()->tryRetrieve< MetaCont<T> >(key);
   if( container ) {
      return container->get( currentRangeID() );
   }
   return nullptr;
}

template <typename T, class TKEY>
const T* IMetaDataSvc::tryConstRetrieve (const TKEY& key) const
{
   const MetaCont<T>* container = outputDataStore()->tryRetrieve< MetaCont<T> >(key);
   if( container ) {
      return container->get( currentRangeID() );
   }
   return nullptr;
}

template <typename T, typename TKEY> 
StatusCode IMetaDataSvc::record(T* pObject, const TKEY& key)
{
   std::lock_guard lock(m_mutex);
   MetaCont<T>* container = outputDataStore()->tryRetrieve< MetaCont<T> >(key);
   StatusCode sc = StatusCode::FAILURE;
   if( !container ) {
      auto cont_uptr = std::make_unique< MetaCont<T> >();
      if( cont_uptr->insert( currentRangeID() , pObject) )
         sc = outputDataStore()->record( std::move(cont_uptr), key );
   } else {
      if (container->insert(currentRangeID(), pObject)) sc = StatusCode::SUCCESS;
   }
   if (sc.isSuccess()) recordHook(typeid(T));
   return sc;
}


template <typename T, typename TKEY> 
StatusCode IMetaDataSvc::record(std::unique_ptr<T> pUnique, const TKEY& key)
{
   if( this->record( pUnique.get(), key ).isSuccess() ) {
      (void)pUnique.release();
      return StatusCode::SUCCESS;
   }
   pUnique.reset();
   return StatusCode::FAILURE;
}


template <typename T, class TKEY>
StatusCode IMetaDataSvc::remove(const TKEY& key, bool ignoreIfAbsent)
{
   std::lock_guard lock(m_mutex);
   // change erase to setting nullptr?
   MetaCont<T>* container = outputDataStore()->tryRetrieve< MetaCont<T> >(key);
   if (container && container->erase(currentRangeID())) {
     if (container->entries() == 0u) removeHook(typeid(T));
     return StatusCode::SUCCESS;
   }
   return ignoreIfAbsent? StatusCode::SUCCESS : StatusCode::FAILURE;
}

template <typename T, typename TKEY>
bool IMetaDataSvc::contains(const TKEY& key) {
  if (!outputDataStore()->contains< MetaCont<T> >(key))
    return false;
  const MetaCont<T>* container =
      outputDataStore()->tryConstRetrieve< MetaCont<T> >(key);
  return container && container->valid(currentRangeID());
}

#endif
