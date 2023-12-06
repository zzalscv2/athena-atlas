/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SIMFIBERHIT_COLLECTION
#define ZDC_SIMFIBERHIT_COLLECTION

#include "ZDC_SimFiberHit.h"
#include "HitManagement/AtlasHitsVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <string>

typedef AtlasHitsVector<ZDC_SimFiberHit> ZDC_SimFiberHit_Collection1;

class ZDC_SimFiberHit_Collection: public ZDC_SimFiberHit_Collection1
{

 public :

  ZDC_SimFiberHit_Collection(const std::string& name="ZDC_SimFiberHit_Collection") : ZDC_SimFiberHit_Collection1(name.c_str()) {}
};

typedef  ZDC_SimFiberHit_Collection::iterator       ZDC_SimFiberHit_Iterator;
typedef  ZDC_SimFiberHit_Collection::const_iterator ZDC_SimFiberHit_ConstIterator;

#ifndef __CINT__
  CLASS_DEF(ZDC_SimFiberHit_Collection, 1264349957, 1)
#endif

#endif
