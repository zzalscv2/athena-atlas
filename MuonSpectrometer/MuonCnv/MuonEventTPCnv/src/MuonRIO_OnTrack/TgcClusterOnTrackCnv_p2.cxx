/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   TgcClusterOnTrackCnv_p2.cxx
//
//-----------------------------------------------------------------------------

#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "MuonEventTPCnv/MuonRIO_OnTrack/TgcClusterOnTrackCnv_p2.h"
#include "TrkEventTPCnv/helpers/EigenHelpers.h"


void TgcClusterOnTrackCnv_p2::
persToTrans( const Muon::TgcClusterOnTrack_p2 *persObj,
  Muon::TgcClusterOnTrack *transObj, MsgStream &log )
{
  Trk::LocalParameters localParams;
  fillTransFromPStore( &m_localParCnv, persObj->m_localParams, &localParams, log );

  Trk::ErrorMatrix dummy;
  Amg::MatrixX localCovariance;
  fillTransFromPStore( &m_errorMxCnv, persObj->m_localErrMat, &dummy, log );
  EigenHelpers::vectorToEigenMatrix(dummy.values, localCovariance, "TgcClusterOnTrackCnv_p2");

  ElementLinkToIDC_TGC_Container rio;
  m_elCnv.persToTrans(&persObj->m_prdLink,&rio,log);

  *transObj = Muon::TgcClusterOnTrack (rio,
                                       std::move(localParams),
                                       std::move(localCovariance),
                                       Identifier(persObj->m_id),
                                       nullptr,
                                       persObj->m_positionAlongStrip);


  // Attempt to call supertool to fill in detElements
  m_eventCnvTool->recreateRIO_OnTrack(transObj);
  if (!transObj->detectorElement())
    log << MSG::WARNING<<"Unable to reset DetEl for this RIO_OnTrack, "
        << "probably because of a problem with the Identifier/IdentifierHash : ("
        << transObj->identify()<<"/"<<transObj->idDE()<<endmsg;
 }


void TgcClusterOnTrackCnv_p2::
transToPers( const Muon::TgcClusterOnTrack *transObj,
  Muon::TgcClusterOnTrack_p2 *persObj, MsgStream &log )
{
  // Prepare ELs
  Trk::IEventCnvSuperTool::ELKey_t key;
  Trk::IEventCnvSuperTool::ELIndex_t index;
  m_eventCnvTool->prepareRIO_OnTrackLink(transObj, key, index);
  ElementLinkToIDC_TGC_Container eltmp (key, index);

  m_elCnv.transToPers(&eltmp, &persObj->m_prdLink,log);
  persObj->m_positionAlongStrip = transObj->positionAlongStrip();

  persObj->m_id = transObj->identify().get_identifier32().get_compact();
  persObj->m_localParams = toPersistent( &m_localParCnv, &transObj->localParameters(), log );
  Trk::ErrorMatrix pMat;
  EigenHelpers::eigenMatrixToVector(pMat.values, transObj->localCovariance(), "TgcClusterOnTrackCnv_p2");
  persObj->m_localErrMat = toPersistent( &m_errorMxCnv, &pMat, log );
}


