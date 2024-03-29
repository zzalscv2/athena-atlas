/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SIMPIXELHIT_COLLECTION
#define ZDC_SIMPIXELHIT_COLLECTION

#include "ZDC_SimPixelHit.h"
#include "HitManagement/AtlasHitsVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <string>

typedef AtlasHitsVector<ZDC_SimPixelHit> ZDC_SimPixelHit_Collection1;

class ZDC_SimPixelHit_Collection: public ZDC_SimPixelHit_Collection1
{

 public :

  ZDC_SimPixelHit_Collection(const std::string& name="ZDC_SimPixelHit_Collection") : ZDC_SimPixelHit_Collection1(name.c_str()) {}
};

typedef  ZDC_SimPixelHit_Collection::iterator       ZDC_SimPixelHit_Iterator;
typedef  ZDC_SimPixelHit_Collection::const_iterator ZDC_SimPixelHit_ConstIterator;

#ifndef __CINT__
  CLASS_DEF(ZDC_SimPixelHit_Collection, 1300880191, 1)
#endif

#endif
