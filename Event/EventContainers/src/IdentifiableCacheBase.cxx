/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: IdentifiableCacheBase.cxx 791541 2017-01-09 10:43:53Z smh $
/**
 * @file IdentifiableCacheBase.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief 
 */

// null --- default state.
// ABORTED -- conversion failed or returned null
// ptr > null && ptr < ABORTED --- Have payload
// INVALID --- Conversion in progress or intention to add soon.

#include "EventContainers/IdentifiableCacheBase.h"
#include "CxxUtils/AthUnlikelyMacros.h"

namespace EventContainers {

const void* const INVALID = reinterpret_cast<const void*>(IdentifiableCacheBase::INVALIDflag);
const void* const ABORTED = reinterpret_cast<const void*>(IdentifiableCacheBase::ABORTEDflag);

IdentifiableCacheBase::IdentifiableCacheBase (IdentifierHash maxHash,
                                              const IMaker* maker)
  : IdentifiableCacheBase(maxHash, maker, s_defaultBucketSize)
{
}

IdentifiableCacheBase::IdentifiableCacheBase (IdentifierHash maxHash,
                                              const IMaker* maker, size_t lockBucketSize)
  : m_vec(maxHash),
    m_maker (maker), m_NMutexes(lockBucketSize), m_currentHashes(0)
{
   if(m_NMutexes>0) m_HoldingMutexes = std::make_unique<mutexPair[]>(m_NMutexes);
}


IdentifiableCacheBase::~IdentifiableCacheBase()=default;

int IdentifiableCacheBase::tryLock(IdentifierHash hash, IDC_WriteHandleBase &lock, std::vector<IdentifierHash> &wait){
   assert(m_NMutexes > 0);
   const void *ptr1 =nullptr;

   if(m_vec[hash].compare_exchange_strong(ptr1, INVALID, std::memory_order_relaxed, std::memory_order_relaxed)){//atomic swap (replaces ptr1 with value)
      //First call
      size_t slot = hash % m_NMutexes;
      auto &mutexpair = m_HoldingMutexes[slot];
      lock.LockOn(&m_vec[hash], &mutexpair);
      return 0;
   }

   if(ptr1 == INVALID){
      //Second call while not finished
      wait.emplace_back(hash);
      return 1;
   }
   if(ptr1 == ABORTED) return 3;
   return 2;   //Already completed
}


void IdentifiableCacheBase::clear (deleter_f* deleter)
{
  size_t s = m_vec.size();
  if(0 != m_currentHashes.load(std::memory_order_relaxed)){
     for (size_t i=0; i<s ;i++) {
       const void* ptr = m_vec[i].load(std::memory_order_relaxed);
       m_vec[i].store(nullptr, std::memory_order_relaxed);
       if (ptr && ptr < ABORTED){
         deleter (ptr);
      }
     }
     m_currentHashes.store(0, std::memory_order_relaxed);
  }else{
    for (size_t i=0; i<s ;i++) m_vec[i].store(nullptr, std::memory_order_relaxed);//Need to clear incase of aborts
  }
}


//Does not lock or clear atomics to allow faster destruction
void IdentifiableCacheBase::cleanUp (deleter_f* deleter)
{
  std::atomic_thread_fence(std::memory_order_acquire);
  if(0 != m_currentHashes.load(std::memory_order_relaxed)){ //Reduce overhead if cache was unused
    size_t s = m_vec.size();
    for (size_t i=0; i<s ;i++) {
      const void* p = m_vec[i].load(std::memory_order_relaxed);
      if(p && p < ABORTED) deleter (p);
    }
  }
}

int IdentifiableCacheBase::itemAborted (IdentifierHash hash){
   const void* p = m_vec[hash].load(std::memory_order_relaxed); //Relaxed because it is not returning a pointer to anything
   return (p == ABORTED);
}


int IdentifiableCacheBase::itemInProgress (IdentifierHash hash){
   const void* p = m_vec[hash].load(std::memory_order_relaxed); //Relaxed because it is not returning a pointer to anything
   return (p == INVALID);
}


const void* IdentifiableCacheBase::find (IdentifierHash hash) noexcept
{
  if (ATH_UNLIKELY(hash >= m_vec.size())) return nullptr;
  const void* p = m_vec[hash].load(std::memory_order_relaxed);
  if (p >= ABORTED)
    return nullptr;
  //Now we know it is a real pointer we can ensure the data is synced
  std::atomic_thread_fence(std::memory_order_acquire);
  return p;
}

const void* IdentifiableCacheBase::waitFor(IdentifierHash hash)
{
   const void* item = m_vec[hash].load(std::memory_order_acquire);
   if(m_NMutexes ==0) return item;
   size_t slot = hash % m_NMutexes;
   if(item == INVALID){
      mutexPair &mutpair = m_HoldingMutexes[slot];
      uniqueLock lk(mutpair.mutex);
      while( (item =m_vec[hash].load(std::memory_order_relaxed)) ==  INVALID){
        mutpair.condition.wait(lk);
      }
   }
   std::atomic_thread_fence(std::memory_order_acquire);
   return item;
}

const void* IdentifiableCacheBase::findWait (IdentifierHash hash)
{
  if (ATH_UNLIKELY(hash >= m_vec.size())) return nullptr;
  const void* p = waitFor(hash);
  if(p>=ABORTED) return nullptr;
  return p;
}

void IdentifiableCacheBase::notifyHash(IdentifierHash hash)
{
    size_t slot = hash % m_NMutexes;
    mutexPair &mutpair = m_HoldingMutexes[slot];
    lock_t lk(mutpair.mutex);
    mutpair.condition.notify_all();
}

const void* IdentifiableCacheBase::get (IdentifierHash hash)
{
  // If it's there already, return directly without locking.
  const void* ptr = nullptr;
  if (ATH_UNLIKELY(hash >= m_vec.size())) return ptr;

  if(m_vec[hash].compare_exchange_strong(ptr, INVALID) ) {//Exchanges ptr with current value!!
     // Make the payload.
     if(m_maker == nullptr){
        m_vec[hash].store( ABORTED );
        return nullptr;
     }
     uniqueLock lock(m_mutex, std::defer_lock);
     if(!m_maker->m_IsReEntrant) lock.lock();//Allow reentrant or non reentrant makers

     try {
       ptr = m_maker->typelessMake (hash).release();
     }
     catch (...) {
       // FIXME: Can this be done with RAII?
       if(m_NMutexes >0) notifyHash(hash);
       throw;
     }
     assert(m_vec[hash] == INVALID);
     if(ptr){
        m_vec[hash].store( ptr );
        m_currentHashes++;
     }else{
        m_vec[hash].store( ABORTED );
     }
     if(m_NMutexes >0) notifyHash(hash);
  }
  else if(ptr == INVALID){
     ptr= waitFor(hash);
  }
  if(ptr == ABORTED) return nullptr;
  assert(ptr < ABORTED);
  return ptr;
}

void IdentifiableCacheBase::createSet (const std::vector<IdentifierHash>& hashes, std::vector<bool> &mask){
   assert(mask.size() == fullSize());
   for(IdentifierHash hash : hashes){
      const void* ptr = get(hash);
      if(ptr !=nullptr) mask[hash] = true;
   }
}


size_t IdentifiableCacheBase::numberOfHashes()
{
  return m_currentHashes.load(std::memory_order_relaxed); //Not to be used for syncing
}

std::vector<IdentifierHash> IdentifiableCacheBase::ids()
{
  std::vector<IdentifierHash> ret;
  ret.reserve (m_currentHashes.load(std::memory_order_relaxed));
  size_t s = m_vec.size();
  for (size_t i =0; i<s; i++) {
    const void* p = m_vec[i].load(std::memory_order_relaxed);
    if (p && p < ABORTED)
      ret.push_back (i);
  }
  return ret;
}


std::pair<bool, const void*> IdentifiableCacheBase::add (IdentifierHash hash, const void* p) noexcept
{
  if (ATH_UNLIKELY(hash >= m_vec.size())) return std::make_pair(false, nullptr);
  if(p==nullptr) return std::make_pair(false, nullptr);
  const void* nul=nullptr;
  if(m_vec[hash].compare_exchange_strong(nul, p, std::memory_order_release, std::memory_order_relaxed)){
     m_currentHashes.fetch_add(1, std::memory_order_relaxed);
     return std::make_pair(true, p);
  }
  const void* invalid = INVALID;
  if(m_vec[hash].compare_exchange_strong(invalid, p, std::memory_order_release, std::memory_order_acquire)){
     m_currentHashes.fetch_add(1, std::memory_order_relaxed);
     return std::make_pair(true, p);
  }
  return std::make_pair(false, invalid);
}


std::pair<bool, const void*> IdentifiableCacheBase::addLock (IdentifierHash hash, const void* p) noexcept
{ //Same as method above except we check for invalid state first,
  // more optimal for calling using writehandle lock method
  assert(hash < m_vec.size());
  if(p==nullptr) return std::make_pair(false, nullptr);
  const void* invalid = INVALID;
  if(m_vec[hash].compare_exchange_strong(invalid, p, std::memory_order_release, std::memory_order_relaxed)){
     m_currentHashes.fetch_add(1, std::memory_order_relaxed);
     return std::make_pair(true, p);
  }
  const void* nul=nullptr;
  if(m_vec[hash].compare_exchange_strong(nul, p, std::memory_order_release, std::memory_order_acquire)){
     m_currentHashes.fetch_add(1, std::memory_order_relaxed);
     return std::make_pair(true, p);
  }
  return std::make_pair(false, nul);
}

std::pair<bool, const void*> IdentifiableCacheBase::addLock (IdentifierHash hash,
                                 void_unique_ptr p) noexcept
{
  std::pair<bool, const void*> b = addLock(hash, p.get());
  if(b.first) p.release();
  return b;
}


std::pair<bool, const void*> IdentifiableCacheBase::add (IdentifierHash hash,
                                 void_unique_ptr p) noexcept
{
  std::pair<bool, const void*> b = add(hash, p.get());
  if(b.first) p.release();
  return b;
}



} // namespace EventContainers


