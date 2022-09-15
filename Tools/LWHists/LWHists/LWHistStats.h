/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class LWHistStats                         //
//                                                            //
//  Description: Access to various statistics                 //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: October 2009                             //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef LWHISTSTATS_H
#define LWHISTSTATS_H

#include <atomic>

class LWHistStats {
public:
  static long nActiveLWHists() { return s_nActiveHists; }

  static long long getTotalPoolMemAllocated();
  static long long getTotalPoolMemUsed();

  //Not more info, but for convenience:
  static long long getMemUnusedButAllocatedInPools();
  static double poolWasteFraction();//0: perfect, 1: very bad

private:
  LWHistStats(){}
  ~LWHistStats(){}
  static std::atomic<long> s_nActiveHists;
  friend class LWHist;
};

#endif
