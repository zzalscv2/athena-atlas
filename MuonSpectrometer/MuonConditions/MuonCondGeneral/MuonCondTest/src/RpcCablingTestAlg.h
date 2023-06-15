/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMDT_CABLING_RPCCABLINGTESTALG_H
#define MUONMDT_CABLING_RPCCABLINGTESTALG_H
/**
   Algorithm to test the validity of the RPC cabling
*/
#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonCablingData/MuonNRPC_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"



class RpcCablingTestAlg : public AthAlgorithm {
public:
    RpcCablingTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~RpcCablingTestAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual unsigned int cardinality() const override final{return 1;}

    using CablingData = NrpcCablingData;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                "Key of input MuonDetectorManager condition data"};

    SG::ReadCondHandleKey<MuonNRPC_CablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap", "Key of input MDT cabling map"};

    Gaudi::Property<std::vector<std::string>> m_considStat{this, "TestStations",
                                                            {"BIS"}, "Cabling only for stations from these stations are tested" };

    std::set<int> m_cabStat{};
};

#endif
