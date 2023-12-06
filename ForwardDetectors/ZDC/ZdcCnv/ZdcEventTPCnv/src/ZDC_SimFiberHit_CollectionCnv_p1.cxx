/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcEventTPCnv/ZDC_SimFiberHitCnv_p1.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHit_CollectionCnv_p1.h"
#include "GaudiKernel/MsgStream.h"

static const ZDC_SimFiberHitCnv_p1 ZdcFiberHitConv;

void ZDC_SimFiberHit_CollectionCnv_p1::transToPers(const ZDC_SimFiberHit_Collection* transObj, ZDC_SimFiberHit_Collection_p1* persObj, MsgStream& log) const {
  
  persObj->resize(transObj->size());
  
  for (unsigned int i=0; i<transObj->size(); ++i) {
    
    ZDC_SimFiberHit_p1& fiberhit = (*persObj)[i];
    const ZDC_SimFiberHit& fiberhit_t = (*transObj)[i];
    
    ZdcFiberHitConv.transToPers(&fiberhit_t, &fiberhit, log);
  }    
}

void ZDC_SimFiberHit_CollectionCnv_p1::persToTrans(const ZDC_SimFiberHit_Collection_p1* persObj, ZDC_SimFiberHit_Collection* transObj, MsgStream& log) const {

  //log << MSG::INFO << " size = " << persObj->size() << endmsg;

  transObj->reserve(persObj->size());
  
  for (unsigned int i=0; i<persObj->size(); ++i) {
    
    //log << MSG::INFO << " i = " << i << endmsg;
    const ZDC_SimFiberHit_p1* fiberhit = &((*persObj)[i]);

    std::unique_ptr<ZDC_SimFiberHit> ptr (ZdcFiberHitConv.createTransientConst(fiberhit, log));
    transObj->push_back(*ptr);
  }    
}
