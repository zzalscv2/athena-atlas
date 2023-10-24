#include "MuonCondData/NswT0Data.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "Identifier/Identifier.h"
#include "AthenaKernel/IOVInfiniteRange.h"

NswT0Data::NswT0Data(const MmIdHelper& mmIdHelper, const sTgcIdHelper& stgcIdHelper): m_mmIdHelper(mmIdHelper), m_stgcIdHelper(stgcIdHelper) {
    m_data_mmg.resize((m_mmIdHelper.detectorElement_hash_max()+1)*m_mmIdHelper.gasGapMax());
    m_data_stg.resize((m_stgcIdHelper.detectorElement_hash_max() +1)*(m_stgcIdHelper.gasGapMax()*3 /*3 channel types*/));
}

unsigned int NswT0Data::identToModuleIdx(const Identifier& chan_id) const{
    if (m_mmIdHelper.is_mm(chan_id)) {
        IdentifierHash hash{0};
        if (m_mmIdHelper.get_detectorElement_hash(chan_id, hash) || hash >= m_mmIdHelper.detectorElement_hash_max()){
            throw std::runtime_error("NswDcsDbData() - Failed to retrieve valid  micromega hash ");
        }
        return static_cast<unsigned int>(hash)*(m_mmIdHelper.gasGapMax()) + (m_mmIdHelper.gasGap(chan_id) -1);
    } else if (m_stgcIdHelper.is_stgc(chan_id)) {        
        IdentifierHash hash{0};
        if (m_stgcIdHelper.get_detectorElement_hash(chan_id, hash) || hash >= m_stgcIdHelper.detectorElement_hash_max()){
            throw std::runtime_error("NswDcsDbData() - Failed to retrieve valid stgc hash ");
        }
        return static_cast<unsigned int>(hash)*(m_stgcIdHelper.gasGapMax() * 3 /*3 channel types*/) + (m_stgcIdHelper.gasGap(chan_id) -1  + m_stgcIdHelper.gasGapMax() * m_stgcIdHelper.channelType(chan_id));
    }
    throw std::runtime_error("NswT0Data() - No MM or sTGC identifier");
    return -1;
}

void NswT0Data::setData(const Identifier& id, const float value){
    uint idx = identToModuleIdx(id);
    ChannelArray& data = m_mmIdHelper.is_mm(id)  ? m_data_mmg : m_data_stg;
    uint channelIdx{0};
    if(m_mmIdHelper.is_mm(id)){
        if(data.at(idx).empty())  data.at(idx).resize(m_mmIdHelper.channelMax(id), 0.f);
        channelIdx = m_mmIdHelper.channel(id) -1;
    } else {
        if(data.at(idx).empty())  data.at(idx).resize(m_stgcIdHelper.channelMax(id), 0.f);
        channelIdx = m_stgcIdHelper.channel(id) -1;
    }
    data.at(idx).at(channelIdx) = value;
}

bool NswT0Data::getT0(const Identifier& id, float& value) const {
    uint idx = identToModuleIdx(id);
    const ChannelArray& data = m_mmIdHelper.is_mm(id)  ? m_data_mmg : m_data_stg;
    uint channelId = m_mmIdHelper.is_mm(id)  ?  m_mmIdHelper.channel(id)-1 : m_stgcIdHelper.channel(id)-1;
    if(data.size() <= idx) return false;
    value = data[idx].at(channelId);
    return true;
}