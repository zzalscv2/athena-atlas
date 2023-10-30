/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/TgcDigitThresholdData.h"

TgcDigitThresholdData::TgcDigitThresholdData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"TgcDigitThresholdData"},
    m_idHelperSvc{idHelperSvc}{}

bool TgcDigitThresholdData::setThreshold(const Identifier& threshId, const double threshold) {
    const Identifier layerId = m_idHelperSvc->layerId(threshId);
    auto insert_itr = m_thresholds.insert(std::make_pair(layerId, threshold));
    if (!insert_itr.second) {
        ATH_MSG_ERROR("Failed to threshold "<<threshold<<" for "<<m_idHelperSvc->toString(layerId)
                     <<" as it's been set before to "<<m_thresholds[layerId]);
        return false;
    }
    ATH_MSG_DEBUG("Theshold for channel "<<m_idHelperSvc->toString(layerId)<<" successfully set to "<<threshold);
    return true;
}
double TgcDigitThresholdData::getThreshold(const Identifier& channelId) const {
    auto itr = m_thresholds.find(m_idHelperSvc->layerId(channelId));
    if (itr != m_thresholds.end()) return itr->second;
    constexpr double defaultThresh{999999.};
    ATH_MSG_WARNING("No threshold has been set for channel "<<m_idHelperSvc->toString(channelId)<<" return "<<defaultThresh);
    return defaultThresh;
}
