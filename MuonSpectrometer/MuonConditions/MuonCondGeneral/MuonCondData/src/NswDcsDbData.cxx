/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/NswDcsDbData.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "Identifier/Identifier.h"
#include <TString.h>


// general functions ---------------------------------
NswDcsDbData::NswDcsDbData(const MmIdHelper& mmIdHelper, const sTgcIdHelper& stgcIdHelper):
    m_mmIdHelper(mmIdHelper),
    m_stgcIdHelper(stgcIdHelper)
{
}

NswDcsDbData::DcsFsmState
NswDcsDbData::getFsmStateEnum(std::string fsmState){
	if(fsmState=="UNKNOWN"  ) return NswDcsDbData::DcsFsmState::UNKNOWN;
	if(fsmState=="ON"       ) return NswDcsDbData::DcsFsmState::ON;
	if(fsmState=="OFF"      ) return NswDcsDbData::DcsFsmState::OFF;
	if(fsmState=="STANDBY"  ) return NswDcsDbData::DcsFsmState::STANDBY;
	if(fsmState=="DEAD"     ) return NswDcsDbData::DcsFsmState::DEAD;
	if(fsmState=="UNPLUGGED") return NswDcsDbData::DcsFsmState::UNPLUGGED;
	if(fsmState=="RAMP_UP"  ) return NswDcsDbData::DcsFsmState::RAMP_UP;
	if(fsmState=="RAMP_DOWN") return NswDcsDbData::DcsFsmState::RAMP_DOWN;
	if(fsmState=="TRIP"     ) return NswDcsDbData::DcsFsmState::TRIP;
	if(fsmState=="RECOVERY" ) return NswDcsDbData::DcsFsmState::RECOVERY;
	if(fsmState=="LOCKED"   ) return NswDcsDbData::DcsFsmState::LOCKED;
	return DcsFsmState::NONE;
}
std::string
NswDcsDbData::getFsmStateStrg(NswDcsDbData::DcsFsmState fsmState){
	if(fsmState==NswDcsDbData::DcsFsmState::UNKNOWN  ) return "UNKNOWN"  ;
	if(fsmState==NswDcsDbData::DcsFsmState::ON       ) return "ON"       ;
	if(fsmState==NswDcsDbData::DcsFsmState::OFF      ) return "OFF"      ;
	if(fsmState==NswDcsDbData::DcsFsmState::STANDBY  ) return "STANDBY"  ;
	if(fsmState==NswDcsDbData::DcsFsmState::DEAD     ) return "DEAD"     ;
	if(fsmState==NswDcsDbData::DcsFsmState::UNPLUGGED) return "UNPLUGGED";
	if(fsmState==NswDcsDbData::DcsFsmState::RAMP_UP  ) return "RAMP_UP"  ;
	if(fsmState==NswDcsDbData::DcsFsmState::RAMP_DOWN) return "RAMP_DOWN";
	if(fsmState==NswDcsDbData::DcsFsmState::TRIP     ) return "TRIP"     ;
	if(fsmState==NswDcsDbData::DcsFsmState::RECOVERY ) return "RECOVERY" ;
	if(fsmState==NswDcsDbData::DcsFsmState::LOCKED   ) return "LOCKED"   ;
	return "NONE";
}

std::ostream& operator<<(std::ostream& ostr, const NswDcsDbData::DcsConstants& obj) {
    ostr<<" v0set: "   <<std::setprecision(15)<<obj.v0set;
    ostr<<" v1set: "   <<std::setprecision(15)<<obj.v1set;
    ostr<<" fsmState: "<<NswDcsDbData::getFsmStateStrg(obj.fsmState);
    return ostr;
}

int NswDcsDbData::identToModuleIdx(const Identifier& chan_id) const{
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
        return array_idx;
    }
    return -1;
 }

// setting functions ---------------------------------

// setData
void
NswDcsDbData::setData(const DcsTechType tech, const Identifier& chnlId, DcsConstants constants) {
    if(tech == DcsTechType::MMG || tech == DcsTechType::MMD) {
        ChannelDcsMapMMG& dcsMap = tech == DcsTechType::MMG ? m_data_hv_mmg : m_data_hv_mmd;
        const int array_idx = identToModuleIdx(chnlId);   
        DcsModule& dcs_mod = dcsMap.at(array_idx);
        const unsigned int channel = m_mmIdHelper.channel(chnlId)-1;
        if(dcs_mod.channels.empty())
            dcs_mod.layer_id = m_mmIdHelper.channelID(chnlId, m_mmIdHelper.multilayer(chnlId), m_mmIdHelper.gasGap(chnlId), 1);
        if(dcs_mod.channels.size() <= channel) dcs_mod.channels.resize(channel +1);
        if(dcs_mod.channels[channel]) {
            throw std::runtime_error("setData() -- Cannot overwrite channel");
            return;
        }
        dcs_mod.channels[channel] = std::make_unique<DcsConstants>(std::move(constants));
    } else if(tech == DcsTechType::STG) {
        ChannelDcsMapSTG& dcsMap = m_data_hv_stg;
        const int array_idx = identToModuleIdx(chnlId);   
        DcsModule& dcs_mod = dcsMap.at(array_idx);
        const unsigned int channel = m_stgcIdHelper.channel(chnlId)-1;
        if(dcs_mod.channels.empty())
            dcs_mod.layer_id = m_stgcIdHelper.channelID(chnlId, m_stgcIdHelper.multilayer(chnlId), m_stgcIdHelper.gasGap(chnlId), m_stgcIdHelper.channelType(chnlId), 1);
        if(dcs_mod.channels.size() <= channel) dcs_mod.channels.resize(channel +1);
        if(dcs_mod.channels[channel]) {
            throw std::runtime_error("setData() -- Cannot overwrite channel");
            return;
        }
        dcs_mod.channels[channel] = std::make_unique<DcsConstants>(std::move(constants));
    }
}




// retrieval functions -------------------------------

// getChannelIds
std::vector<Identifier>
NswDcsDbData::getChannelIds(const DcsTechType tech, const std::string& side) const {
    std::vector<Identifier> chnls;
    if(tech == DcsTechType::MMG || tech == DcsTechType::MMD){
        const ChannelDcsMapMMG& dcsMap = tech == DcsTechType::MMG ? m_data_hv_mmg : m_data_hv_mmd;
        chnls.reserve(dcsMap.size());
        for(const DcsModule& module : dcsMap) {
            if(module.channels.empty()) continue;
            if(side == "A" && m_mmIdHelper.stationEta(module.layer_id) < 0) continue;
            if(side == "C" && m_mmIdHelper.stationEta(module.layer_id) > 0) continue;
            for(unsigned int chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if(!module.channels[chn -1]) continue;
                chnls.push_back(m_mmIdHelper.channelID(module.layer_id, m_mmIdHelper.multilayer(module.layer_id), m_mmIdHelper.gasGap(module.layer_id), chn ));
            }
        }
    } else if(tech == DcsTechType::STG){
        const ChannelDcsMapSTG& dcsMap = m_data_hv_stg;
        chnls.reserve(dcsMap.size());
        for(const DcsModule& module : dcsMap) {
            if(module.channels.empty()) continue;
            if(side == "A" && m_stgcIdHelper.stationEta(module.layer_id) < 0) continue;
            if(side == "C" && m_stgcIdHelper.stationEta(module.layer_id) > 0) continue;
            for(unsigned int chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if(!module.channels[chn -1]) continue;
                chnls.push_back(m_stgcIdHelper.channelID(module.layer_id, m_stgcIdHelper.multilayer(module.layer_id), 
                                        m_stgcIdHelper.gasGap(module.layer_id),  m_stgcIdHelper.channelType(module.layer_id), chn ));
            }
        }
    }
    return chnls;
}

const NswDcsDbData::DcsConstants* 
NswDcsDbData::getDataForChannel(const DcsTechType tech, const Identifier& channelId) const {
    if(tech == DcsTechType::MMG || tech == DcsTechType::MMD){
        if(!m_mmIdHelper.is_mm(channelId)) return nullptr;
        const ChannelDcsMapMMG& dcsMap = tech==DcsTechType::MMG ? m_data_hv_mmg : m_data_hv_mmd; // later add something like: type == DcsDataType::HV ? m_data_hv : m_data_lv;
        const int array_idx = identToModuleIdx(channelId);
        const unsigned int channel = m_mmIdHelper.channel(channelId) -1;
        if (dcsMap.at(array_idx).channels.size() > channel && dcsMap[array_idx].channels[channel]) return dcsMap[array_idx].channels[channel].get();
    } else if(tech == DcsTechType::STG){
        if(!m_mmIdHelper.is_stgc(channelId)) return nullptr;
        const ChannelDcsMapSTG& dcsMap = m_data_hv_stg; // later add something like: type == DcsDataType::HV ? m_data_hv_stg : m_data_lv_stg;
        const int array_idx = identToModuleIdx(channelId);
        const unsigned int channel = m_stgcIdHelper.channel(channelId) -1;
        if (dcsMap.at(array_idx).channels.size() > channel && dcsMap[array_idx].channels[channel]) return dcsMap[array_idx].channels[channel].get();
    }
    return nullptr;
}


