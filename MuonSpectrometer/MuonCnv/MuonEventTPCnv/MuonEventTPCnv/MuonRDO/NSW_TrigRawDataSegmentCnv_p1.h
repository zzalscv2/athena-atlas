/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MUONEVENTTPCNV_NSW_TRIGRAWDATASEGMENTCNV_P1_H
#define MUONEVENTTPCNV_NSW_TRIGRAWDATASEGMENTCNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "MuonRDO/NSW_TrigRawDataSegment.h"
#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataSegment_p1.h"

class MsgStream;

namespace Muon {
  class NSW_TrigRawDataSegmentCnv_p1 : public T_AthenaPoolTPCnvBase<NSW_TrigRawDataSegment, NSW_TrigRawDataSegment_p1> {
    public:
      virtual NSW_TrigRawDataSegment* createTransient(const NSW_TrigRawDataSegment_p1* persObj, MsgStream &log) override final;
      virtual void persToTrans(const NSW_TrigRawDataSegment_p1* persObj, NSW_TrigRawDataSegment* transObj, MsgStream &log) override final;
      virtual void transToPers(const NSW_TrigRawDataSegment* transObj, NSW_TrigRawDataSegment_p1* persObj, MsgStream &log) override final;
  };
}

#endif
