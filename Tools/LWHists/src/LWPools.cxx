/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class LWPools               //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2009                            //
//                                                            //
////////////////////////////////////////////////////////////////

#include "LWPools.h"

//____________________________________________________________________
LWPools::PoolList LWPools::s_pools;
std::atomic<long long> LWPools::s_bytesDynAlloc = 0;

//____________________________________________________________________
LWPools::PoolList::PoolList()
{
  nullout();
}

//____________________________________________________________________
void LWPools::PoolList::nullout()
{
  for (size_t i = 0; i < LWPoolSelector::nPools(); i++) {
    m_poolarr[i] = nullptr;
  }
}

//____________________________________________________________________
void LWPools::PoolList::cleanup()
{
#ifdef LW_DEBUG_POOLS_DEBUG_USAGE
  std::scoped_lock lock (m_mutex);
  if (!m_memoryHandedOut.empty())
    std::cout<<"LWPools::PoolList::cleanup() WARNING: "<<m_memoryHandedOut.size()
	     <<" unreleased pool allocations"<<std::endl;
#endif
  for (unsigned i=0;i<LWPoolSelector::nPools();++i) {
    LWPool* p = m_poolarr[i].exchange (nullptr);
    if (p)
      s_bytesDynAlloc -= sizeof(LWPool);
    delete p;
  }
  LWPool::forceCleanupMotherPool();
}

//____________________________________________________________________
LWPool * LWPools::initPool(unsigned idx,unsigned length)
{
  assert(idx<LWPoolSelector::numberOfPools);
  assert(LWPoolSelector::poolIndex(length)==idx);
  LWPool * pool = new LWPool(LWPoolSelector::poolSize(length));
  s_bytesDynAlloc += sizeof(LWPool);
  LWPool* exp = nullptr;
  s_pools[idx].compare_exchange_strong (exp, pool);
  if (exp) {
    s_bytesDynAlloc -= sizeof(LWPool);
    delete pool;
    pool = exp;
  }
  return pool;
}

//____________________________________________________________________
void LWPools::cleanup()
{
  s_pools.cleanup();
  //Fixme: something here to flush mother pool also
}

//____________________________________________________________________
long long LWPools::getTotalPoolMemAllocated()
{
  return LWPool::getMotherMemOwned()+s_bytesDynAlloc;
}

//____________________________________________________________________
long long LWPools::getTotalPoolMemUsed()
{
  long long l(s_bytesDynAlloc);
  for (unsigned i=0;i<LWPoolSelector::nPools();++i) {
    LWPool* p = s_pools[i];
    if (p) l+=p->getMemDishedOut();
  }
  return l;
}
