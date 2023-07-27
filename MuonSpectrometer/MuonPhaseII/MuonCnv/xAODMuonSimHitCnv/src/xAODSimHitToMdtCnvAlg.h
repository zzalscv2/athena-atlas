/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHITCNV_MdtSimHitToxAODCnvAlg_H
#define XAODMUONSIMHITCNV_MdtSimHitToxAODCnvAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <MuonSimEvent/MDTSimHitCollection.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>

/**
 * The MdtSimHitToxAODCnvAlg takes the legacy MdtSimHits and translates them into the xAOD format
*/
class MdtHitIdHelper;

class xAODSimHitToMdtCnvAlg: public AthReentrantAlgorithm {
    public:
        xAODSimHitToMdtCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

        ~xAODSimHitToMdtCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;
    private:
        SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_readKey{this, "InputCollection", "xMdtSimHits",
                                                              "Name of the new xAOD SimHit collection"};

        SG::WriteHandleKey<MDTSimHitCollection> m_writeKey{this, "OutputCollection", "MDTHits", 
                                                        "Name of the legacy SimHit collection"};

        Gaudi::Property<bool> m_useNewGeo{this, "UseR4DetMgr", false,
                                         "Switch between the legacy and the new geometry"};


        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        /// IdHelper needed to decode the legacy sim hit Identifier
        const MdtHitIdHelper* m_muonHelper{nullptr};
        
};
#endif