
/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtPrepDataToxAODCnvAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"

MdtPrepDataToxAODCnvAlg::MdtPrepDataToxAODCnvAlg(const std::string& name,
                                                 ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MdtPrepDataToxAODCnvAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_inputKey.initialize());
    ATH_CHECK(m_outputKey.initialize());
    ATH_CHECK(m_detMgr.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MdtPrepDataToxAODCnvAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<Muon::MdtPrepDataContainer> inputContainer{m_inputKey, ctx};
    if (!inputContainer.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve Mdt prepdata collection "
                      << m_inputKey.fullKey());
        return StatusCode::FAILURE;
    }

    SG::WriteHandle<xAOD::MdtDriftCircleContainer> outputContainer{m_outputKey,
                                                                   ctx};
    ATH_CHECK(outputContainer.record(
        std::make_unique<xAOD::MdtDriftCircleContainer>(),
        std::make_unique<xAOD::MdtDriftCircleAuxContainer>()));

    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    /// Loop over all collections and prepdata objects to convert them to
    /// xAOD::DriftCicles
    for (const Muon::MdtPrepDataCollection* mdt_coll : *inputContainer) {
        for (const Muon::MdtPrepData* prd : *mdt_coll) {
            xAOD::MdtDriftCircle* new_circle = new xAOD::MdtDriftCircle();
            outputContainer->push_back(new_circle);
            /// Electronic properties
            new_circle->setAdc(prd->adc());
            new_circle->setTdc(prd->tdc());
            new_circle->setStatus(prd->status());
            /// identifier properties
            const Identifier id = prd->identify();
            new_circle->setLayer(idHelper.tubeLayer(id));
            new_circle->setTube(idHelper.tube(id));
            /// local position / covariance / hash
            new_circle->setDriftRadius(prd->localPosition()[Trk::locR]);
            new_circle->setDriftRadCov(
                prd->localCovariance()(Trk::locR, Trk::locR));
            new_circle->setIdentifierHash(
                prd->detectorElement()->detectorElementHash());
        }
    }
    return StatusCode::SUCCESS;
}
