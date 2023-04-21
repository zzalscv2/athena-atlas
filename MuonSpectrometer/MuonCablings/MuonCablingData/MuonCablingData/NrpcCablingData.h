/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_CABLING_NRPCCABLINGDATA_H
#define MUON_CABLING_NRPCCABLINGDATA_H

#include <cstdint>
#include <iostream>
/*
 * @brief: Helper struct containing all information to convert from the offline
 * identifiers to the online identifiers
 */

/// Split the offline part of the cabling apart to use it later for sorting
struct NrpcCablingOffData {
    NrpcCablingOffData() = default;
    NrpcCablingOffData(const NrpcCablingOffData& other) : NrpcCablingOffData() {
        m_cache.hash = other.m_cache.hash;
    }
    NrpcCablingOffData& operator=(const NrpcCablingOffData& other) {
        if (&other != this)
            m_cache.hash = other.m_cache.hash;
        return *this;
    }
    int8_t& stationIndex{
        m_cache.cache[0]};  /// Station of the chamber (i.e, BIL,BIS,etc.)
    int8_t& eta{m_cache.cache[1]};       /// Eta of the RPC station
    int8_t& phi{m_cache.cache[2]};       /// Phi sector of the RPC station
    int8_t& doubletR{m_cache.cache[3]};  /// doublet R -> 1,2

    int8_t& doubletPhi{m_cache.cache[4]};  /// doublet Phi -> 1,2
    int8_t& doubletZ{m_cache.cache[5]};    /// doublet Z -> 1,2
    int8_t& gasGap{m_cache.cache[6]};      /// gas gap -> 1-3
    int8_t& measPhi{m_cache.cache[7]};     /// measures phi -> 0,1

    /// Equality operator
    bool operator==(const NrpcCablingOffData& other) const {
        return m_cache.hash == other.m_cache.hash;
    }
    bool operator!=(const NrpcCablingOffData& other) const {
        return m_cache.hash != other.m_cache.hash;
    }
    bool operator<(const NrpcCablingOffData& other) const {
        return m_cache.hash < other.m_cache.hash;
    }
    bool operator!() const { return !m_cache.hash; }

   private:
    union {
        long int hash{0};
        int8_t cache[8];
    } m_cache{};
};

struct NrpcCablingOnData {
    NrpcCablingOnData() = default;

    /// Identifier of the subdetector region in the readout BA / BC etc.
    int16_t& subDetector{m_cache.cache[0]};
    /// Unique Identifier of the Rpc chamber from an online perspective
    int16_t& tdcSector{m_cache.cache[1]};
    /// TDC chip
    int16_t& tdc{m_cache.cache[2]};

    /// First strip read out by the online card
    uint8_t firstStrip{0};
    /// Last strip read out by the online card (inclusive)
    uint8_t lastStrip{0};

    NrpcCablingOnData(const NrpcCablingOnData& other) : NrpcCablingOnData() {
        m_cache.hash = other.m_cache.hash;
        firstStrip = other.firstStrip;
        lastStrip = other.lastStrip;
    }
    NrpcCablingOnData& operator=(const NrpcCablingOnData& other) {
        if (&other == this)
            return *this;
        m_cache.hash = other.m_cache.hash;
        firstStrip = other.firstStrip;
        lastStrip = other.lastStrip;
        return *this;
    }

    bool operator==(const NrpcCablingOnData& other) const {
        return m_cache.hash == other.m_cache.hash;
    }
    bool operator!=(const NrpcCablingOnData& other) const {
        return m_cache.hash != other.m_cache.hash;
    }
    bool operator<(const NrpcCablingOnData& other) const {
        return m_cache.hash < other.m_cache.hash;
    }
    bool operator!() const { return !m_cache.hash; }

   private:
    union {
        int64_t hash{0};
        int16_t cache[4];
    } m_cache{};
};

struct NrpcCablingData : public NrpcCablingOffData, NrpcCablingOnData {
    NrpcCablingData() = default;

    uint8_t strip{0};      // Offline strip number
    uint8_t channelId{0};  // Online tdc channel number

    /// Equality operator
    bool operator==(const NrpcCablingData& other) const {
        return this->NrpcCablingOffData::operator==(other) &&
               this->NrpcCablingOnData::operator==(other) &&
               channelId == other.channelId && strip == other.strip;
    }
    bool operator!=(const NrpcCablingData& other) const {
        return !((*this) == other);
    }
    bool operator<(const NrpcCablingData& other) const {
        if (this->NrpcCablingOffData::operator!=(other))
            return this->NrpcCablingOffData::operator<(other);
        if (this->NrpcCablingOnData::operator!=(other))
            return this->NrpcCablingOnData::operator<(other);
        if (channelId != other.channelId)
            return channelId < other.channelId;
        return strip < other.strip;
    }
};

std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOffData& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOnData& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingData& obj);

#endif
