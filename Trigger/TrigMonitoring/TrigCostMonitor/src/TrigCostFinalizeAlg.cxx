/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCostFinalizeAlg.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"


TrigCostFinalizeAlg::TrigCostFinalizeAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}


StatusCode TrigCostFinalizeAlg::initialize() {
    ATH_MSG_DEBUG("TrigCostFinalizeAlg start");
    ATH_CHECK(m_trigCostSvcHandle.retrieve());

    ATH_CHECK( m_costWriteHandleKey.initialize() );
    ATH_CHECK( m_rosWriteHandleKey.initialize() );

    return StatusCode::SUCCESS;
}


StatusCode TrigCostFinalizeAlg::execute (const EventContext& context) const {
    ATH_MSG_DEBUG("TrigCostFinalizeAlg execute");

    SG::WriteHandle<xAOD::TrigCompositeContainer> costMonOutput = TrigCompositeUtils::createAndStore(m_costWriteHandleKey, context);
    SG::WriteHandle<xAOD::TrigCompositeContainer> rosMonOutput = TrigCompositeUtils::createAndStore(m_rosWriteHandleKey, context);
    ATH_CHECK(m_trigCostSvcHandle->endEvent(context, costMonOutput, rosMonOutput));

    return StatusCode::SUCCESS;
}