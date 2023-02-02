/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1EVENTTPCNV_MuCTPIL1TopoCNV_P1_H
#define TRIGT1EVENTTPCNV_MuCTPIL1TopoCNV_P1_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// TrigT1 inlcude(s):
#include "TrigT1Interfaces/MuCTPIL1Topo.h"

// Local include(s):
#include "TrigT1EventTPCnv/MuCTPIL1Topo_p1.h"

/**
 *   @short T/P converter for MuCTPIL1Topo_p1
 *
 *          This converter is used to transfer the data between the
 *          transient MuCTPIL1Topo and persistent MuCTPIL1Topo_p1 objects.
 *
 *   @author Anil Sonay
 */
class MuCTPIL1TopoCnv_p1 : public T_AthenaPoolTPCnvBase< LVL1::MuCTPIL1Topo, MuCTPIL1Topo_p1 > {

public:

  virtual void persToTrans( const MuCTPIL1Topo_p1* persObj, LVL1::MuCTPIL1Topo* transObj,
			    MsgStream& log ) override;
  virtual void transToPers( const LVL1::MuCTPIL1Topo* transObj, MuCTPIL1Topo_p1* persObj,
			    MsgStream& log ) override;

}; // class MuCTPIL1TopoCnv_p1

#endif // TRIGT1EVENTTPCNV_MuCTPIL1TopoCNV_P1_H
