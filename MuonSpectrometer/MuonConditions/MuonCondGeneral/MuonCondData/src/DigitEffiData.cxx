/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondData/DigitEffiData.h"

DigitEffiData::DigitEffiData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"DigitEffiData"},
    m_idHelperSvc{idHelperSvc} {}

double DigitEffiData::getEfficiency(const Identifier& channelId) const {
    EffiMap::const_iterator effi_itr = m_effiData.find(m_idHelperSvc->gasGapId(channelId));
    if (effi_itr != m_effiData.end()) {
        ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(channelId)
                        <<" has  an efficiency of "<<effi_itr->second);
        return effi_itr->second;
    }
    ATH_MSG_WARNING("Efficiency of channel "<<m_idHelperSvc->toString(channelId)<<" is unknown. Return 1.");
    return 1.;
}

StatusCode DigitEffiData::setEfficiency(const Identifier& channelId, const double effi){
    const Identifier gasGapId = m_idHelperSvc->gasGapId(channelId);
    auto insert_itr = m_effiData.insert(std::make_pair(gasGapId, effi));
    if (!insert_itr.second) {
        ATH_MSG_ERROR("An efficiency for gasGap "<<m_idHelperSvc->toStringGasGap(gasGapId)
        <<" has already been stored "<<m_effiData[gasGapId]<<" vs. "<<effi);
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
    