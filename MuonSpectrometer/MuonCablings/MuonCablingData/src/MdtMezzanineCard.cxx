/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCablingData/MdtMezzanineCard.h>

#include <algorithm>
#include <iostream>
#include <set>

namespace {
using Mapping = MdtMezzanineCard::Mapping;
using OfflineCh = MdtMezzanineCard::OfflineCh;
std::ostream& operator<<(std::ostream& ostr, const Mapping& map) {
    for (unsigned int ch = 0; ch < map.size(); ++ch) {
        ostr << ch << " -> " << static_cast<int>(map[ch]);
        if (ch != (map.size() - 1))
            ostr << ", ";
    }
    return ostr;
}
}  // namespace
std::ostream& operator<<(std::ostream& ostr, const MdtMezzanineCard& map) {
    ostr << "MezzId: " << static_cast<int>(map.id()) << ", ";
    ostr << "nTubeLayer: " << static_cast<int>(map.numTubeLayers()) << ", ";
    ostr << "tdcToTubeMap: " << map.tdcToTubeMap() << ", ";
    ostr << "tubeToTdcMap: " << std::endl;
    for (uint8_t lay = map.numTubeLayers(); lay > 0; --lay) {
        if (!(lay % 2))
            ostr << "   ";
        for (uint8_t tube = 1; tube <= map.numTubesPerLayer(); ++tube) {
            uint8_t tubeNum = map.tubeNumber(lay, tube);
            ostr << std::setw(5)
                 << static_cast<int>(map.tubeToTdcMap()[tubeNum]);
        }
        ostr << std::endl;
    }
    return ostr;
}
MdtMezzanineCard::MdtMezzanineCard(const Mapping& tdcToTubeMap,
                                   uint8_t num_layers, uint8_t mezz_id)
    : m_tdcToTubes{tdcToTubeMap}, m_nlay{num_layers}, m_mezzId{mezz_id} {
    /// Should be 6 or 8 tubes per layer
    m_nTubes = m_tdcToTubes.size() / num_layers;
    /// Fill the map from tdc -> tube number
    for (unsigned int tdc = 0; tdc < m_tdcToTubes.size(); ++tdc) {
        uint8_t tube = m_tdcToTubes[tdc];
        if (tube < m_tubesToTdc.size())
            m_tubesToTdc[tube] = tdc;
    }
}

bool MdtMezzanineCard::checkConsistency(MsgStream& log) const {
    const bool debug = (log.level() <= MSG::VERBOSE);
    if (debug) {
        log << MSG::VERBOSE << " Check consistency of mezzanine card "
            << static_cast<int>(id()) << "." << endmsg;
    }
    if (numTubeLayers() != 3 && numTubeLayers() != 4) {
        log << MSG::ERROR << "Mezzanine card " << static_cast<int>(id())
            << " has invalid many tube layers "
            << static_cast<int>(numTubeLayers()) << endmsg;
        return false;
    }
    /// Check that all channels are actually set
    std::set<uint8_t> uniqueSet{};
    std::copy_if(m_tdcToTubes.begin(), m_tdcToTubes.end(),
                 std::inserter(uniqueSet, uniqueSet.end()),
                 [](const uint8_t ch) { return ch != NOTSET; });
    unsigned int unmapped =
        std::count_if(m_tdcToTubes.begin(), m_tdcToTubes.end(),
                      [](const uint8_t ch) { return ch == NOTSET; });
    if ((uniqueSet.size() + unmapped) != m_tdcToTubes.size()) {
        log << MSG::ERROR
            << "Mezzanine card has unassigned tdc -> tube channels " << endmsg;
        log << MSG::ERROR << "Mapped channels " << uniqueSet.size()
            << " dead channels: " << unmapped << std::endl;
        log << MSG::ERROR << "Please check " << (*this) << endmsg;
        return false;
    }
    if (unmapped == m_tdcToTubes.size()) {
        log << MSG::ERROR
            << "Mezzanine card does not have any associated channel " << endmsg;
        log << MSG::ERROR << "Please check " << (*this) << endmsg;
    }
    uniqueSet.clear();
    std::copy_if(m_tubesToTdc.begin(), m_tubesToTdc.end(),
                 std::inserter(uniqueSet, uniqueSet.end()),
                 [](const uint8_t ch) { return ch != NOTSET; });
    unmapped = std::count_if(m_tubesToTdc.begin(), m_tubesToTdc.end(),
                             [](const uint8_t ch) { return ch == NOTSET; });
    if ((uniqueSet.size() + unmapped) != m_tubesToTdc.size()) {
        log << MSG::ERROR << " Mezzanine card maps tubes -> tdc inconsitently "
            << endmsg;
        log << MSG::ERROR << "Mapped channels " << uniqueSet.size()
            << " dead channels: " << unmapped << std::endl;
        log << MSG::ERROR << " Please check " << (*this) << endmsg;
        return false;
    }
    if (debug) {
        log << MSG::VERBOSE << " All checks passed " << std::endl
            << (*this) << endmsg;
    }
    return true;
}

uint8_t MdtMezzanineCard::tdcChannel(uint8_t tubeLay, uint8_t tube,
                                     MsgStream& msg) const {
    if (tubeLay > numTubeLayers()) {
        msg << MSG::WARNING << "MdtMezzanineCard::tdcChannel() -- Tube layer "
            << static_cast<int>(tubeLay) << " is out of range. Max allowed "
            << static_cast<int>(numTubeLayers()) << endmsg;
        return NOTSET;
    }
    uint8_t globTubeNum = tubeNumber(tubeLay, tube);
    if (msg.level() <= MSG::VERBOSE) {
        msg << "MdtMezzanineCard::tdcChannel() -- Resolved layer "
            << static_cast<int>(tubeLay) << " & tube " << static_cast<int>(tube)
            << " to " << static_cast<int>(globTubeNum) << endmsg;
    }
    return m_tubesToTdc[globTubeNum];
}
uint8_t MdtMezzanineCard::tubeNumber(uint8_t tubeLay, uint8_t tube) const {
    return ((tube - 1) % numTubesPerLayer()) +
           (numTubesPerLayer() * (tubeLay - 1));
}
OfflineCh MdtMezzanineCard::offlineTube(uint8_t tdc, MsgStream& msg) const {
    if (tdc >= m_tdcToTubes.size()) {
        msg << MSG::WARNING << " Tdc channel is out of range "
            << static_cast<int>(tdc) << endmsg;
        return {};
    }
    uint8_t globTubeNum = m_tdcToTubes[tdc];
    uint8_t tube = globTubeNum % numTubesPerLayer();
    uint8_t lay = (globTubeNum - tube) / numTubesPerLayer();
    OfflineCh ret{};
    /// Do not offset the tube as the
    /// tube number needs to be shifted by the first tube of the card
    ret.tube = tube;
    ret.layer = lay + 1;
    ret.isValid = globTubeNum != NOTSET;
    return ret;
}
