/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1EVENTTPCNV_MuCTPIL1TopoCandidateCNV_P1_H
#define TRIGT1EVENTTPCNV_MuCTPIL1TopoCandidateCNV_P1_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// TrigT1 inlcude(s):
#include "TrigT1Interfaces/MuCTPIL1TopoCandidate.h"

// Local include(s):
#include "TrigT1EventTPCnv/MuCTPIL1TopoCandidate_p1.h"

/**
 *   @short T/P converter for MuCTPIL1TopoCandidate_p1
 *
 *          This converter is used to transfer the data between the
 *          transient MuCTPIL1TopoCandidate and persistent MuCTPIL1TopoCandidate_p1 objects.
 *
 *   @author Anil Sonay
 */
class MuCTPIL1TopoCandidateCnv_p1 : public T_AthenaPoolTPCnvBase< LVL1::MuCTPIL1TopoCandidate, MuCTPIL1TopoCandidate_p1 > {

public:

  virtual void persToTrans( const MuCTPIL1TopoCandidate_p1* persObj, LVL1::MuCTPIL1TopoCandidate* transObj,
			    MsgStream& log ) override;
  virtual void transToPers( const LVL1::MuCTPIL1TopoCandidate* transObj, MuCTPIL1TopoCandidate_p1* persObj,
			    MsgStream& log ) override;

}; // class MuCTPIL1TopoCandidateCnv_p1

#endif // TRIGT1EVENTTPCNV_MuCTPIL1TopoCandidateCNV_P1_H
