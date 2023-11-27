/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMomentTools/JetConstituentFrac.h"

#include "xAODJet/JetAccessorMap.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include "xAODPFlow/FlowElement.h"

//**********************************************************************

JetConstituentFrac::JetConstituentFrac(const std::string& name)
: AsgTool(name) { }

//**********************************************************************

StatusCode JetConstituentFrac::initialize() {
  ATH_MSG_INFO("Initializing JetConstituentFrac " << name());
  
  if(m_jetContainerName.empty()){
    ATH_MSG_ERROR("JetConstituentFrac needs to have its input jet container configured!");
    return StatusCode::FAILURE;
  }
  
  m_neutralEFracKey = m_jetContainerName + "." + m_neutralEFracKey.key();
  m_chargePTFracKey = m_jetContainerName + "." + m_chargePTFracKey.key();
  m_chargeMFracKey = m_jetContainerName + "." + m_chargeMFracKey.key();

  ATH_CHECK(m_neutralEFracKey.initialize());
  ATH_CHECK(m_chargePTFracKey.initialize());
  ATH_CHECK(m_chargeMFracKey.initialize());

  return StatusCode::SUCCESS;
}

//**********************************************************************

StatusCode JetConstituentFrac::decorate(const xAOD::JetContainer& jets) const {
  ATH_MSG_VERBOSE("Begin decorating jets.");
  for(const xAOD::Jet* jet : jets) {
    
    fillConstituentFrac(*jet);

  }
  return StatusCode::SUCCESS;
}

void JetConstituentFrac::fillConstituentFrac(const xAOD::Jet& jet) const {

  float neutE=0;
  float chargPt=0;
  TLorentzVector chargM = {0., 0., 0., 0.};

  size_t numConstit = jet.numConstituents();
  for ( size_t i=0; i<numConstit; i++ ) {
    if(jet.rawConstituent(i)->type()!=xAOD::Type::FlowElement) {
      ATH_MSG_WARNING("Tried to call fillConstituentFrac with a jet constituent that is not a FlowElement!");
      continue;
    }
    const xAOD::FlowElement* constit = static_cast<const xAOD::FlowElement*>(jet.rawConstituent(i));

    if(constit->signalType() == xAOD::FlowElement::Neutral) neutE+= constit->e();    
    if (constit->signalType() == xAOD::FlowElement::Charged){
      chargPt += constit->pt();
      chargM += constit->p4();
    }
  }

  SG::WriteDecorHandle<xAOD::JetContainer, float> neutralEFracFracHandle(m_neutralEFracKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> chargePTFracFracHandle(m_chargePTFracKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> chargeMFracFracHandle(m_chargeMFracKey);
  
  neutralEFracFracHandle(jet)  = neutE/jet.e();
  chargePTFracFracHandle(jet)  = chargPt/jet.pt();
  chargeMFracFracHandle(jet)   = jet.m() != 0. ? chargM.M()/jet.m() : 0.;

}
