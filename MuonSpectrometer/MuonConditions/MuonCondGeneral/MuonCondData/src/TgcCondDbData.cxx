/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/TgcCondDbData.h"

TgcCondDbData::TgcCondDbData(const Muon::IMuonIdHelperSvc* idHelperSvc):
        m_idHelperSvc{idHelperSvc}{}

void TgcCondDbData::setDeadGasGap(const Identifier& Id) { 
    m_cachedDeadStationsId.insert(m_idHelperSvc->gasGapId(Id)); 
}
bool TgcCondDbData::isGood(const Identifier & Id) const { 
    return !m_cachedDeadStationsId.count(m_idHelperSvc->gasGapId(Id)); 
}
