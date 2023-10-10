/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSimHitSortingAlg.h"


#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <AthContainers/ConstDataVector.h>


MuonSimHitSortingAlg::MuonSimHitSortingAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode MuonSimHitSortingAlg::initialize() {
    if (m_readKeys.empty()) {
        ATH_MSG_FATAL("Please provide at least one container to sort");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_readKeys.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode MuonSimHitSortingAlg::execute(const EventContext& ctx) const {
    ConstDataVector<xAOD::MuonSimHitContainer> allSimHits{SG::VIEW_ELEMENTS};
    for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& inKey : m_readKeys) {
         SG::ReadHandle<xAOD::MuonSimHitContainer> readHandle{inKey, ctx};
         if(!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve "<<inKey.fullKey());
            return StatusCode::FAILURE;
         }
         std::copy(readHandle->begin(), readHandle->end(), std::back_inserter(allSimHits));
    }
    std::stable_sort(allSimHits.begin(), allSimHits.end(),
                    [this](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                        return m_idHelperSvc->detElementHash(a->identify()) < 
                               m_idHelperSvc->detElementHash(b->identify());
                    });
    SG::WriteHandle<xAOD::MuonSimHitContainer> writeHandle{m_writeKey, ctx};
    if (m_writeDeepCopy) {
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSimHitContainer>(),
                                     std::make_unique<xAOD::MuonSimHitAuxContainer>()));
        for (const xAOD::MuonSimHit* copy_me : allSimHits) {
            xAOD::MuonSimHit* newHit = new xAOD::MuonSimHit();
            writeHandle->push_back(newHit);
            newHit->setLocalDirection(copy_me->localDirection());
            newHit->setLocalPosition(copy_me->localPosition());
            newHit->setStepLength(copy_me->stepLength());
            newHit->setGlobalTime(copy_me->globalTime());
            newHit->setPdgId(copy_me->pdgId());
            newHit->setIdentifier(copy_me->identify());
            newHit->setEnergyDeposit(copy_me->energyDeposit());
            newHit->setKineticEnergy(copy_me->kineticEnergy());
            newHit->setGenParticleLink(copy_me->genParticleLink());
        }
    } else {
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSimHitContainer>(*allSimHits.asDataVector())));
     }
    return StatusCode::SUCCESS;    
}
