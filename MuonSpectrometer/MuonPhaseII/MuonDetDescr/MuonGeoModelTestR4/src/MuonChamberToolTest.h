/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H
#define MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <MuonStationGeoHelpers/IActsMuonChamberTool.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <StoreGate/ReadCondHandleKey.h>

namespace MuonGMR4 { 
class MuonChamberToolTest: public AthReentrantAlgorithm {
    public:
        MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator);

        ~MuonChamberToolTest() = default;

        StatusCode execute(const EventContext& ctx) const override;        
        StatusCode initialize() override;        

        bool isReEntrant() const override final {return false;}   
    
    private:
        
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadCondHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};


        PublicToolHandle<MuonGMR4::IActsMuonChamberTool> m_chambTool{this, "ChamberTool", "" };

        const MuonDetectorManager* m_detMgr{nullptr};

};
}
#endif