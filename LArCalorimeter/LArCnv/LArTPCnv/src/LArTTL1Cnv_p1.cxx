/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArTTL1.h"
#include "LArTPCnv/LArTTL1Cnv_p1.h"
#include "Identifier/HWIdentifier.h"

void
LArTTL1Cnv_p1::persToTrans(const LArTTL1_p1* persObj, LArTTL1* transObj, MsgStream &/*log*/) const
{
  HWIdentifier ttChannel;
  *transObj = LArTTL1 (ttChannel,
                        persObj->m_offlineId,
                        persObj->m_samples);
}


void
LArTTL1Cnv_p1::transToPers(const LArTTL1* transObj, LArTTL1_p1* persObj, MsgStream &/*log*/) const
{
  persObj->m_offlineId = transObj->ttOfflineID();
  persObj->m_samples = transObj->samples();
}
