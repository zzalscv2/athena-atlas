/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigHIFwdGapHypoTool.h"

#include "GaudiKernel/SystemOfUnits.h"
using Gaudi::Units::GeV;

TrigHIFwdGapHypoTool::TrigHIFwdGapHypoTool(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent):
  base_class(type, name, parent),
  m_decisionId(HLT::Identifier::fromToolName(name)) {}


StatusCode TrigHIFwdGapHypoTool::initialize() {
  ATH_MSG_DEBUG("Initializing TrigHIFwdGapHypoTool for " << name());
  return StatusCode::SUCCESS;
}

StatusCode TrigHIFwdGapHypoTool::finalize() {
  return StatusCode::SUCCESS;
}


StatusCode TrigHIFwdGapHypoTool::decide(const xAOD::HIEventShapeContainer* eventShapeContainer, bool& pass) const {
  ATH_MSG_DEBUG("Executing decide() of " << name());

  float totalFCalEtSideA = 0.;
  float totalFCalEtSideC = 0.;
  for (const xAOD::HIEventShape* es: *eventShapeContainer) {
    const int layer = es->layer();
    if (layer < 21 or layer > 23) { //only use FCal information (calo samplings 21: FCAL0, 22: FCAL1, 23: FCAL2)
      continue;
    }

    const float et = es->et();
    if (std::abs(et) < 0.1) { //don't add negligible energy deposits (< 0.1 MeV)
      continue;
    }

    const float eta = 0.5 * (es->etaMin() + es->etaMax());
    if (eta < 0.) {
      totalFCalEtSideA += et;
    }
    else {
      totalFCalEtSideC += et;
    }
  }

  ATH_MSG_DEBUG("Total FCal ET: side A = " << totalFCalEtSideA << " MeV, side C = " << totalFCalEtSideC << " MeV");
  if (m_useDoubleSidedGap) {
    pass = (totalFCalEtSideA < m_maxFCalEt * GeV and totalFCalEtSideC < m_maxFCalEt * GeV);
    if (pass) ATH_MSG_DEBUG("Passed max FCal ET cut of " << m_maxFCalEt * GeV << " MeV\n");
  }
  else {
    if (m_useSideA) {
      pass = (totalFCalEtSideA < m_maxFCalEt * GeV);
      if (pass) ATH_MSG_DEBUG("Passed max FCal ET cut of " << m_maxFCalEt * GeV << " MeV on side A\n");
    }
    else {
      pass = (totalFCalEtSideC < m_maxFCalEt * GeV);
      if (pass) ATH_MSG_DEBUG("Passed max FCal ET cut of " << m_maxFCalEt * GeV << " MeV on side C\n");
    }
  }

  return StatusCode::SUCCESS;
}


const HLT::Identifier& TrigHIFwdGapHypoTool::getId() const {
  return m_decisionId;
}

