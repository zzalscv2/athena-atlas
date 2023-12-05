/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SIMFIBERHITCNV_P1_H
#define ZDC_SIMFIBERHITCNV_P1_H

#include "ZDC_SimEvent/ZDC_SimFiberHit.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHit_p1.h"

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

class MsgStream;

class ZDC_SimFiberHitCnv_p1: public T_AthenaPoolTPCnvConstBase<ZDC_SimFiberHit, ZDC_SimFiberHit_p1> {

 public:
  using base_class::persToTrans;
  using base_class::transToPers;

  ZDC_SimFiberHitCnv_p1() {}
  
  virtual void persToTrans(const ZDC_SimFiberHit_p1* persObj, ZDC_SimFiberHit*	 transObj, MsgStream& log) const;
  virtual void transToPers(const ZDC_SimFiberHit*   transObj, ZDC_SimFiberHit_p1* persObj, MsgStream& log) const;


};


#endif
