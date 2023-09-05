/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonChamberToolTest.h"

#include <StoreGate/ReadCondHandle.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>

 namespace MuonGMR4 {

MuonChamberToolTest::MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator):
      AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode MuonChamberToolTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_chambTool.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}

StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
    SG::ReadCondHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
    if (!gctx.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the Acts alignment "<<m_geoCtxKey.fullKey());
        return StatusCode::FAILURE;
    }
    ChamberSet chambers = m_chambTool->buildChambers();
    std::vector<const MuonReadoutElement*> elements = m_detMgr->getAllReadoutElements();
    for (const MuonReadoutElement* readOut : elements) {
        ChamberSet::const_iterator itr = chambers.find(*readOut);
        if (itr == chambers.end()) {
            ATH_MSG_FATAL("The element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                         <<" is not attributed with any chamber");
            return StatusCode::FAILURE;
        }
        const MuonChamber& chamber{*itr};
        ATH_MSG_INFO("Muon chamber "<<m_idHelperSvc->mdtIdHelper().stationNameString(chamber.stationIndex())
                                    <<", eta: "<<chamber.stationEta()<<", phi: "<<chamber.stationPhi()
                                    <<" "<<m_idHelperSvc->toStringChamber(readOut->identify()));
        
        std::shared_ptr<Acts::Volume> chambVol = chamber.boundingVolume(**gctx);
        if (readOut->detectorType() == ActsTrk::DetectorType::Mdt) {
            const MdtReadoutElement* mdtMl = static_cast<const MdtReadoutElement*>(readOut);
            if (!chambVol->inside(mdtMl->center(**gctx))){
                ATH_MSG_FATAL("The Mdt "<<m_idHelperSvc->toStringDetEl(mdtMl->identify())<<" is outside the chamber volume");
                return StatusCode::FAILURE;
            }
        }
    }
    return StatusCode::SUCCESS;
}

}