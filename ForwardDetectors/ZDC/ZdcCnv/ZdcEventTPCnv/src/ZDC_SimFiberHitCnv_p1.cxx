/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_SimEvent/ZDC_SimFiberHit.h"
#include "Identifier/Identifier.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHitCnv_p1.h"

void ZDC_SimFiberHitCnv_p1::persToTrans(const ZDC_SimFiberHit_p1* persObj, ZDC_SimFiberHit* transObj, MsgStream& log) const {

  log << MSG::DEBUG << " In ZDC_SimFiberHitCnv_p1::persToTrans " << endmsg;

  *transObj = ZDC_SimFiberHit (persObj->m_ID,
                               persObj->m_Nphotons,
                               persObj->m_Edep);
}

void ZDC_SimFiberHitCnv_p1::transToPers(const ZDC_SimFiberHit* transObj, ZDC_SimFiberHit_p1* persObj, MsgStream& log) const {

  log << MSG::DEBUG << " In ZDC_SimFiberHitCnv_p1::transToPers " << endmsg;

  persObj->m_ID = transObj->getID();
  persObj->m_Nphotons = transObj->getNPhotons();
  persObj->m_Edep = transObj->getEdep();
}
