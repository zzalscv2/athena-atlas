/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAlignmentData/NswPassivationDbData.h"
#include "MuonIdHelpers/MmIdHelper.h"
NswPassivationDbData::NswPassivationDbData(const MmIdHelper& mmIdHelper):
    m_mmIdHelper(mmIdHelper) {}


void NswPassivationDbData::setData(const Identifier& chnlId, 
                                   const int pcb, 
                                   const float indiv, 
                                   const float extra, 
                                   const std::string& position) {
    unsigned long long channelId = chnlId.get_compact();
    PCBPassivation& passiv = m_data[channelId];
    passiv.valid = true;
    if (position=="left" ) passiv.left = indiv + extra;
    else if(position=="right") passiv.right = indiv + extra;
    else {
        passiv.left = extra;
        passiv.right = extra;
    }
    if(extra==0) return;
    /* in case an extra passivation is given (extra>0), it is applied (1/2) 
    to top or bottom depending on the following cases:
    * if pcb = 1 or 6 => only top
    * if pcb = 5 or 8 => only bottom
    * else (if pcb = 2, 3, 4 or 7) => both top and bottom
    */
    if(pcb!=5 && pcb!=8) passiv.top = extra/2;
    if(pcb!=1 && pcb!=6) passiv.bottom = extra/2;
}
// retrieval functions -------------------------------

// getChannelIds
std::vector<Identifier> NswPassivationDbData::getChannelIds() const {
    std::vector<Identifier> keys;
    std::transform(m_data.begin(),m_data.end(), std::back_inserter(keys), 
        [](const PassivationMap::value_type& key_pair) ->Identifier{
            return Identifier{key_pair.first};
    });    
    return keys;
}

const NswPassivationDbData::PCBPassivation&  NswPassivationDbData::getPassivation(const Identifier& id) const {
    unsigned long long channelId = m_mmIdHelper.pcbID(id).get_compact();
    PassivationMap::const_iterator itr = m_data.find(channelId);
    if (itr != m_data.end()) return itr->second;
    static const PCBPassivation dummy{};
    return dummy;    
}
