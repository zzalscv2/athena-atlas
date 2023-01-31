/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class LWPools                             //
//                                                            //
//  Description: ...                                          //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2009                                 //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef LWPOOLS_H
#define LWPOOLS_H

#include "LWPoolSelector.h"
#include "LWPool.h"
#include "CxxUtils/checker_macros.h"
#include <cassert>
#include <atomic>

#define MP_NEW(Class) new(LWPools::acquire(sizeof(Class))) Class
#define MP_DELETE(Ptr) LWPools::deleteObject(Ptr)

class LWPools {
public:

  //The fundamental interface:
  static char * acquire(unsigned length);
  static void release(char*,unsigned length);
  static void cleanup();//Returns all acquired memory to the system.
                        //Not safe to call this concurrently with anything
                        //else.

  //Convenience:
  template<unsigned length> static char* acquire() { return acquire(length); }
  template<unsigned length> static void release(char*c) { return release(c,length); }
  template<class T, unsigned length> static T* acquire();
  template<class T, unsigned length> static void release(T*);
  template<class T> static T * acquire(unsigned length);
  template<class T> static void release(T*,unsigned length);

  //For MP_DELETE (to ensure the destructor gets called):
  template<class T> static void deleteObject(T*);

  //statistics:
  static long long getTotalPoolMemAllocated();
  static long long getTotalPoolMemUsed();

private:
  LWPools();
  ~LWPools();
private:
  class PoolList;
  static PoolList s_pools ATLAS_THREAD_SAFE;
  static std::atomic<long long> s_bytesDynAlloc;
  static LWPool * initPool(unsigned poolIndex,unsigned length);
  static LWPool * getPool(unsigned length);
  LWPools( const LWPools & );
  LWPools & operator= ( const LWPools & );
};

#include "LWPools.icc"

#endif
