/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTFastDigiTester.h"


namespace MuonValR4 {
    MDTFastDigiTester::MDTFastDigiTester(const std::string& name,
                                ISvcLocator* pSvcLocator)
        : AthHistogramAlgorithm(name, pSvcLocator) {}


    StatusCode MDTFastDigiTester::initialize() {
        ATH_CHECK(m_inSimHitKey.initialize());
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_inDriftCircleKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_MSG_DEBUG("Succesfully initialised");
        return StatusCode::SUCCESS;
    }

    StatusCode MDTFastDigiTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

    StatusCode MDTFastDigiTester::execute()  {

        // retrieve the two input collections
        const EventContext & context = Gaudi::Hive::currentContext();
        SG::ReadHandle<xAOD::MuonSimHitContainer> readSimHits(m_inSimHitKey,
                                                              context);
        SG::ReadHandle<xAOD::MdtDriftCircleContainer> readDriftCircles(
            m_inDriftCircleKey, context);
        if (!readSimHits.isValid()){
            ATH_MSG_FATAL("Failed to read MDT sim hits");
            return StatusCode::FAILURE;
        }
        if (!readDriftCircles.isValid()){
            ATH_MSG_FATAL("Failed to read MDT drift circles");
            return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG("Succesfully retrieved input collections");

        // map the drift circles to identifiers. 
        // The fast digi should only generate one circle per tube. 
        std::map<Identifier, const xAOD::MdtDriftCircle*> driftCircleMap{};

        for (const xAOD::MdtDriftCircle* driftCircle : *readDriftCircles) {
            // need a small type conversion, as the UncalibratedMeasurement::identifier
            // returns a 32-bit unsigned long, while the Identifier::value_type is 64-bit
            Identifier driftCircleID{(Identifier::value_type)driftCircle->identifier()};
            auto empl_res = driftCircleMap.emplace(driftCircleID,driftCircle); 
            // report any found duplication
            if (!empl_res.second) {
                ATH_MSG_WARNING("Duplicate drift circle found for identifier "<<m_idHelperSvc->toString(driftCircleID));
                ATH_MSG_WARNING("  This instance is at r = "<<driftCircle->driftRadius());
                ATH_MSG_WARNING("  The conflicting instance is at r = "<<empl_res.first->second->driftRadius()); 
            }
        }
        // also track duplicate sim hits for early validation
        std::map<Identifier, const xAOD::MuonSimHit*> simHitIDsSeen;

        for (const xAOD::MuonSimHit* simHit : *readSimHits) {
            // The sim hit collection contains non-muon hits, while the fast digi only writes out muon 
            // deposits. Here remove sim hits that are not expected to be accounted for in the digi. 
            if (std::abs(simHit->pdgId()) != 13) continue;
            Identifier ID = simHit->identify();    
            // check for duplicates 
            auto empl_res = simHitIDsSeen.emplace(ID,simHit); 
            // report any found duplication
            if (!empl_res.second) {
                const xAOD::MuonSimHit* exist{empl_res.first->second};
                ATH_MSG_WARNING("Duplicate sim hit found for identifier "<<m_idHelperSvc->toString(ID));
                ATH_MSG_WARNING("  This instance is at r,z, time "
                                    <<simHit->localPosition().perp()
                                    <<", "<<simHit->localPosition().z()
                                    <<", "<<simHit->globalTime()
                                    <<" from a "<<simHit->pdgId()
                                    <<" with BC "<<simHit->genParticleLink().barcode()); 
                ATH_MSG_WARNING("  The conflicting instance is at r,z, time "
                                    <<exist->localPosition().perp()
                                    <<", "<<exist->localPosition().z()
                                    <<", "<<exist->globalTime()                                    
                                    <<" from a "<<exist->pdgId()
                                    <<" with BC "<<exist->genParticleLink().barcode()); 
            }
            const MdtIdHelper& mdtHelper{m_idHelperSvc->mdtIdHelper()};
            // populate initial output
            m_out_stationName = m_idHelperSvc->stationName(ID); 
            m_out_stationEta = m_idHelperSvc->stationEta(ID); 
            m_out_stationPhi = m_idHelperSvc->stationPhi(ID); 
            m_out_multilayer = mdtHelper.multilayer(ID); 
            m_out_tubeLayer = mdtHelper.tubeLayer(ID); 
            m_out_tube = mdtHelper.tube(ID); 
            m_out_simDriftRadius = simHit->localPosition().perp(); 
            m_out_barcode = simHit->genParticleLink().barcode();
            // find the corresponding drift circle
            auto foundDriftCircle = driftCircleMap.find(ID);
            if(foundDriftCircle != driftCircleMap.end()){
                const xAOD::MdtDriftCircle* dt = foundDriftCircle->second; 
                m_out_digiDriftRadius = dt->driftRadius();
                m_out_digiDriftRadiusCov = dt->driftRadiusCov();
                m_out_hasDigi = true;             
            } 
            // fill the tree
            ATH_CHECK(m_tree.fill(context)); 
        }
        return StatusCode::SUCCESS;
    }

}  // namespace MuonValR4
