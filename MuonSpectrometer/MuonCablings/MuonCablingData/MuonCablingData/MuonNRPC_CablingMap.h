/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONNRPC_CABLING_MUONNRPC_CABLINGMAP_H
#define MUONNRPC_CABLING_MUONNRPC_CABLINGMAP_H
#include "AthenaKernel/CLASS_DEF.h"
#include "Identifier/Identifier.h"
#include "MuonCablingData/NrpcCablingData.h"
#include <set>
/**********************************************
 *
 * @brief NRPC map data object
 *
 **********************************************/

class RpcIdHelper;
class IdentifierHash;

class MuonNRPC_CablingMap {
public:
    /** typedef to implement the csm mapping to ROB */
    /* mapping from hashid to ROB identifier as Subdetector+Rodid */
    using CablingData = NrpcCablingData;
    MuonNRPC_CablingMap();
    ~MuonNRPC_CablingMap();

    /** return the offline id given the online id */
    bool getOfflineId(CablingData& cabling_data, MsgStream& log) const;

    /** return the online id given the offline id */
    bool getOnlineId(CablingData& cabling_data, MsgStream& log) const;
    /** converts the cabling data into an identifier. The check valid argument optionally enables the check that the returned identifier is
     * actually well defined within the ranges but is also slow */
    bool convert(const CablingData& cabling_data, Identifier& id, bool check_valid = true) const;
    /** converts the identifier into a cabling data object. Returns false if the Identifier is not Nrpc */
    bool convert(const Identifier& id, CablingData& cabling_data) const;

    /// Inserts a cabling object into the map
    bool insertChannels(const CablingData& cabling_data, MsgStream& log);
    /// Performs consistency checks for the cabling data (I.e. looking for 0 strips and overlaps)
    bool finalize(MsgStream& log);

private:

    using OnlToOfflMap = std::map<NrpcCablingOnData, NrpcCablingOffData>;
    using OfflToOnlMap = std::map<NrpcCablingOffData, std::set<NrpcCablingOnData>>;
    OnlToOfflMap m_onToOffline;
    OfflToOnlMap m_offToOnline;

    bool stripReadByCard(const NrpcCablingOnData& card, uint16_t strip) const;
    

    /** Pointer to the RpcIdHelper */
    const RpcIdHelper* m_rpcIdHelper{};

};

CLASS_DEF(MuonNRPC_CablingMap, 94020450, 0)
#include "AthenaKernel/CondCont.h"
CLASS_DEF(CondCont<MuonNRPC_CablingMap>, 207572956, 0)

#endif
