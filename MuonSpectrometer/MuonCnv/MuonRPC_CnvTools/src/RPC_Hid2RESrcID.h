/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRPC_CNVTOOLS_RPC_HID2RESRCID_H
#define MUONRPC_CNVTOOLS_RPC_HID2RESRCID_H

#include <stdint.h>

#include <map>

#include "GaudiKernel/StatusCode.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "RPC_CondCabling/RpcCablingCondData.h"

class RPC_Hid2RESrcID {
public:
    RPC_Hid2RESrcID();

    RPC_Hid2RESrcID(int specialROBNumber);

    void set(const RpcIdHelper* rpdId);

    uint32_t getRodID(const Identifier& offlineId, const RpcCablingCondData* readCdo) const;
    static uint32_t getRodID(const int& side, const int& slogic, const int& padId, const RpcCablingCondData* readCdo) ;
    static uint32_t getRodID(const int& sector) ;
    static uint32_t getRodID(const uint16_t& side, const uint16_t& rodIndex) ;

    static uint32_t getRobID(const uint32_t rod_id) ;

    static uint32_t getRosID(const uint32_t rob_id) ;

    static uint32_t getDetID(const uint32_t ros_id) ;

private:
    const RpcIdHelper* m_rpcIdHelper;

    int m_specialROBNumber;
};

#endif
