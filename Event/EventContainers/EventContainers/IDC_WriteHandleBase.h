/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IDC_WRITEHANDLEBASE_H
#define IDC_WRITEHANDLEBASE_H
#include <atomic>
#ifndef __cpp_lib_atomic_wait
//These are not needed when atomic wait is supported
//This code can be removed once C++20(+) is firmly adopted
#include <condition_variable>
#include <mutex>
#endif

namespace EventContainers {

#ifndef __cpp_lib_atomic_wait
struct mutexPair{
   std::condition_variable condition;
   std::mutex mutex;
   mutexPair() : condition(), mutex() {
   }
};
#endif

class IDC_WriteHandleBase{
protected:
   std::atomic<const void*>*  m_atomic;
#ifndef __cpp_lib_atomic_wait
   mutexPair *m_mut;
#endif

   IDC_WriteHandleBase() : m_atomic(nullptr)
#ifndef __cpp_lib_atomic_wait
   , m_mut(nullptr)
#endif
   { }
public:

#ifndef __cpp_lib_atomic_wait
   void LockOn(std::atomic<const void*>* in, mutexPair *pair) noexcept {
      m_atomic = in;
      m_mut    = pair;
   }
#else
   void LockOn(std::atomic<const void*>* in) noexcept {
      m_atomic = in;
   }
#endif
   void DropLock() noexcept;
   void ReleaseLock();

   ~IDC_WriteHandleBase();
};

}
#endif

