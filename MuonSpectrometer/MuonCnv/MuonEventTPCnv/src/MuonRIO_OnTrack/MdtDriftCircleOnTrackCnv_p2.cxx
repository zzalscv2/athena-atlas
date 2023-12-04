/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   MdtDriftCircleOnTrackCnv_p2.cxx
//
//-----------------------------------------------------------------------------

#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonDriftCircleErrorStrategy.h"
#include "MuonEventTPCnv/MuonRIO_OnTrack/MdtDriftCircleOnTrackCnv_p2.h"
#include "TrkEventTPCnv/helpers/EigenHelpers.h"

#include <iostream>

void MdtDriftCircleOnTrackCnv_p2::
persToTrans( const Muon::MdtDriftCircleOnTrack_p2 *persObj,
	     Muon::MdtDriftCircleOnTrack *transObj, MsgStream &log )
{
  ElementLinkToIDC_MDT_Container rio;
  m_elCnv.persToTrans(&persObj->m_prdLink,&rio,log);

  Trk::LocalParameters localParams;
  fillTransFromPStore( &m_localParCnv, persObj->m_localParams, &localParams, log );

  Trk::ErrorMatrix dummy;
  Amg::MatrixX localCovariance;
  fillTransFromPStore( &m_errorMxCnv, persObj->m_localErrMat, &dummy, log );
  EigenHelpers::vectorToEigenMatrix(dummy.values, localCovariance, "MdtDriftCircleOnTrackCnv_p2");



  *transObj = Muon::MdtDriftCircleOnTrack (rio,
                                           std::move(localParams),
                                           std::move(localCovariance),
                                           Identifier(persObj->m_id),
                                           nullptr, // detEl,
                                           persObj->m_driftTime,
                                           static_cast<Trk::DriftCircleStatus>( persObj->m_status ),
                                           persObj->m_positionAlongWire,
                                           persObj->m_localAngle,
                                           Muon::MuonDriftCircleErrorStrategy (persObj->m_muonDriftCircleErrorStrategy));
  // Attempt to call supertool to fill in detElements
  m_eventCnvTool->recreateRIO_OnTrack(transObj);

}

void MdtDriftCircleOnTrackCnv_p2::
transToPers( const Muon::MdtDriftCircleOnTrack    *transObj,
	     Muon::MdtDriftCircleOnTrack_p2 *persObj, MsgStream &log)
{
  // Prepare ELs
  Trk::IEventCnvSuperTool::ELKey_t key;
  Trk::IEventCnvSuperTool::ELIndex_t index;
  m_eventCnvTool->prepareRIO_OnTrackLink(transObj, key, index);
  ElementLinkToIDC_MDT_Container eltmp (key, index);
  m_elCnv.transToPers(&eltmp, &persObj->m_prdLink,log);

  persObj->m_id = transObj->identify().get_identifier32().get_compact();
  persObj->m_localParams = toPersistent( &m_localParCnv, &transObj->localParameters(), log );
  Trk::ErrorMatrix pMat;
  EigenHelpers::eigenMatrixToVector(pMat.values, transObj->localCovariance(), "MdtDriftCircleOnTrackCnv_p2");
  persObj->m_localErrMat = toPersistent( &m_errorMxCnv, &pMat, log );


  persObj->m_status    = static_cast<unsigned int>( transObj->status() );
  persObj->m_localAngle            = transObj->localAngle() ;
  persObj->m_positionAlongWire     = transObj->positionAlongWire() ;
  persObj->m_driftTime             = transObj->driftTime() ;
  persObj->m_muonDriftCircleErrorStrategy = transObj->errorStrategy().getBits().to_ulong();
}

