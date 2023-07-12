/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHITCNV_MDTSIMHITTOXAODCNVALG_H
#define XAODMUONSIMHITCNV_MDTSIMHITTOXAODCNVALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <MuonSimEvent/MDTSimHitCollection.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometry/MuonDetectorManager.h>
/**
 * The MdtSimHitToxAODCnvAlg takes the legacy MdtSimHits and translates them into the xAOD format
*/
class MdtHitIdHelper;

class MdtSimHitToxAODCnvAlg: public AthReentrantAlgorithm{
    public:
        MdtSimHitToxAODCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

        ~MdtSimHitToxAODCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;
    private:
        /// Retrieves the transformation into the restframe of a Mdt tube
        Amg::Transform3D getTubeTransform(const EventContext& ctx, const Identifier& id) const;

        SG::ReadHandleKey<MDTSimHitCollection> m_readKey{this, "InputCollection", "MDTHits", 
                                                        "Name of the legacy SimHit collection"};
        SG::WriteHandleKey<xAOD::MuonSimHitContainer> m_writeKey{this, "OutputCollection", "xMdtSimHits",
                                                              "Name of the new xAOD SimHit collection"};
  
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        /// IdHelper needed to decode the legacy sim hit Identifier
        const MdtHitIdHelper* m_muonHelper{nullptr};
        /// Legacy access to the geometry
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_legDetMgrKey{this,  "MuonManagerKey", "MuonDetectorManager"};
        
};
#endif