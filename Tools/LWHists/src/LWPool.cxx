/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class LWPool                            //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: April 2009                               //
//                                                            //
////////////////////////////////////////////////////////////////

#include "LWPool.h"
#include "CxxUtils/checker_macros.h"
#include <algorithm>
#include <new>

LWPool * LWPool::getMotherPool()
{
  //The mother pool parameters determine the growth
  //dynamics of all pools:
  static constexpr unsigned extra = sizeof(LWPool)+sizeof(LWPoolAreaBookKeeper)+8;
  static constexpr unsigned poolgrow = 16384+extra;
  static constexpr unsigned motherpoolgrow = poolgrow*32+extra;
  static LWPool mother ATLAS_THREAD_SAFE (poolgrow,motherpoolgrow);
  return &mother;
}

//____________________________________________________________________
LWPool::LWPool(unsigned chunksize)
  : m_chunksize(chunksize),
    m_growsize(LWPool::getMotherPool()->chunkSize())
{
  //Normal constructor
  init();
}

//____________________________________________________________________
LWPool::LWPool(unsigned chunksize,unsigned ngrowsize)
  : m_chunksize(chunksize),
    m_growsize(ngrowsize)
{
  //Mother pool constructor
  init();
}

//____________________________________________________________________
void LWPool::init()
{
  m_likelyNonEmptyArea = 0;
  m_likelyReleaseArea = 0;
  m_nchunksPerArea = UINT_MAX;
  assert(m_chunksize>=1);
  LWHISTMALLOC(sizeof(m_areas[0])*16);
  m_areas.reserve(16);
}

//____________________________________________________________________
LWPool::~LWPool()
{
  erase();
}

//____________________________________________________________________
void LWPool::erase()
{
  std::scoped_lock lock (m_mutex);
  for (LWPoolArea* a : m_areas) {
    char * c = reinterpret_cast<char*>(a);
    if (isMotherPool())
      delete[] c;
    else
      getMotherPool()->release(c);
  }
  m_areas.clear();
  m_likelyNonEmptyArea = nullptr;
  m_likelyReleaseArea = nullptr;
}

//____________________________________________________________________
long long LWPool::getMotherMemOwned()
{
  return getMotherPool()->getMemOwned();
}

//____________________________________________________________________
long long LWPool::getMemOwned() const
{
  std::scoped_lock lock (m_mutex);
  return m_growsize*m_areas.size();
}

//____________________________________________________________________
long long LWPool::getMemDishedOut() const {
  std::scoped_lock lock (m_mutex);
  long long l(0);
  for (LWPoolArea* a : m_areas)
    l += a->getMemDishedOut();
  return l;
}

//____________________________________________________________________
char* LWPool::acquireClean()
{
  char * c = acquire();
  memset(c,0,m_chunksize);
  return c;
}

//____________________________________________________________________
char * LWPool::searchAcquire() // Must hold lock.
{
  m_likelyNonEmptyArea = 0;
  for (LWPoolArea* a : m_areas) {
    char * c = a->acquire();
    if (c) {
      m_likelyNonEmptyArea = a;
      return c;
    }
  }

  //3) All areas empty - create a new area:
  m_likelyNonEmptyArea = grow();
  assert(m_likelyNonEmptyArea);
  return m_likelyNonEmptyArea->acquire();
}

//____________________________________________________________________
LWPoolArea * LWPool::findArea(char* c) // Must hold lock
{
  //Figure out which area (if any), we belong in. For reasons of cpu
  //and (in particular) cache-efficiency we do not actually ask the
  //classes themselves, rather we just look at their addresses (since
  //their addresses marks the beginning of the pool-areas also):

  std::vector<LWPoolArea*>::iterator areaIt =
    std::lower_bound(m_areas.begin(),m_areas.end(),reinterpret_cast<void*>(c));
  LWPoolArea* a(areaIt==m_areas.end()?m_areas.back():*(--areaIt));
  assert(a&&a->belongsInArea(c)&&"Trying to release chunk back to a mem-pool from which it was never acquired");
  assert(reinterpret_cast<char*>(a)<c&&c<reinterpret_cast<char*>(a)+m_growsize);
  return a;
}

//____________________________________________________________________
LWPoolArea* LWPool::grow() // Must hold lock.
{
  char * c(0);
  if (isMotherPool()) {
    LWHISTMALLOC(m_growsize);
    c = new char[m_growsize];
  } else {
    c = getMotherPool()->acquire();
  }
  assert(c);
  LWPoolArea * a = LWPoolArea::create( m_chunksize, c, m_growsize, m_nchunksPerArea);

  if (m_nchunksPerArea==UINT_MAX)
    m_nchunksPerArea = a->totalNumberOfChunks();

  unsigned debug(m_areas.capacity());
  m_areas.push_back(a);
  if (debug!=m_areas.capacity())
    LWHISTMALLOC(sizeof(m_areas.at(0))*m_areas.capacity());
  sort(m_areas.begin(),m_areas.end());//always keep sorted
  //fixme: make slightly faster by manually inserting and moving up mem.
  m_likelyNonEmptyArea = a;

  return a;
}

//____________________________________________________________________
void LWPool::freeArea(LWPoolArea*a) // Must hold lock
{
  assert(!m_areas.empty());

  if (m_likelyNonEmptyArea==a)
    m_likelyNonEmptyArea = 0;
  if (m_likelyReleaseArea==a)
    m_likelyReleaseArea = 0;

  //Find index of the area (fixme: use binary search!):
  unsigned i(0);
  const unsigned nareasplusone(m_areas.size()+1);
  for (;i<nareasplusone;++i) {
    if (a==m_areas.at(i))
      break;
  }
  assert(i<m_areas.size());
  const unsigned nareasminusone(nareasplusone-2);
  if (i<nareasminusone) {
    //This is not the last area in the list, so move those above down by one:
    for (unsigned j=i;j<nareasminusone;++j)
      m_areas.at(j) = m_areas.at(j+1);
  }
  m_areas.resize(nareasminusone);
  char * c = reinterpret_cast<char*>(a);
  if (isMotherPool())
    delete[] c;
  else
    getMotherPool()->release(c);
}

//____________________________________________________________________
void LWPool::forceCleanupMotherPool()
{
  getMotherPool()->erase();
}
