/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#define private public
#define protected public
#include "TrigParticle/TrigEFBjet.h"
#include "TrigParticleTPCnv/TrigEFBjet_p1.h"
#undef private
#undef protected
 
#include "TrigParticleTPCnv/TrigEFBjetCnv_p1.h"


//* Persistent to transient *//
void TrigEFBjetCnv_p1::persToTrans(const TrigEFBjet_p1 *persObj, TrigEFBjet *transObj, MsgStream &log) {

  log << MSG::DEBUG << "TrigEFBjetCnv_p1::persToTrans called " << endreq;

  transObj->m_valid  = persObj->m_valid;
  transObj->m_roiID  = persObj->m_roiID;
  transObj->m_eta    = persObj->m_eta;
  transObj->m_phi    = persObj->m_phi;
  transObj->m_prmVtx = persObj->m_prmVtx;
  transObj->m_xcomb  = persObj->m_xcomb;
  transObj->m_xIP1d  = persObj->m_xz0;
  transObj->m_xIP2d  = persObj->m_xd0;
  transObj->m_xIP3d  = persObj->m_x2d;
  transObj->m_xSv    = persObj->m_x3d;
  transObj->m_xmvtx  = persObj->m_xmvtx;
  transObj->m_xevtx  = persObj->m_xevtx;
  transObj->m_xnvtx  = persObj->m_xnvtx;

  transObj->m_xChi2  = -9.9;

}
 
//* Transient to persistent *//
void TrigEFBjetCnv_p1::transToPers(const TrigEFBjet *transObj, TrigEFBjet_p1 *persObj, MsgStream &log) {

  log << MSG::DEBUG << "TrigEFBjetCnv_p1::transToPers called " << endreq;
  
  persObj->m_valid  = transObj->m_valid;
  persObj->m_roiID  = transObj->m_roiID;
  persObj->m_eta    = transObj->m_eta;
  persObj->m_phi    = transObj->m_phi;
  persObj->m_prmVtx = transObj->m_prmVtx;
  persObj->m_xcomb  = transObj->m_xcomb;
  persObj->m_xmvtx  = transObj->m_xmvtx;
  persObj->m_xevtx  = transObj->m_xevtx;
  persObj->m_xnvtx  = transObj->m_xnvtx;

  persObj->m_x2d    = transObj->m_xIP1d;
  persObj->m_xd0    = transObj->m_xIP2d;
  persObj->m_xz0    = transObj->m_xIP3d;
  persObj->m_x3d    = transObj->m_xSv;
 
}
