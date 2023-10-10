/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSIMHITSORTING_MUONSIMHITSORTINGALG_H
#define MUONSIMHITSORTING_MUONSIMHITSORTINGALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/WriteHandleKey.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

class MuonSimHitSortingAlg: public AthReentrantAlgorithm{
    public:
        MuonSimHitSortingAlg(const std::string& name, ISvcLocator* pSvcLocator);

        ~MuonSimHitSortingAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;    
    private:
        SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_readKeys{this, "InContainers", {}};
        SG::WriteHandleKey<xAOD::MuonSimHitContainer> m_writeKey{this, "OutContainer", ""};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        Gaudi::Property<bool> m_writeDeepCopy{this, "deepCopy", false, 
                            "If set to true. A new xAOD container is created instead of a VIEW_ELEMENTS version"};
};


#endif