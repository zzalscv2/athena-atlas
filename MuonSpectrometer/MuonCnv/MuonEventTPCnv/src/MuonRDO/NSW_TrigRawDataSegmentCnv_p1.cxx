/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataSegmentCnv_p1.h"

namespace Muon {
  NSW_TrigRawDataSegment* NSW_TrigRawDataSegmentCnv_p1::createTransient(const NSW_TrigRawDataSegment_p1* persObj, MsgStream &log) {
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Creating transient NSW_TrigRawDataSegment from persistent NSW_TrigRawDataSegment_p1" << endmsg;
    auto transObj = std::make_unique<NSW_TrigRawDataSegment>(persObj->m_deltaTheta, persObj->m_phiIndex, persObj->m_rIndex, persObj->m_spare,
                                                             persObj->m_lowRes, persObj->m_phiRes, persObj->m_monitor);
    for (const auto &ch : persObj->m_channels) transObj->addChannel(ch.first, ch.second);
    return (transObj.release());
  }

  void NSW_TrigRawDataSegmentCnv_p1::persToTrans(const NSW_TrigRawDataSegment_p1* persObj, NSW_TrigRawDataSegment* transObj, MsgStream &log) {
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting persistent NSW_TrigRawDataSegment_p1 to transient NSW_TrigRawDataSegment" << endmsg;
    transObj->m_deltaTheta = persObj->m_deltaTheta;
    transObj->m_phiIndex = persObj->m_phiIndex;
    transObj->m_rIndex = persObj->m_rIndex;
    transObj->m_spare = persObj->m_spare;
    transObj->m_lowRes = persObj->m_lowRes;
    transObj->m_phiRes = persObj->m_phiRes;
    transObj->m_monitor = persObj->m_monitor;
    transObj->m_channels = persObj->m_channels;
    for (const auto &ch : persObj->m_channels) transObj->addChannel(ch.first, ch.second);
  }

  void NSW_TrigRawDataSegmentCnv_p1::transToPers(const NSW_TrigRawDataSegment* transObj, NSW_TrigRawDataSegment_p1* persObj, MsgStream &log)
  {
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting transient NSW_TrigRawDataSegment to persistent NSW_TrigRawDataSegment_p1" << endmsg;
    persObj->m_deltaTheta = transObj->m_deltaTheta;
    persObj->m_phiIndex = transObj->m_phiIndex;
    persObj->m_rIndex = transObj->m_rIndex;
    persObj->m_spare = transObj->m_spare;
    persObj->m_lowRes = transObj->m_lowRes;
    persObj->m_phiRes = transObj->m_phiRes;
    persObj->m_monitor = transObj->m_monitor;
    persObj->m_channels = transObj->m_channels;
    for (const auto &ch : transObj->m_channels) persObj->addChannel(ch.first, ch.second);
  }
}
