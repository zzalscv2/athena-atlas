/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/MuonNRPC_CablingMap.h"

#include <cmath>

#include "GaudiKernel/ISvcLocator.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "StoreGate/StoreGateSvc.h"

MuonNRPC_CablingMap::MuonNRPC_CablingMap() {
    // initialize the message service

    // retrieve the RpcIdHelper
    ISvcLocator* svcLocator = Gaudi::svcLocator();
    StoreGateSvc* detStore = nullptr;
    StatusCode sc = svcLocator->service("DetectorStore", detStore);
    if (sc != StatusCode::SUCCESS) {
        throw std::runtime_error("Could not find the detctor store");
    }
    sc = detStore->retrieve(m_rpcIdHelper, "RPCIDHELPER");
    if (sc != StatusCode::SUCCESS) {
        throw std::runtime_error("Could not retrieve the RpcIdHelper");
    }
}

MuonNRPC_CablingMap::~MuonNRPC_CablingMap() = default;

bool MuonNRPC_CablingMap::convert(const CablingData& cabling_data,
                                  Identifier& id, bool check_valid) const {
    bool valid{!check_valid};
    id =
        check_valid
            ? m_rpcIdHelper->channelID(
                  cabling_data.stationIndex, cabling_data.eta, cabling_data.phi,
                  cabling_data.doubletR, cabling_data.doubletZ,
                  cabling_data.doubletPhi, cabling_data.gasGap,
                  cabling_data.measPhi, cabling_data.strip, valid)
            : m_rpcIdHelper->channelID(
                  cabling_data.stationIndex, cabling_data.eta, cabling_data.phi,
                  cabling_data.doubletR, cabling_data.doubletZ,
                  cabling_data.doubletPhi, cabling_data.gasGap,
                  cabling_data.measPhi, cabling_data.strip);
    return valid;
}

bool MuonNRPC_CablingMap::convert(const Identifier& module_id,
                                  CablingData& cabling_data) const {
    if (!m_rpcIdHelper->is_rpc(module_id))
        return false;
    cabling_data.stationIndex = m_rpcIdHelper->stationName(module_id);
    cabling_data.eta = m_rpcIdHelper->stationEta(module_id);
    cabling_data.phi = m_rpcIdHelper->stationPhi(module_id);
    cabling_data.doubletR = m_rpcIdHelper->doubletR(module_id);
    cabling_data.doubletPhi = m_rpcIdHelper->doubletPhi(module_id);
    cabling_data.doubletZ = m_rpcIdHelper->doubletZ(module_id);
    cabling_data.gasGap = m_rpcIdHelper->gasGap(module_id);
    cabling_data.measPhi = m_rpcIdHelper->measuresPhi(module_id);
    cabling_data.strip = m_rpcIdHelper->strip(module_id);
    return true;
}

bool MuonNRPC_CablingMap::getOfflineId(CablingData& cabling_map,
                                       MsgStream& log) const {
    // Identify the chamber
    OnlToOfflMap::const_iterator itr = m_onToOffline.find(cabling_map);
    if (itr == m_onToOffline.end()) {
        log << MSG::ERROR << "getOfflineId() -- The detector "
            << static_cast<NrpcCablingOnData&>(cabling_map) << " is unknown"
            << endmsg;
        return false;
    }
    cabling_map.NrpcCablingOffData::operator=(itr->second);
    cabling_map.strip = itr->first.firstStrip + cabling_map.channelId - 1;
    if (log.level() <= MSG::DEBUG) {
        log << MSG::ALWAYS << " getOfflineId() -- Successfully converted "
            << cabling_map << endmsg;
    }
    return true;
}

/** get the online id from the offline id */
bool MuonNRPC_CablingMap::getOnlineId(CablingData& cabling_map,
                                      MsgStream& log) const {
    OfflToOnlMap::const_iterator itr = m_offToOnline.find(cabling_map);
    if (itr == m_offToOnline.end()) {
        log << MSG::ERROR << "getOnlineId() -- The offline identifier "
            << static_cast<const NrpcCablingOffData&>(cabling_map)
            << " is unknown " << endmsg;
        return false;
    }
    const std::set<NrpcCablingOnData>& onlineCards = itr->second;
    for (const NrpcCablingOnData& card : onlineCards) {
        if (log.level() <= MSG::DEBUG) {
            log << MSG::ALWAYS << " getOnlineId() -- Test card " << card
                << " first strip: " << static_cast<int>(card.firstStrip)
                << " last strip: " << static_cast<int>(card.lastStrip)
                << endmsg;
        }
        if (stripReadByCard(card, cabling_map.strip)) {
            cabling_map.channelId = (cabling_map.strip - card.firstStrip) + 1;
            cabling_map.NrpcCablingOnData::operator=(card);
            if (log.level() <= MSG::DEBUG) {
                log << MSG::ALWAYS
                    << "getOnlineId() -- Successfully converted channel "
                    << cabling_map << endmsg;
            }
            return true;
        }
    }
    log << MSG::ERROR
        << "getOnlineId() -- No tdc channel could be found for the object "
        << static_cast<const NrpcCablingOffData&>(cabling_map)
        << " strip: " << static_cast<int>(cabling_map.strip) << endmsg;
    return false;
}
inline bool MuonNRPC_CablingMap::stripReadByCard(const NrpcCablingOnData& card,
                                                 uint16_t strip) const {
    return card.firstStrip <= strip && card.lastStrip >= strip;
}

bool MuonNRPC_CablingMap::insertChannels(const CablingData& cabling_data,
                                         MsgStream& log) {
    // Check that the channel is not overwritten
    if (log.level() <= MSG::DEBUG) {
        log << MSG::ALWAYS << "insertChannels() -- Insert new channel "
            << cabling_data
            << " firstStrip: " << static_cast<int>(cabling_data.firstStrip)
            << " lastStrip: " << static_cast<int>(cabling_data.lastStrip)
            << endmsg;
    }
    if (!m_offToOnline[cabling_data].insert(cabling_data).second) {
        log << MSG::ERROR
            << "insertChannels() -- Failed to fill the offline -> online map "
            << endmsg;
        log << MSG::ERROR
            << " --- Old: " << (*m_offToOnline[cabling_data].find(cabling_data))
            << endmsg;
        log << MSG::ERROR << " --- New: "
            << static_cast<const NrpcCablingOnData&>(cabling_data) << endmsg;
        return false;
    }
    if (!m_onToOffline[cabling_data]) {
        m_onToOffline[cabling_data] = cabling_data;
    } else {
        log << MSG::ERROR
            << "insertChannels() -- The online to offline map is already "
               "booked for identifier "
            << static_cast<const NrpcCablingOnData&>(cabling_data) << endmsg;
        log << MSG::ERROR << " -- Old: " << m_onToOffline[cabling_data]
            << endmsg;
        log << MSG::ERROR << " -- New: "
            << static_cast<const NrpcCablingOffData&>(cabling_data) << endmsg;
        return false;
    }
    return true;
}
bool MuonNRPC_CablingMap::finalize(MsgStream& log) {
    if (m_offToOnline.empty()) {
        log << MSG::ERROR << "finalize() -- No data has been loaded " << endmsg;
        return false;
    }
    /// First check that the map does not have any overlapping channels
    for (const auto& [chambChannel, cards] : m_offToOnline) {
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << "Check mapping consistency of "
                << chambChannel << endmsg;
        }
        for (const NrpcCablingOnData& card : cards) {
            /// Check whether the first and last strips are non-zero and that
            /// the first strip is actually smaller than the last one
            if (!card.firstStrip || !card.lastStrip ||
                card.firstStrip > card.lastStrip) {
                log << MSG::ERROR << "finalize() -- Invalid card detected for "
                    << chambChannel << endmsg;
                log << MSG::ERROR << card << static_cast<int>(card.firstStrip)
                    << " lastStrip: " << static_cast<int>(card.lastStrip)
                    << endmsg;
                return false;
            }
            if (std::count_if(
                    cards.begin(), cards.end(),
                    [this, &card](const NrpcCablingOnData& other_crd) {
                        return stripReadByCard(other_crd, card.firstStrip) ||
                               stripReadByCard(other_crd, card.lastStrip);
                    }) > 1) {
                log << MSG::ERROR
                    << "finalize() -- Detected readout overlap for "
                    << chambChannel << endmsg;
                log << MSG::ERROR << "The strips of card " << card
                    << static_cast<int>(card.firstStrip)
                    << " lastStrip: " << static_cast<int>(card.lastStrip)
                    << " are read by another one" << endmsg;
                return false;
            }
        }
    }
    /// Generate the ROB maps
    CablingData RobOffId{};
    RobOffId.strip = 1;
    for (const auto& [card, chamber] : m_onToOffline) {
        static_cast<NrpcCablingOffData&>(RobOffId) = chamber;
        Identifier chId{0};
        if (!convert(RobOffId, chId, true)) {
            log << MSG::ERROR
                << "Could not construct an offline identifier from " << chamber
                << endmsg;
            return false;
        }
        IdentifierHash hash{0};
        if (m_rpcIdHelper->get_module_hash(chId, hash)) {
            log << MSG::ERROR << " Failed to generate a hash for " << chamber
                << endmsg;
            return false;
        }
        uint32_t hardId = (card.subDetector << 16) | card.tdcSector;
        m_chambROBs[hash] = hardId;
        std::vector<IdentifierHash>& robHashes = m_ROBHashes[hardId];
        if (std::find(robHashes.begin(), robHashes.end(), hash) ==
            robHashes.end())
            robHashes.push_back(hash);
    }
    return true;
}
uint32_t MuonNRPC_CablingMap::getROBId(const IdentifierHash& stationCode,
                                       MsgStream& log) const {
    ChamberToROBMap::const_iterator it = m_chambROBs.find(stationCode);
    if (it != m_chambROBs.end()) {
        return it->second;
    }
    log << MSG::WARNING << "Station code " << stationCode << " not found !"
        << endmsg;
    return 0;
}

MuonNRPC_CablingMap::ListOfROB MuonNRPC_CablingMap::getROBId(
    const std::vector<IdentifierHash>& rpcHashVector, MsgStream& log) const {
    ListOfROB to_ret{};
    to_ret.reserve(rpcHashVector.size());
    for (const IdentifierHash& hash : rpcHashVector)
        to_ret.push_back(getROBId(hash, log));
    return to_ret;
}
const std::vector<IdentifierHash>& MuonNRPC_CablingMap::getChamberHashVec(
    const uint32_t ROBI, MsgStream& log) const {
    ROBToChamberMap::const_iterator itr = m_ROBHashes.find(ROBI);
    if (itr != m_ROBHashes.end())
        return itr->second;
    log << MSG::WARNING << "ROB ID " << ROBI << " not found ! " << endmsg;
    static const std::vector<IdentifierHash> dummy;
    return dummy;
}
