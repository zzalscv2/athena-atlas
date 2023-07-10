/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonMDT_CablingAlg reads raw condition data and writes derived condition data
   to the condition store
*/

#ifndef MUONMM_CABLING_MUONMM_CABLINGALG_H
#define MUONMM_CABLING_MUONMM_CABLINGALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCablingData/MicroMega_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class MuonMM_CablingAlg : public AthAlgorithm {
   public:
    MuonMM_CablingAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonMM_CablingAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readCablingKey{
        this, "CablingFolder", /* "/MDT/MM/CABLING" */ "", "Key of input conditions folder for MM cabling map"};
    SG::WriteCondHandleKey<MicroMega_CablingMap> m_writeKey{
        this, "WriteKey", "MicroMegaCabling", "Key of output MM cabling map"};

    Gaudi::Property<std::string> m_JSONFile{
        this, "JSONFile", "", "External path to read the cabling from"};

    StatusCode loadCablingSchema(const std::string& payload,
                                 MicroMega_CablingMap& cabling_map) const;
};

#endif
