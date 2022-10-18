/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataContainerCnv_p1.h"

namespace Muon
{
  NSW_TrigRawDataContainer* NSW_TrigRawDataContainerCnv_p1::createTransient(const NSW_TrigRawDataContainer_p1* persCont, MsgStream& log) {
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Creating transient NSW_TrigRawDataContainer from persistent with size " << persCont->size() << endmsg;
    auto transCont = std::make_unique<Muon::NSW_TrigRawDataContainer>();
    persToTrans(persCont, transCont.get(), log);
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Created transient NSW_TrigRawDataContainer with " << transCont->size() << " entries" << endmsg;
    return(transCont.release());
  }

  void NSW_TrigRawDataContainerCnv_p1::persToTrans(const NSW_TrigRawDataContainer_p1* persCont, NSW_TrigRawDataContainer* transCont, MsgStream &log) {
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting persistent NSW_TrigRawDataContainer_p1 to transient NSW_TrigRawDataContainer" << endmsg;

    for (const auto &raw : *persCont) {
      auto rawData = std::make_unique<NSW_TrigRawData>(raw.m_sectorId, raw.m_sectorSide, raw.m_bcId);
      if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Created empty NSW_TrigRawData contaier, to be filled with " << raw.size() << " segments" << endmsg;
      for (unsigned int i = 0; i < raw.size(); ++i) {
        const NSW_TrigRawDataSegment_p1* persRawSegObj = &( raw[i] );
        auto transObj = std::make_unique<NSW_TrigRawDataSegment>();
        m_segmentCnv_p1.persToTrans(persRawSegObj, transObj.get(), log);
        rawData->push_back(std::move(transObj));
      }
      transCont->push_back(std::move(rawData));
    }
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Created transient NSW_TrigRawDataContainer representation with " << transCont->size() << " entries" << endmsg;
  }

  void NSW_TrigRawDataContainerCnv_p1::transToPers(const NSW_TrigRawDataContainer* transCont, NSW_TrigRawDataContainer_p1* persCont, MsgStream &log) {
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting transient NSW_TrigRawDataContainer to persistent NSW_TrigRawDataContainer_p1" << endmsg;

    for (const auto &raw : *transCont) {
      auto persRawData = std::make_unique<NSW_TrigRawData_p1>();
      persRawData->m_sectorId = raw->sectorId();
      persRawData->m_sectorSide = raw->sectorSide();
      persRawData->m_bcId = raw->bcId();
      for (const auto &segment : *raw) {
        auto persRawSegm = std::make_unique<NSW_TrigRawDataSegment_p1>();
        m_segmentCnv_p1.transToPers(segment, persRawSegm.get(), log);
        persRawData->push_back(*persRawSegm);
      }
      persCont->push_back(*persRawData);
    }
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Created persistent NSW_TrigRawDataContainer representation with " << persCont->size() << " entries" << endmsg;
  }
}
