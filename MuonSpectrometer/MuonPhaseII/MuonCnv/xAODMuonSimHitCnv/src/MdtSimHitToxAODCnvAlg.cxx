/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtSimHitToxAODCnvAlg.h"

#include <MuonSimEvent/MdtHitIdHelper.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <MuonReadoutGeometry/MdtReadoutElement.h>

MdtSimHitToxAODCnvAlg::MdtSimHitToxAODCnvAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode MdtSimHitToxAODCnvAlg::initialize() {
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_legDetMgrKey.initialize());
      m_muonHelper = MdtHitIdHelper::GetHelper(m_idHelperSvc->mdtIdHelper().tubeMax());
    ATH_MSG_INFO("Successfully initialized the legacy -> xAOD MdtSimHit conversion "<<m_readKey.fullKey()<<", "<<m_writeKey.fullKey());    
    return StatusCode::SUCCESS;
}

StatusCode MdtSimHitToxAODCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<MDTSimHitCollection> inContainer{m_readKey, ctx};
    if (!inContainer.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the input SimHit collection "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    SG::WriteHandle<xAOD::MuonSimHitContainer> outContainer{m_writeKey, ctx};
    ATH_CHECK(outContainer.record(std::make_unique<xAOD::MuonSimHitContainer>(), 
                                  std::make_unique<xAOD::MuonSimHitAuxContainer>()));

    for (const MDTSimHit& hit : *inContainer) {
        const HitID legId = hit.MDTid();
        /// Translate the Identifier
        const std::string stName = m_muonHelper->GetStationName(legId);
        const int stEta = m_muonHelper->GetZSector(legId);
        const int stPhi = m_muonHelper->GetPhiSector(legId);
        const int ml = m_muonHelper->GetMultiLayer(legId);
        const int tubeLay = m_muonHelper->GetLayer(legId);
        const int tube = m_muonHelper->GetTube(legId);
        bool is_valid{false};
        const Identifier hitID = m_idHelperSvc->mdtIdHelper().channelID(stName, stEta, stPhi, ml, tubeLay, tube, is_valid);
        if (!is_valid){
            ATH_MSG_FATAL("No valid Identifier could be extracted from "<<stName<<","<<stPhi<<","<<ml<<","<<tubeLay<<","
                                                                        <<tube<<" giving "<<m_idHelperSvc->toString(hitID));
            return StatusCode::FAILURE;
        }
        xAOD::MuonSimHit* newHit = new xAOD::MuonSimHit();
        outContainer->push_back(newHit);
        newHit->setIdentifier(hitID);

        newHit->setLocalPosition(xAOD::toStorage(hit.localPosition()));
        /// Need to check whether we can persistify the HEPMCParticle link
        Amg::Vector3D locDir{Amg::Vector3D::Zero()};
        if (hit.particleLink().isValid()) {
            const auto& genParticle = (*hit.particleLink());
            const auto& mom = genParticle.momentum();
            locDir.x() = mom.x();
            locDir.y() = mom.y();
            locDir.z() = mom.z();
            locDir = getTubeTransform(ctx,hitID).linear() * locDir.unit();
        }
        newHit->setLocalDirection(xAOD::toStorage(locDir));
        newHit->setStepLength(hit.stepLength());
        newHit->setGlobalTime(hit.globalTime());
        newHit->setPdgId(hit.particleEncoding());
        newHit->setEnergyDeposit(hit.energyDeposit());
        newHit->setKineticEnergy(hit.kineticEnergy());
    }
    return StatusCode::SUCCESS;
}
Amg::Transform3D MdtSimHitToxAODCnvAlg::getTubeTransform(const EventContext& ctx, const Identifier& id) const{
   
    /// We may need to add a pass retrieving the new detector manager instead of the legacy one
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_legDetMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to retrieve the detector manager "<<m_legDetMgrKey.fullKey());
        throw std::runtime_error("Legacyt detector manager cannot be retrieved");
    }
    const MuonGM::MdtReadoutElement* reElement = detMgr->getMdtReadoutElement(id);
    if (reElement) {
        return reElement->globalToLocalTransf(id);
    } 
    ATH_MSG_WARNING("Failed to retrieve a valid transformation for "<<m_idHelperSvc->toString(id));
    return Amg::Transform3D::Identity();
}