///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ZDC_SimFiberHit_Collection_p1.h 
// Persistent represenation of a ZDC_SimFiberHit_Collection
/////////////////////////////////////////////////////////////////// 
#ifndef ZDC_SIMFIBERHIT_COLLECTION_P1_H
#define ZDC_SIMFIBERHIT_COLLECTION_P1_H

#include "ZdcEventTPCnv/ZDC_SimFiberHit_p1.h"
#include <vector>

class ZDC_SimFiberHit_Collection_p1 : public std::vector<ZDC_SimFiberHit_p1>
{
public:
    /// typedefs
    typedef std::vector<ZDC_SimFiberHit_p1> FiberHitVector;
    typedef FiberHitVector::const_iterator  const_iterator;
    typedef FiberHitVector::iterator        iterator;

    /// Default constructor
    ZDC_SimFiberHit_Collection_p1 () {}
      
private:
};

#endif
