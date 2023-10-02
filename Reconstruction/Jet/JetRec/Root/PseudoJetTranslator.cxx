/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#include "JetRec/PseudoJetTranslator.h"


xAOD::Jet& PseudoJetTranslator::translate(const fastjet::PseudoJet& pj,
					  const PseudoJetContainer& pjCont,
					  xAOD::JetContainer& jetCont, const xAOD::Vertex* originVertex) const {

  // Create a new jet in place at the end of the container
  jetCont.emplace_back(new xAOD::Jet());
  xAOD::Jet& jet = *jetCont.back();
  jet.setJetP4( xAOD::JetFourMom_t( pj.pt(), pj.eta(), pj.phi(), pj.m() ) );

  const static SG::AuxElement::Accessor<const fastjet::PseudoJet*> pjAccessor("PseudoJet");
  pjAccessor(jet) = &pj;

  // Record the jet-finding momentum, i.e. the one used to find/groom the jet.
  jet.setJetP4(xAOD::JetConstitScaleMomentum, jet.jetP4());
  
  // save area if needed ---------
  if( pj.has_area() ){

    if(m_saveArea) jet.setAttribute(xAOD::JetAttribute::ActiveArea,pj.area());
    if(m_saveArea4Vec){
        fastjet::PseudoJet pja = pj.area_4vector();
        xAOD::JetFourMom_t fvarea(pja.pt(), pja.eta(), pja.phi(), pja.m());
        jet.setAttribute(xAOD::JetAttribute::ActiveArea4vec, fvarea);      
    }    
  }// area -------------
  
  if (originVertex == nullptr){
    pjCont.extractConstituents(jet, pj);
  }
  else{
    pjCont.extractByVertexConstituents(jet, pj, originVertex);
  }

  return jet;
}

xAOD::Jet& PseudoJetTranslator::translate(const fastjet::PseudoJet& pj,
					  const PseudoJetContainer& pjCont,
					  xAOD::JetContainer& jetCont,
					  const xAOD::Jet &parent, const xAOD::Vertex* originVertex) const {
  xAOD::Jet& jet = translate(pj, pjCont, jetCont, originVertex);

  const xAOD::JetContainer* parentCont = dynamic_cast<const xAOD::JetContainer*>(parent.container());
  if ( parentCont == 0 ) { return jet ;}  // can this happen? if so THIS IS an ERROR ! should do something

  ElementLink<xAOD::JetContainer> el(*parentCont, parent.index());
  static const SG::AuxElement::Accessor<ElementLink<xAOD::JetContainer> > parentELacc("Parent"); 
  parentELacc(jet) =el;

  jet.setInputType(parent.getInputType());
  jet.setAlgorithmType(parent.getAlgorithmType());
  jet.setSizeParameter(parent.getSizeParameter());
  jet.setConstituentsSignalState(parent.getConstituentsSignalState());

  float area=0;
  if(parent.getAttribute(xAOD::JetAttribute::JetGhostArea, area)) jet.setAttribute(xAOD::JetAttribute::JetGhostArea, area);
    
  return jet;
}
