/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/IDVarCalculator.h"
#include "DiTauRec/DiTauToolBase.h"
#include "DiTauRec/DiTauCandidateData.h"

#include "fastjet/PseudoJet.hh"

IDVarCalculator::IDVarCalculator(const std::string& type,
				 const std::string& name,
				 const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_useCells(true)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty("useCells", m_useCells);
}


IDVarCalculator::~IDVarCalculator() = default;


StatusCode IDVarCalculator::initialize() {

  return StatusCode::SUCCESS;
}


StatusCode IDVarCalculator::execute(DiTauCandidateData * data,
                                    const EventContext& /*ctx*/) const {

  ATH_MSG_DEBUG("execute IDVarCalculator...");

  // get ditau elements

  // ditau
  xAOD::DiTauJet* pDiTau = data->xAODDiTau;
  if (!pDiTau) {
    ATH_MSG_ERROR("no di-tau candidate given");
    return StatusCode::FAILURE;
  }

  // seed jet
  const xAOD::Jet* pSeed = data->seed;
  if (!pSeed) {
    ATH_MSG_WARNING("No jet seed given.");
    return StatusCode::FAILURE;
  }

  // subjets
  std::vector<fastjet::PseudoJet> vSubjets = data->subjets;
  if (vSubjets.empty()) {
    ATH_MSG_WARNING("No subjets given. Continue without ID variable calculation.");
    return StatusCode::SUCCESS;
  }

  // cells if available 
  bool useCells = m_useCells;;
  std::vector<const CaloCell*> vSubjetCells = data->subjetCells;
  if (vSubjetCells.empty()) {
    ATH_MSG_DEBUG("No cell information available.");
    useCells = false; 
  } 

  // write subjets
  for (unsigned int i = 0; i < vSubjets.size(); i++) {
    const fastjet::PseudoJet& subjet = vSubjets.at(i);
    pDiTau->setSubjetPtEtaPhiE(i, subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.e());
    ATH_MSG_DEBUG("subjet " << i << " pt: " << subjet.pt() << " eta: " << subjet.eta() << " phi: " << subjet.phi_std() << " e: " << subjet.e());
  }

  // write f_core
  if (!useCells) {
    ATH_MSG_DEBUG("no cells are used for ID variable calculation. Continue.");
    return StatusCode::SUCCESS;
  }

  float f_core;
  for (unsigned int i = 0; i < vSubjets.size(); i++) {
    const fastjet::PseudoJet& subjet = vSubjets.at(i);
    float ptAll = 0.;
    float ptCore = 0.;

    TLorentzVector temp_sub_p4;
    temp_sub_p4.SetPtEtaPhiM(subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.m());

    for (const auto& cc : vSubjetCells) {
     
      TLorentzVector temp_cc_p4;
      temp_cc_p4.SetPtEtaPhiM(cc->pt(), cc->eta(), cc->phi(), cc->m()); 

      if (temp_cc_p4.DeltaR(temp_sub_p4) < data->Rsubjet) {
	ptAll += cc->pt();
      }

      if (temp_cc_p4.DeltaR(temp_sub_p4) < data->Rcore) {
	ptCore += cc->pt();
      }
    }

    // FIXME: why require ptCore != 0
    if (ptAll != 0. && ptCore != 0.)
      f_core = ptCore/ptAll;
    else 
      f_core = -999.;

    ATH_MSG_DEBUG("subjet "<< i << ": f_core=" << f_core);
    pDiTau->setfCore(i, f_core);
  }

  return StatusCode::SUCCESS;
}
