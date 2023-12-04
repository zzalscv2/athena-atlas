/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   CscClusterOnTrackCnv_p2.cxx
//
//-----------------------------------------------------------------------------

#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonEventTPCnv/MuonRIO_OnTrack/CscClusterOnTrackCnv_p2.h"
#include "TrkEventTPCnv/helpers/EigenHelpers.h"


void CscClusterOnTrackCnv_p2::
persToTrans( const Muon::CscClusterOnTrack_p2 *persObj,
  Muon::CscClusterOnTrack *transObj, MsgStream &log )
{
  ElementLinkToIDC_CSC_Container rio;
  m_elCnv.persToTrans(&persObj->m_prdLink,&rio,log);

  Trk::LocalParameters localParams;
  fillTransFromPStore( &m_localParCnv, persObj->m_localParams, &localParams, log );

  Trk::ErrorMatrix dummy;
  Amg::MatrixX localCovariance;
  fillTransFromPStore( &m_errorMxCnv, persObj->m_localErrMat, &dummy, log );
  EigenHelpers::vectorToEigenMatrix(dummy.values, localCovariance, "CscClusterOnTrackCnv_p2");

  *transObj = Muon::CscClusterOnTrack (rio,
                                       std::move(localParams),
                                       std::move(localCovariance),
                                       Identifier(persObj->m_id),
                                       nullptr, // detEL
                                       persObj->m_positionAlongStrip,
                                       static_cast<Muon::CscClusterStatus>((persObj->m_status)&0xFF), // First 8 bits reserved for ClusterStatus.
                                       static_cast<Muon::CscTimeStatus>((persObj->m_status)>>8),
                                       persObj->m_time);

  m_eventCnvTool->recreateRIO_OnTrack(transObj);
  if (transObj->detectorElement()==nullptr)
    log << MSG::WARNING<<"Unable to reset DetEl for this RIO_OnTrack, "
    << "probably because of a problem with the Identifier/IdentifierHash : ("
    << transObj->identify()<<"/"<<transObj->idDE()<<endmsg;
}


void CscClusterOnTrackCnv_p2::
transToPers( const Muon::CscClusterOnTrack *transObj,
  Muon::CscClusterOnTrack_p2 *persObj, MsgStream &log )
{
 // Prepare ELs
  Trk::IEventCnvSuperTool::ELKey_t key;
  Trk::IEventCnvSuperTool::ELIndex_t index;
  m_eventCnvTool->prepareRIO_OnTrackLink(transObj, key, index);
  ElementLinkToIDC_CSC_Container eltmp (key, index);
  m_elCnv.transToPers(&eltmp, &persObj->m_prdLink,log);

  persObj->m_id = transObj->identify().get_identifier32().get_compact();
  persObj->m_localParams = toPersistent( &m_localParCnv, &transObj->localParameters(), log );
  // persObj->m_localErrMat = toPersistent( &m_errorMxCnv, &transObj->m_localErrMat, log );
  Trk::ErrorMatrix pMat;
  EigenHelpers::eigenMatrixToVector(pMat.values, transObj->localCovariance(), "CscClusterOnTrackCnv_p2");
  persObj->m_localErrMat = toPersistent( &m_errorMxCnv, &pMat, log );

  persObj->m_status         = (transObj->timeStatus()<<8); // First 8 bits reserved for ClusterStatus.
  persObj->m_status         += transObj->status();
  persObj->m_positionAlongStrip = transObj->positionAlongStrip();
  persObj->m_time = transObj->time();
}


