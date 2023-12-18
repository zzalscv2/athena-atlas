
/*
Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigAFPToFHypoTool.h"

using namespace TrigCompositeUtils;

TrigAFPToFHypoTool::TrigAFPToFHypoTool(const std::string &type, const std::string &name, const IInterface *parent)
  : AthAlgTool(type, name, parent),
    m_decisionId(HLT::Identifier::fromToolName(name)) {}

StatusCode TrigAFPToFHypoTool::initialize()
{
  ATH_MSG_DEBUG("TrigAFPToFHypoTool::initialize()");
  ATH_MSG_DEBUG("m_deltaZCut: " << m_deltaZCut);
  ATH_MSG_DEBUG("m_acceptAll: " << m_acceptAll);
  return StatusCode::SUCCESS;
}

StatusCode TrigAFPToFHypoTool::decide(std::vector<AFPToFHypoToolInfo>& inputs) const
{
  ATH_MSG_DEBUG("Executing the TrigAFPToFHypoTool selection for chain: " << name());

  //Make decision here with input AFP vertex and dijet vertex
  for (AFPToFHypoToolInfo& info : inputs) {
    if (info.inputPassingChains.count(m_decisionId.numeric()) == 0) {
      ATH_MSG_VERBOSE("This DecisionObject is not active for this chain coming into this step");
      continue;
    }

    bool pass = true;

    ATH_MSG_DEBUG("AFP ToF vertex z position: " << info.afpVtx->position() 
                  << " ID vertex z position: " << info.idVtx->z() 
                  << " deltaZCut: " << m_deltaZCut);

    if( std::abs( info.afpVtx->position() - info.idVtx->z() ) > m_deltaZCut) pass = false; 

    if(pass or m_acceptAll) {
      addDecisionID(m_decisionId.numeric(), info.outputDecision);
      ATH_MSG_DEBUG("AFP ToF vertex accepted");
    }
    else ATH_MSG_DEBUG("AFP ToF vertex rejected");
  }

  return StatusCode::SUCCESS;
}
