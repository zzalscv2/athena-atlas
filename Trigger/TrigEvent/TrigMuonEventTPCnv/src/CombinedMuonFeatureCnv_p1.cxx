/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#define private public
#define protected public
#include "TrigMuonEvent/CombinedMuonFeature.h"
#include "TrigMuonEventTPCnv/CombinedMuonFeature_p1.h"
#undef private
#undef protected
 
#include "TrigMuonEventTPCnv/CombinedMuonFeatureCnv_p1.h"
 
//-----------------------------------------------------------------------------
// Persistent to transient
//-----------------------------------------------------------------------------
void CombinedMuonFeatureCnv_p1::persToTrans( const CombinedMuonFeature_p1 *persObj,
					     CombinedMuonFeature    *transObj,
					     MsgStream       &log )
{
  log << MSG::DEBUG << "CombinedMuonFeatureCnv_p1::persToTrans called " << endreq;

  transObj->m_pt          = persObj->m_pt;
  transObj->m_sigma_pt    = persObj->m_sigma_pt;
  //need to get the ElementLink from the pointer...
  //  transObj->m_muFastTrack = createTransFromPStore( &m_mfCnv, persObj->m_muFastTrack, log );
  //  transObj->m_IDTrack = createTransFromPStore( &m_IDTkCnv, persObj->m_IDTrack, log );
     
}
 
//-----------------------------------------------------------------------------
// Transient to persistent
//-----------------------------------------------------------------------------
void CombinedMuonFeatureCnv_p1::transToPers( const CombinedMuonFeature    */*transObj*/,
					     CombinedMuonFeature_p1 */*persObj*/,
					     MsgStream       &log )
{
  log << MSG::WARNING << "CombinedMuonFeatureCnv_p1::transToPers called but CombinedMuonFeatureCnv_p2 exists!! " << endreq;

  //persObj->m_pt           = transObj->m_pt;
  //persObj->m_sigma_pt     = transObj->m_sigma_pt;
  //persObj->m_muFastTrack  = toPersistent( &m_mfCnv, transObj->m_muFastTrack, log );
  //persObj->m_IDTrack  = toPersistent( &m_IDTkCnv, transObj->m_IDTrack, log );
    
}
