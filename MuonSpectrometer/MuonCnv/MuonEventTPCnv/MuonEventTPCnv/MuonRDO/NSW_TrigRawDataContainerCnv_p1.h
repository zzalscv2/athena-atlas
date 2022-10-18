/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MUONEVENTTPCNV_NSW_TRIGRAWDATACONTAINERCNV_P1_H
#define MUONEVENTTPCNV_NSW_TRIGRAWDATACONTAINERCNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"
#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataContainer_p1.h"
#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataSegmentCnv_p1.h"

class MsgStream;

namespace Muon {
  class NSW_TrigRawDataContainerCnv_p1 : public T_AthenaPoolTPCnvBase<NSW_TrigRawDataContainer, NSW_TrigRawDataContainer_p1>
  {
    public:
      virtual NSW_TrigRawDataContainer* createTransient(const NSW_TrigRawDataContainer_p1* persCont, MsgStream& log) override final;
      virtual void persToTrans(const NSW_TrigRawDataContainer_p1* persCont, NSW_TrigRawDataContainer* transCont, MsgStream &log) override final;
      virtual void transToPers(const NSW_TrigRawDataContainer* transCont, NSW_TrigRawDataContainer_p1* persCont, MsgStream &log) override final;

    private:
      NSW_TrigRawDataSegmentCnv_p1 m_segmentCnv_p1;
  };
}

#endif
