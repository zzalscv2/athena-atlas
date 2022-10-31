/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/NswCalibDbTimeChargeData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "Identifier/Identifier.h"
#include <TString.h>

std::ostream& operator<<(std::ostream& ostr, const NswCalibDbTimeChargeData::CalibConstants& obj) {
    ostr<<" slope: "<<std::setprecision(15)<<obj.slope;//<<" pm "<<std::setprecision(15)<<obj.slopeError;
    ostr<<" intercept: "<<std::setprecision(15)<<obj.intercept;//<<" pm "<<std::setprecision(15)<<obj.interceptError;
    return ostr;
}

// general functions ---------------------------------
NswCalibDbTimeChargeData::NswCalibDbTimeChargeData(const MmIdHelper& mmIdHelper, const sTgcIdHelper& stgcIdHelper):
    m_mmIdHelper(mmIdHelper),
    m_stgcIdHelper(stgcIdHelper)
{
}

int NswCalibDbTimeChargeData::identToModuleIdx(const Identifier& chan_id) const{
    if (m_mmIdHelper.is_mm(chan_id)) {
        const int stEta = m_mmIdHelper.stationEta(chan_id);        
        const int steta_index = stEta + mmGMRanges::NMMcStEtaOffset - (stEta > 0);
        const int stphi_index = 2 * (m_mmIdHelper.stationPhi(chan_id) - 1) + m_mmIdHelper.isSmall(chan_id);
        const int ml_index = m_mmIdHelper.multilayer(chan_id) - 1;
        const int lay_index = m_mmIdHelper.gasGap(chan_id) -1;

        if (steta_index < 0 || steta_index >= mmGMRanges::NMMcStatEta) {
            throw std::runtime_error(Form("%s:%d - stEtaindex out of range %d 0-%d",
                                          __FILE__, __LINE__, steta_index, mmGMRanges::NMMcStatEta - 1));
        }
        if (stphi_index < 0 || stphi_index >= mmGMRanges::NMMcStatPhi) {
            throw std::runtime_error(Form("%s:%d - stPhiindex out of range %d 0-%d",
                                          __FILE__, __LINE__, stphi_index, mmGMRanges::NMMcStatPhi - 1));
        }
        if (ml_index < 0 || ml_index >= mmGMRanges::NMMcChamberLayer) {
            throw std::runtime_error(Form("%s:%d - ml_index out of range %d 0-%d",
                                          __FILE__, __LINE__, ml_index, mmGMRanges::NMMcChamberLayer - 1));
        }
        if (lay_index < 0 || lay_index >= mmGMRanges::NMMcStatLay) {
           throw std::runtime_error(Form("%s:%d - lay_index out of range %d 0-%d",
                                          __FILE__, __LINE__, lay_index, mmGMRanges::NMMcStatLay - 1));             
        }
        constexpr int C = mmGMRanges::NMMcStatLay;
        constexpr int BxC = mmGMRanges::NMMcChamberLayer * C;
        constexpr int AxBxC = mmGMRanges::NMMcStatPhi * BxC;
        const int array_idx = steta_index * AxBxC + stphi_index * BxC + ml_index*C + lay_index;
        return array_idx;
    } else if (m_stgcIdHelper.is_stgc(chan_id)) {        
        const int stEta = m_stgcIdHelper.stationEta(chan_id);
        const int steta_index = stEta + sTgcGMRanges::NsTgStEtaOffset - (stEta > 0);
        const int stphi_index = 2 * (m_stgcIdHelper.stationPhi(chan_id) - 1) +  m_stgcIdHelper.isSmall(chan_id);
        const int ml_index = m_stgcIdHelper.multilayer(chan_id) - 1;
        const int lay_index = m_stgcIdHelper.gasGap(chan_id) -1;
        if (steta_index < 0 || steta_index >= sTgcGMRanges::NsTgStatEta) {
            throw std::runtime_error(
                Form("%s:%d -- stEtaIndex out of range %d 0-%d", __FILE__, __LINE__,
                     steta_index, sTgcGMRanges::NsTgStatEta - 1));
        }
        if (stphi_index < 0 || stphi_index >= sTgcGMRanges::NsTgStatPhi) {
            throw std::runtime_error(
                Form("%s:%d -- stPhiIndex out of range %d 0-%d", __FILE__, __LINE__,
                     stphi_index, sTgcGMRanges::NsTgStatPhi - 1));
        }
        if (ml_index < 0 || ml_index >= sTgcGMRanges::NsTgChamberLayer) {
            throw std::runtime_error(Form("%s:%d - ml_index out of range %d 0-%d",
                                          __FILE__, __LINE__, ml_index, sTgcGMRanges::NsTgChamberLayer - 1));
        }
        if (lay_index < 0 || lay_index >= sTgcGMRanges::NsTgcStatLay) {
            throw std::runtime_error(Form("%s:%d - lay_index out of range %d 0-%d",
                                          __FILE__, __LINE__, lay_index, sTgcGMRanges::NsTgcStatLay - 1));
        }
        constexpr int D = sTgcGMRanges::NsTgcChannelTypes;
        constexpr int CxD = sTgcGMRanges::NsTgcStatLay*D;
        constexpr int BxCxD = sTgcGMRanges::NsTgChamberLayer*CxD;
        constexpr int AxBxCxD = sTgcGMRanges::NsTgStatPhi * BxCxD;        
        const int array_idx = steta_index * AxBxCxD + stphi_index * BxCxD + ml_index*CxD + lay_index*D  + m_stgcIdHelper.channelType(chan_id);
        return array_idx + s_NumMaxMMElements;
    }
    return -1;
 }

// setting functions ---------------------------------

// setData
void
NswCalibDbTimeChargeData::setData(CalibDataType type, const Identifier& chnlId,  CalibConstants constants) {
    ChannelCalibMap& calibMap =  type == CalibDataType::PDO ? m_pdo_data : m_tdo_data; 
    const int array_idx = identToModuleIdx(chnlId);   
    CalibModule& calib_mod = calibMap.at(array_idx);
    const unsigned int channel = (m_mmIdHelper.is_mm(chnlId) ? m_mmIdHelper.channel(chnlId) : m_stgcIdHelper.channel(chnlId)) -1;
    if (calib_mod.channels.empty()) {
        if (m_mmIdHelper.is_mm(chnlId)) {
            calib_mod.layer_id = m_mmIdHelper.channelID(chnlId, m_mmIdHelper.multilayer(chnlId), m_mmIdHelper.gasGap(chnlId), 1);
        } else if (m_stgcIdHelper.is_stgc(chnlId)) {
            calib_mod.layer_id = m_stgcIdHelper.channelID(chnlId, m_stgcIdHelper.multilayer(chnlId), m_stgcIdHelper.gasGap(chnlId), m_stgcIdHelper.channelType(chnlId), 1);
        }
    }
    if (calib_mod.channels.size() <= channel) calib_mod.channels.resize(channel +1);
    if (calib_mod.channels[channel]) {
        throw std::runtime_error("setData() -- Cannot overwrite channel");
        return;
    }    
    calib_mod.channels[channel] = std::make_unique<CalibConstants>(std::move(constants));    
}

// setZeroData
void
NswCalibDbTimeChargeData::setZero(CalibDataType type, CalibTechType tech,  CalibConstants constants) {
    ZeroCalibMap& calibMap = m_zero[tech];    
    calibMap.insert(std::make_pair(type, std::move(constants)));
}



// retrieval functions -------------------------------

// getChannelIds
std::vector<Identifier>
NswCalibDbTimeChargeData::getChannelIds(const CalibDataType type, const std::string& tech, const std::string& side) const {
    std::vector<Identifier> chnls;
    const ChannelCalibMap& calibMap =  type == CalibDataType::PDO ? m_pdo_data : m_tdo_data;    
    chnls.reserve(calibMap.size());
    for (const CalibModule& module : calibMap) {
        /// No calibration constants saved here
        if (module.channels.empty()) continue;
        if (m_mmIdHelper.is_mm(module.layer_id)) {
            if (tech == "STGC") continue;
            if (side == "A" && m_mmIdHelper.stationEta(module.layer_id) < 0) continue;
            if (side == "C" && m_mmIdHelper.stationEta(module.layer_id) > 0) continue;
            for (unsigned int chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if (!module.channels[chn -1]) continue;
                chnls.push_back(m_mmIdHelper.channelID(module.layer_id, m_mmIdHelper.multilayer(module.layer_id), m_mmIdHelper.gasGap(module.layer_id), chn ));
            }
        } else if (m_mmIdHelper.is_stgc(module.layer_id)) {
            if (tech == "MM") break;
            if (side == "A" && m_stgcIdHelper.stationEta(module.layer_id) < 0) continue;
            if (side == "C" && m_stgcIdHelper.stationEta(module.layer_id) > 0) continue;
            for (unsigned int chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if (!module.channels[chn -1]) continue;
                chnls.push_back(m_stgcIdHelper.channelID(module.layer_id, m_stgcIdHelper.multilayer(module.layer_id), 
                                        m_stgcIdHelper.gasGap(module.layer_id),  m_stgcIdHelper.channelType(module.layer_id), chn ));
            }
        }
    
    }

    return chnls;
}
const NswCalibDbTimeChargeData::CalibConstants* NswCalibDbTimeChargeData::getCalibForChannel(const CalibDataType type, const Identifier& channelId) const {
    const ChannelCalibMap& calibMap =  type == CalibDataType::PDO ? m_pdo_data : m_tdo_data;    
    const int array_idx = identToModuleIdx(channelId);
    const unsigned int channel = (m_mmIdHelper.is_mm(channelId) ? m_mmIdHelper.channel(channelId) : m_stgcIdHelper.channel(channelId)) -1;
    if (calibMap.at(array_idx).channels.size() > channel && calibMap[array_idx].channels[channel]) return calibMap[array_idx].channels[channel].get();
    // search for data for channel zero
    const CalibTechType tech = (m_stgcIdHelper.is_stgc(channelId))? CalibTechType::STGC : CalibTechType::MM;
    return getZeroCalibChannel(type, tech);        

}
const NswCalibDbTimeChargeData::CalibConstants* NswCalibDbTimeChargeData::getZeroCalibChannel(const CalibDataType type, const CalibTechType tech) const{   
    std::map<CalibTechType, ZeroCalibMap>::const_iterator itr = m_zero.find(tech);
    if(itr != m_zero.end()) {
        const ZeroCalibMap& zeroMap = itr->second;
        ZeroCalibMap::const_iterator type_itr = zeroMap.find(type);
        if(type_itr != zeroMap.end()) return &type_itr->second;
    }
    return nullptr;
}


