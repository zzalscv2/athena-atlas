/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTgcPREPDATACNV_p3_H
#define sTgcPREPDATACNV_p3_H

//-----------------------------------------------------------------------------
//
// file:   sTgcPrepDataCnv_p3.h
//
//-----------------------------------------------------------------------------
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "MuonPrepRawData/sTgcPrepData.h"
#include "sTgcPrepData_p3.h"

class MsgStream;

class sTgcPrepDataCnv_p3
   : public T_AthenaPoolTPPolyCnvBase< Trk::PrepRawData, Muon::sTgcPrepData, Muon::sTgcPrepData_p3 >
{
public:
  sTgcPrepDataCnv_p3() = default;
  
  Muon::sTgcPrepData
  createsTgcPrepData( const Muon::sTgcPrepData_p3 *persObj,
                      const Identifier clusId,
                      const MuonGM::sTgcReadoutElement* m_detEl,
                      MsgStream & log );
  
  void persToTrans( const Muon::sTgcPrepData_p3 *persObj,
                    Muon::sTgcPrepData    *transObj,
                    MsgStream                &log );
  void transToPers( const Muon::sTgcPrepData    *transObj,
                    Muon::sTgcPrepData_p3 *persObj,
                    MsgStream                &log );
protected:        
};

#endif
