/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonCablingData/MdtCSMCrate.h>

MdtCSMCrate::MdtCSMCrate(const MdtCablingData& id):
    m_Id{id} {}
    
 bool MdtCSMCrate::getOnlineId(MdtCablingData& channel, MsgStream& msg) const {
    const bool debug = msg.level() <= MSG::VERBOSE;
    if (debug) msg<<MSG::VERBOSE<<"getOnlineId() -- Try to find online channel of "<<channel<<endmsg;
    if (offlineId() != channel) {
        if (debug) msg<<MSG::VERBOSE<<"getOnlineId() -- This crate "<<offlineId()<<" does not match! "<<endmsg;
        return false;   
    }
    for (const TdcCard& card : m_tdcs) {
        if (card.firstTube >= channel.tube && channel.tube < card.firstTube + card.mezzanine->numTubesPerLayer()) {
            channel.channelId = card.mezzanine->tdcChannel(channel.layer, channel.tube, msg);
            static_cast<MdtCablingOnData&>(channel) = csmId();
            if (debug) msg<<MSG::VERBOSE<<"getOnlineId() -- Mapping succeeded "<<channel<<endmsg;
            return true;
        }
    }
    return false;
 }
 bool MdtCSMCrate::getOfflineId(MdtCablingData& channel, MsgStream& msg) const {
    const bool debug = msg.level() <= MSG::VERBOSE;
    if (debug) msg<<MSG::VERBOSE<<"getOfflineId() -- Try to find online channel of "<<channel<<endmsg;
    if (csmId() != channel) {
        if (debug) msg<<MSG::VERBOSE<<"getOfflineId() -- This crate "<<csmId()<<" does not match! "<<endmsg;
        return false;
    }
    if (channel.tdcId >= m_tdcs.size()) {
        msg << MSG::WARNING<<"getOfflineChannel() -- "<<channel<<" has too large tdc number "<<endmsg;
        return false;
    }
    const TdcCard& card = m_tdcs[channel.tdcId];
    MdtMezzanineCard::OfflineCh tubeLayer = card.mezzanine->offlineTube(channel.channelId, msg);
    channel.layer = tubeLayer.layer;
    channel.tube = card.firstTube + tubeLayer.tube;
    return tubeLayer.isValid;
 }
bool MdtCSMCrate::checkConsistency(MsgStream& msg) const {
    const bool debug = msg.level() <= MSG::VERBOSE;
    if (debug) msg<<MSG::VERBOSE<<"checkConsistency() -- Check consistent mapping of CSM crate "<<m_Id<<endmsg;
   
    /// Check that the tdc has at least one valid chip
    unsigned int non_empty{0};
    uint8_t tid = -1;
    for (const TdcCard& card : m_tdcs) {
        ++tid;
        /// Allow for empty tdcs but ensure that they've a consistent mapping
        if (!card.mezzanine) continue;
        if (!card.mezzanine->checkConsistency(msg)) {
            msg<<MSG::ERROR<<"checkConsistency() -- Invalid mezzanine found for CSM crate "<<m_Id<<endmsg;
            return false;
        }
        if (tid != card.tdcId) {
            msg<<MSG::ERROR<<"checkConsistency() -- Found wrong tdc id at the"
                            <<static_cast<int>(tid)<<"-th position. Found: "<<static_cast<int>(card.tdcId)<<endmsg;
            return false;
        }        
        ++non_empty; 
    }
    if(!non_empty) {
        msg<<MSG::ERROR<<"checkConsistency() -- All tdc cards are empty for CSM crate "<<m_Id<<endmsg;
        return false;
    }
    return true;
}
bool MdtCSMCrate::addTdc(TdcCard&& card, MsgStream& msg) {
    if (!card.firstTube) {
        msg<<MSG::ERROR<<"addTdc() -- Invalid first tube given to CSM crate "<<m_Id<<endmsg;
        return false;
    }
    if (!card.mezzanine) {
        msg<<MSG::ERROR<<"addTdc() -- The card has no associated mezzanine "<<endmsg;
        return false;
    }
    /// Create enough empty cards if neccessary
    if (card.tdcId >= m_tdcs.size()) m_tdcs.resize(card.tdcId + 1);
    TdcCard& saveTo = m_tdcs[card.tdcId];
    /// The exact same chip has been added before
    if (saveTo.mezzanine) {
        msg<<MSG::ERROR<<"addTdc() -- The card "<<static_cast<int>(card.tdcId)<<" has already been added "<<endmsg;
        return false;
    }
    saveTo = std::move(card);
    return true;
}
