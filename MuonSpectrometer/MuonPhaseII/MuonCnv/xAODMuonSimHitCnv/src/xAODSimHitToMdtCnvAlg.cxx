/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODSimHitToMdtCnvAlg.h"

#include <MuonSimEvent/MdtHitIdHelper.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <GaudiKernel/SystemOfUnits.h>
namespace{
    constexpr double toGeV = 1. / Gaudi::Units::GeV;

}
xAODSimHitToMdtCnvAlg::xAODSimHitToMdtCnvAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitToMdtCnvAlg::initialize() {
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    m_muonHelper = MdtHitIdHelper::GetHelper(m_idHelperSvc->mdtIdHelper().tubeMax());
    ATH_MSG_INFO("Successfully initialized the xAOD MdtSimHit -> legacy conversion "<<m_readKey.fullKey()<<", "<<m_writeKey.fullKey());    
    return StatusCode::SUCCESS;
}

StatusCode xAODSimHitToMdtCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::MuonSimHitContainer> inContainer{m_readKey, ctx};
    if (!inContainer.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the input SimHit collection "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    SG::WriteHandle<MDTSimHitCollection> outContainer{m_writeKey, ctx};
    ATH_CHECK(outContainer.record(std::make_unique<MDTSimHitCollection>()));

    for (const xAOD::MuonSimHit* hit : *inContainer) {
        int MDTid = m_muonHelper->BuildMdtHitId(m_idHelperSvc->stationNameString(hit->identify()), 
                                                m_idHelperSvc->stationPhi(hit->identify()),
                                                m_idHelperSvc->stationEta(hit->identify()), 
                                                id_helper.multilayer(hit->identify()),
                                                id_helper.tubeLayer(hit->identify()),
                                                id_helper.tube(hit->identify()));
        outContainer->Emplace(MDTid, 
                              hit->globalTime(), 
                              hit->localPosition().perp(),
                              xAOD::toEigen(hit->localPosition()),
                              hit->genParticleLink(),
                              hit->stepLength(),
                              hit->energyDeposit(),
                              hit->pdgId(),
                              hit->kineticEnergy());
        if (msgLvl(MSG::DEBUG)){            
            std::stringstream hep_mc{};
            if (!hit->genParticleLink().isValid()){
                hep_mc<<"HEP mc link invalid";
            } else {
               const HepMC::GenParticle& mc_part{*hit->genParticleLink()};
#ifdef HEPMC3
               hep_mc<<"Valid HEP mc link: "<<mc_part.pid()<<", ";
#else
               hep_mc<<"Valid HEP mc link: "<<mc_part.pdg_id()<<", ";
#endif
               hep_mc<<"Four momentum: pt:"<<mc_part.momentum().perp()*toGeV<<" [GeV], ";
               hep_mc<<"eta: "<<mc_part.momentum().eta()<<", ";
               hep_mc<<"phi: "<<mc_part.momentum().phi()<<", ";
            }
            ATH_MSG_INFO("Converted xAOD simHit "<<m_idHelperSvc->toString(hit->identify())<<", "
                     <<"globalTime: "<<hit->globalTime()<<", "
                     <<"drift radius: "<<hit->localPosition().perp()<<", "
                     <<"step length: "<<hit->stepLength()<<", "
                     <<"energy deposit: "<<hit->energyDeposit()*toGeV<<" [GeV], "
                     <<"pdgId: "<<hit->pdgId()<<", "
                     <<"kinetic energy: "<<hit->kineticEnergy()*toGeV<<" [GeV], "
                     <<hep_mc.str());
     
        }
    }  
    return StatusCode::SUCCESS;
}
