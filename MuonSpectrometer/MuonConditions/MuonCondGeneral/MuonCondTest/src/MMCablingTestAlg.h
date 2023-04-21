/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   Algorithm to test the validity of the MDT cabling
*/

#ifndef MUONMDT_CABLING_MMCablingTestAlg_H
#define MUONMDT_CABLING_MMCablingTestAlg_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonCablingData/MicroMega_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"



class MMCablingTestAlg
 : public AthAlgorithm {
public:
    MMCablingTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MMCablingTestAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual unsigned int cardinality() const override final{return 1;}

   

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                "Key of input MuonDetectorManager condition data"};

    SG::ReadCondHandleKey<MicroMega_CablingMap> m_cablingKey{this, "ReadKey", "MicroMegaCabling", "Key of input MM cabling map"};

    Gaudi::Property<std::string> m_dumpFile{this, "DumpMap", "" , "Text file to which every cabling channel is dumped"};
    
};

#endif
