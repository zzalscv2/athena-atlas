/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetRec/JetDecorationAlg.h"
#include "AsgDataHandles/ReadHandle.h"


StatusCode JetDecorationAlg::initialize() {

  ATH_CHECK(m_jetKey.initialize());
  if (m_decorators.empty()){
     ATH_MSG_FATAL("No decorators were given");
     return StatusCode::FAILURE;
  }
  ATH_CHECK(m_decorators.retrieve());
  ATH_MSG_INFO("Initialize .... List of decorators:");
  for(const ToolHandle<IJetDecorator>& t : m_decorators){
    ATH_MSG_INFO("    --> : "<< t->name());
  }

  return StatusCode::SUCCESS;
}


StatusCode JetDecorationAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::JetContainer> jetHandle(m_jetKey, ctx);

  ATH_MSG_DEBUG("Applying jet decorators to " << m_jetKey.key());
  for(const ToolHandle<IJetDecorator>& t : m_decorators){
    ATH_MSG_DEBUG("Running " << t.name());
    ATH_CHECK(t->decorate(*jetHandle));
  }

  return StatusCode::SUCCESS;

}
