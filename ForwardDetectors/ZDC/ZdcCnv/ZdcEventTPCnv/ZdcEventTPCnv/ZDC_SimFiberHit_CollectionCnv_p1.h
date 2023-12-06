/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SimFiberHit_CollectionCnv_p1_H
#define ZDC_SimFiberHit_CollectionCnv_p1_H

#include <vector>

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHit_p1.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHit_Collection_p1.h"

class MsgStream;

class ZDC_SimFiberHit_CollectionCnv_p1: public T_AthenaPoolTPCnvConstBase<ZDC_SimFiberHit_Collection, ZDC_SimFiberHit_Collection_p1> {
  
public:
  using base_class::persToTrans;
  using base_class::transToPers;

  ZDC_SimFiberHit_CollectionCnv_p1() {}
  
  virtual void persToTrans(const ZDC_SimFiberHit_Collection_p1* persObj, ZDC_SimFiberHit_Collection* transObj, MsgStream& log) const override;
  virtual void transToPers(const ZDC_SimFiberHit_Collection*   transObj, ZDC_SimFiberHit_Collection_p1* persObj, MsgStream& log) const override;
};


template<>
class T_TPCnv<ZDC_SimFiberHit_Collection, ZDC_SimFiberHit_Collection_p1>
  : public ZDC_SimFiberHit_CollectionCnv_p1
{
public:
};

#endif

