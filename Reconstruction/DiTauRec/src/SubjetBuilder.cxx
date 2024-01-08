/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/SubjetBuilder.h"
#include "DiTauRec/DiTauToolBase.h"
#include "DiTauRec/DiTauCandidateData.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetConstituentVector.h"

// fastjet includes
#include "fastjet/Selector.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/tools/Filter.hh"
#include "fastjet/tools/Pruner.hh"


SubjetBuilder::SubjetBuilder(const std::string& type,
			     const std::string& name,
			     const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_Rsubjet(0.2),
  m_ptmin(10000)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty("Rsubjet", m_Rsubjet);
  declareProperty("ptminsubjet", m_ptmin);
}


SubjetBuilder::~SubjetBuilder() = default;


StatusCode SubjetBuilder::initialize() {

  return StatusCode::SUCCESS;
}


StatusCode SubjetBuilder::execute(DiTauCandidateData * data,
                                  const EventContext& /*ctx*/) const {

  ATH_MSG_DEBUG("subjet builder executing...");

  // get seed jet
  xAOD::DiTauJet* pDiTau = data->xAODDiTau;

  if (pDiTau == nullptr) {
    ATH_MSG_ERROR("no di-tau candidate given");
    return StatusCode::FAILURE;
  }

  const xAOD::Jet* pSeed = data->seed;
  if (!pSeed) {
    ATH_MSG_WARNING("no seed given. Di-Tau can not be reconstructed.");
    return StatusCode::FAILURE;
  }

  // retrieve seed jet clusters 
	
  using namespace fastjet;
  const xAOD::JetConstituentVector vConst = pSeed->getConstituents();

  if (vConst.empty()) {
    ATH_MSG_WARNING("cluster constituents could not be retrieved from seed jet");
    return StatusCode::FAILURE;
  }

  std::vector<PseudoJet> vpjClusters;

  for (const auto *cl: vConst) {

    TLorentzVector temp_p4;
    temp_p4.SetPtEtaPhiM(cl->pt(), cl->eta(), cl->phi(), cl->m());
    PseudoJet c( temp_p4.Px(), temp_p4.Py(), temp_p4.Pz(), temp_p4.E());

    vpjClusters.push_back(c);
  }

  // reconstruct subjets

  // Jet and area definitions
  JetDefinition jd = JetDefinition(antikt_algorithm, m_Rsubjet);
  AreaDefinition area_def(active_area_explicit_ghosts,GhostedAreaSpec(SelectorAbsRapMax(4.0)));
  ClusterSequenceArea cs(vpjClusters, jd, area_def); 

  // store (pt-sorted) subjets
  std::vector<PseudoJet> vSubjets = sorted_by_pt( cs.inclusive_jets(m_ptmin) );

  if (vSubjets.empty()) {
    ATH_MSG_DEBUG("Found no subjet. Reject ditau candidate");
    return StatusCode::FAILURE;
  }
  if (vSubjets.size()==1) {
    if (data->electrons.empty() && data->muons.empty()) {
      ATH_MSG_DEBUG("Found 1 subjet, but no additional electron or muon. Reject ditau candidate");
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_DEBUG("found "<< vSubjets.size() << " subjets");
  for (const auto& subjet: vSubjets) {
    ATH_MSG_DEBUG("pt: " << subjet.pt() << "  eta: " << subjet.eta() << "  phi: " << subjet.phi());
  }

  data->subjets = vSubjets;
  vSubjets.clear();

  return StatusCode::SUCCESS;
}
