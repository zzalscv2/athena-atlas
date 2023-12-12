/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMPREPDATACNV_p2_H
#define MMPREPDATACNV_p2_H

//-----------------------------------------------------------------------------
//
// file:   MMPrepDataCnv_p2.h
//
//-----------------------------------------------------------------------------
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "MuonPrepRawData/MMPrepData.h"
#include "MMPrepData_p2.h"

class MsgStream;

class MMPrepDataCnv_p2
   : public T_AthenaPoolTPPolyCnvBase< Trk::PrepRawData, Muon::MMPrepData, Muon::MMPrepData_p2 >
{
    public:
  MMPrepDataCnv_p2() {}

  static Muon::MMPrepData
  createMMPrepData ( const Muon::MMPrepData_p2 *persObj, 
		     Identifier clusId,
                     const MuonGM::MMReadoutElement* detEl,
                     MsgStream & log );

  void persToTrans( const Muon::MMPrepData_p2 *persObj,
                    Muon::MMPrepData    *transObj,
                    MsgStream                &log );
  void transToPers( const Muon::MMPrepData    *transObj,
                    Muon::MMPrepData_p2 *persObj,
                    MsgStream                &log );
protected:        
};

#endif
