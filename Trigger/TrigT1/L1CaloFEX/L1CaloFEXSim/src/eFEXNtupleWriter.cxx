/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <L1CaloFEXSim/eFEXNtupleWriter.h>
#include "StoreGate/StoreGateSvc.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eFEXtauAlgo.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include <vector>
#include "TTree.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include <memory>

#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"

#include "TruthUtils/MagicNumbers.h"

LVL1::eFEXNtupleWriter::eFEXNtupleWriter(const std::string& name, ISvcLocator* pSvcLocator): AthAlgorithm(name, pSvcLocator) { }

LVL1::eFEXNtupleWriter::~eFEXNtupleWriter() {

}

StatusCode LVL1::eFEXNtupleWriter::initialize () {
  ServiceHandle<ITHistSvc> histSvc("THistSvc",name()); 
  CHECK( histSvc.retrieve() );
  m_myTree = new TTree("data","data");
  CHECK( histSvc->regTree("/ANALYSIS/data",m_myTree) );

  ATH_CHECK( m_eFEXOutputCollectionSGKey.initialize() );
  
  m_load_truth_jet = false;

  m_myTree->Branch ("truth_tauvisible_eta",  &m_truth_tauvisible_eta);
  m_myTree->Branch ("truth_tauvisible_phi",  &m_truth_tauvisible_phi);
  m_myTree->Branch ("truth_tauvisible_ET",  &m_truth_tauvisible_ET);

  m_myTree->Branch ("truth_e_eta",  &m_truth_e_eta);
  m_myTree->Branch ("truth_e_phi",  &m_truth_e_phi);
  m_myTree->Branch ("truth_e_ET",  &m_truth_e_ET);
  if (m_load_truth_jet){
    m_myTree->Branch ("truth_jet_eta",  &m_truth_jet_eta);
    m_myTree->Branch ("truth_jet_phi",  &m_truth_jet_phi);
    m_myTree->Branch ("truth_jet_ET",  &m_truth_jet_ET);
  }

  m_myTree->Branch ("em",  &m_em);
  m_myTree->Branch ("had",  &m_had);

  m_myTree->Branch ("eg_eta",  &m_eg_eta);
  m_myTree->Branch ("eg_phi",  &m_eg_phi);
  m_myTree->Branch ("eg_ET",  &m_eg_ET);
  m_myTree->Branch ("eg_wstotnum",  &m_eg_WstotNum);
  m_myTree->Branch ("eg_wstotden",  &m_eg_WstotDen);
  m_myTree->Branch ("eg_retanum",  &m_eg_RetaNum);
  m_myTree->Branch ("eg_retaden",  &m_eg_RetaDen);
  m_myTree->Branch ("eg_rhadnum",  &m_eg_RhadNum);
  m_myTree->Branch ("eg_rhadden",  &m_eg_RhadDen);
  m_myTree->Branch ("eg_haveSeed",  &m_eg_haveseed);
  m_myTree->Branch ("tau_RealRCore",  &m_tau_realRCore);
  m_myTree->Branch ("tau_RCoreCore",  &m_tau_rCoreCore);
  m_myTree->Branch ("tau_RCoreEnv",  &m_tau_rCoreEnv);
  m_myTree->Branch ("tau_RCoreWP",  &m_tau_rCoreWP);
  m_myTree->Branch ("tau_RealRHad",  &m_tau_realRHad);
  m_myTree->Branch ("tau_RHadCore",  &m_tau_rHadCore);
  m_myTree->Branch ("tau_RHadEnv",  &m_tau_rHadEnv);
  m_myTree->Branch ("tau_RHadWP",  &m_tau_rHadWP);
  m_myTree->Branch ("tau_Seed",  &m_tau_seed);
  m_myTree->Branch ("tau_UnD",  &m_tau_und);
  m_myTree->Branch ("tau_Et",  &m_tau_Et);
  m_myTree->Branch ("tau_Eta",  &m_tau_Eta);
  m_myTree->Branch ("tau_Phi",  &m_tau_Phi);
  m_myTree->Branch ("tau_floatEta",  &m_tau_floatEta);
  m_myTree->Branch ("tau_floatPhi",  &m_tau_floatPhi);
  m_myTree->Branch ("tau_isCentralTowerSeed",  &m_tau_isCentralTowerSeed);
  m_myTree->Branch ("tau_CenterTowerEt",  &m_tau_CenterTowerEt);
  m_myTree->Branch ("tau_OneOffEtaTowerEt",  &m_tau_OneOffEtaTowerEt);
  m_myTree->Branch ("tau_OneBelowEtaTowerEt",  &m_tau_OneBelowEtaTowerEt);
  m_myTree->Branch ("tau_eFEXID",  &m_tau_eFEXID);
  m_myTree->Branch ("tau_FPGAID",  &m_tau_FPGAID);
  m_myTree->Branch ("eFEXnumber",  &m_eFex_number);
  m_myTree->Branch ("eg_nTOBs",  &m_eg_nTOBs);

  m_myTree->Branch ("eg_TOB_FP", &m_eg_TOB_FP);
  m_myTree->Branch ("eg_TOB_Eta", &m_eg_TOB_Eta);
  m_myTree->Branch ("eg_TOB_Phi", &m_eg_TOB_Phi);
  m_myTree->Branch ("eg_TOB_ha", &m_eg_TOB_ha);
  m_myTree->Branch ("eg_TOB_f3", &m_eg_TOB_f3);
  m_myTree->Branch ("eg_TOB_Re", &m_eg_TOB_Re);
  m_myTree->Branch ("eg_TOB_Sd", &m_eg_TOB_Sd);
  m_myTree->Branch ("eg_TOB_UnD", &m_eg_TOB_UnD);
  m_myTree->Branch ("eg_TOB_Max", &m_eg_TOB_Max);
  m_myTree->Branch ("eg_TOB_zeros", &m_eg_TOB_zeros);
  m_myTree->Branch ("eg_TOB_energy", &m_eg_TOB_energy);
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::execute () {
  SG::ReadHandle<LVL1::eFEXOutputCollection> eFEXOutputCollectionobj = SG::ReadHandle<LVL1::eFEXOutputCollection>(m_eFEXOutputCollectionSGKey/*,ctx*/);
  if(!eFEXOutputCollectionobj.isValid()){
    ATH_MSG_FATAL("Could not retrieve eFEXOutputCollection " << m_eFEXOutputCollectionSGKey.key());
    return StatusCode::FAILURE;
  }
  if (!eFEXOutputCollectionobj->getdooutput()) {
    return StatusCode::SUCCESS; 
  }

  CHECK(loadegAlgoVariables(eFEXOutputCollectionobj));
  CHECK(loadegAlgoTOBs(eFEXOutputCollectionobj));
  CHECK(loadtauAlgoVariables(eFEXOutputCollectionobj));
  CHECK(loadTruthElectron());
  CHECK(loadTruthTau());
  if (m_load_truth_jet){
    CHECK(loadTruthJets());
  }

  m_myTree->Fill();

  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::finalize () {
  ATH_MSG_DEBUG("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::loadtauAlgoVariables(SG::ReadHandle<LVL1::eFEXOutputCollection> eFEXOutputCollectionobj) {
  m_tau_realRCore.clear();
  m_tau_rCoreCore.clear();
  m_tau_rCoreEnv.clear();
  m_tau_rCoreWP.clear();
  m_tau_realRHad.clear();
  m_tau_rHadCore.clear();
  m_tau_rHadEnv.clear();
  m_tau_rHadWP.clear();
  m_tau_seed.clear();
  m_tau_und.clear();
  m_tau_Et.clear();
  m_tau_Eta.clear();
  m_tau_Phi.clear();
  m_tau_floatEta.clear();
  m_tau_floatPhi.clear();
  m_tau_isCentralTowerSeed.clear();
  m_tau_CenterTowerEt.clear();
  m_tau_OneOffEtaTowerEt.clear();
  m_tau_OneBelowEtaTowerEt.clear();
  m_tau_eFEXID.clear();
  m_tau_FPGAID.clear();
  for (int i = 0; i < eFEXOutputCollectionobj->tau_size(); i++)
  {
    m_tau_isCentralTowerSeed.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["isCentralTowerSeed"]);
    m_tau_Et.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["Et"]);
    m_tau_Eta.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["Eta"]);
    m_tau_Phi.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["Phi"]);
    m_tau_floatEta.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["FloatEta"]);
    m_tau_floatPhi.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["FloatPhi"]);
    m_tau_realRCore.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RealRCore"]);
    m_tau_rCoreCore.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RCoreCore"]);
    m_tau_rCoreEnv.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RCoreEnv"]);
    m_tau_rCoreWP.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RCoreWP"]);
    m_tau_realRHad.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RealRHad"]);
    m_tau_rHadCore.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RHadCore"]);
    m_tau_rHadEnv.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RHadEnv"]);
    m_tau_rHadWP.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["RHadWP"]);
    m_tau_seed.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["Seed"]);
    m_tau_und.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["UnD"]);
    m_tau_CenterTowerEt.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["CenterTowerEt"]);
    m_tau_OneOffEtaTowerEt.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["OneOffEtaTowerEt"]);
    m_tau_OneBelowEtaTowerEt.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["OneBelowEtaTowerEt"]);
    m_tau_eFEXID.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["eFEXID"]);
    m_tau_FPGAID.push_back((*(eFEXOutputCollectionobj->get_tau(i)))["FPGAID"]);
  }
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::loadegAlgoVariables(SG::ReadHandle<LVL1::eFEXOutputCollection> eFEXOutputCollectionobj) {
  m_eg_ET.clear();
  m_eg_WstotNum.clear();
  m_eg_WstotDen.clear();
  m_eg_eta.clear();
  m_eg_phi.clear();
  m_eg_haveseed.clear();
  m_eg_RetaNum.clear();
  m_eg_RetaDen.clear();
  m_eg_RhadNum.clear();
  m_eg_RhadDen.clear();
  m_eFex_number.clear();
  m_em.clear();
  m_had.clear();

  m_eg_nTOBs = eFEXOutputCollectionobj->size();
  for (int i = 0; i < eFEXOutputCollectionobj->size(); i++)
  {
    std::map<std::string, float> eFEXegvalue_tem = (*(eFEXOutputCollectionobj->get_eg(i)));
    m_eg_WstotNum.push_back(eFEXegvalue_tem["WstotNum"]);
    m_eg_WstotDen.push_back(eFEXegvalue_tem["WstotDen"]);
    m_eg_RetaNum.push_back(eFEXegvalue_tem["RetaNum"]);
    m_eg_RetaDen.push_back(eFEXegvalue_tem["RetaDen"]);
    m_eg_RhadNum.push_back(eFEXegvalue_tem["RhadNum"]);
    m_eg_RhadDen.push_back(eFEXegvalue_tem["RhadDen"]);
    m_eg_haveseed.push_back(eFEXegvalue_tem["haveSeed"]);
    m_eg_ET.push_back(eFEXegvalue_tem["ET"]);
    m_eg_eta.push_back(eFEXegvalue_tem["eta"]);
    m_eg_phi.push_back(eFEXegvalue_tem["phi"]);
    m_em.push_back(eFEXegvalue_tem["em"]);
    m_had.push_back(eFEXegvalue_tem["had"]);
  }
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::loadegAlgoTOBs(SG::ReadHandle<LVL1::eFEXOutputCollection> eFEXOutputCollectionobj) {
  m_eg_TOB_FP.clear();
  m_eg_TOB_Eta.clear();
  m_eg_TOB_Phi.clear();
  m_eg_TOB_ha.clear();
  m_eg_TOB_f3.clear();
  m_eg_TOB_Re.clear();
  m_eg_TOB_Sd.clear();
  m_eg_TOB_UnD.clear();
  m_eg_TOB_Max.clear();
  m_eg_TOB_zeros.clear();
  m_eg_TOB_energy.clear();
  for (int i = 0; i < eFEXOutputCollectionobj->size(); i++)
  {
    uint32_t TOB = eFEXOutputCollectionobj->getEMtob()[i];
    uint32_t FP = getbits(TOB, 1, 2);
    uint32_t Eta = getbits(TOB, 3, 5);
    uint32_t Phi = getbits(TOB, 6, 8);
    uint32_t ha = getbits(TOB, 9, 10);
    uint32_t f3 = getbits(TOB, 11, 12);
    uint32_t Re = getbits(TOB, 13, 14);
    uint32_t Sd = getbits(TOB, 15, 16);
    uint32_t UnD = getbits(TOB, 17, 17);
    uint32_t Max = getbits(TOB, 18, 18);
    uint32_t zeros = getbits(TOB, 19, 20);
    uint32_t energy = getbits(TOB, 21, 32);

    m_eg_TOB_FP.push_back(FP);
    m_eg_TOB_Eta.push_back(Eta);
    m_eg_TOB_Phi.push_back(Phi);
    m_eg_TOB_ha.push_back(ha);
    m_eg_TOB_f3.push_back(f3);
    m_eg_TOB_Re.push_back(Re);
    m_eg_TOB_Sd.push_back(Sd);
    m_eg_TOB_UnD.push_back(UnD);
    m_eg_TOB_Max.push_back(Max);
    m_eg_TOB_zeros.push_back(zeros);
    m_eg_TOB_energy.push_back(energy * 100);
  }
  m_eFex_number = eFEXOutputCollectionobj->geteFexNumber();
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::loadTruthElectron() {
  m_truth_e_eta.clear();
  m_truth_e_phi.clear();
  m_truth_e_ET.clear();
  const xAOD::TruthEventContainer* truthEvents;
  CHECK(evtStore()->retrieve( truthEvents, "TruthEvents"));
  for(auto ite : *truthEvents) {
    int nParticle = ite->nTruthParticles();
    for(int i = 0; i < nParticle; i++){
      const xAOD::TruthParticle* each_particle = ite->truthParticle(i);

      // ignore geant4
      if(HepMC::is_simulation_particle(each_particle)) continue;
      // select particles that is not decayed further by the generator
      if(each_particle->status() != 1) continue;
      // select electrons
      if(fabs(each_particle->pdgId()) != 11) continue;
      // select particles from Z
      if(!getMother(each_particle, 23)) continue;

      m_truth_e_ET.push_back(each_particle->p4().Pt());
      m_truth_e_eta.push_back(each_particle->p4().Eta());
      m_truth_e_phi.push_back(each_particle->p4().Phi());
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::loadTruthJets() {
  m_truth_jet_eta.clear();
  m_truth_jet_phi.clear();
  m_truth_jet_ET.clear();
  const xAOD::JetContainer* truth_jets;
  StatusCode sc = evtStore()->retrieve( truth_jets, m_jet_container_name);
  if (sc ==  StatusCode::FAILURE){
    m_jet_container_name = "InTimeAntiKt4TruthJets";
    StatusCode sc2 = evtStore()->retrieve( truth_jets, m_jet_container_name);
    if (sc2 ==  StatusCode::FAILURE){
      ATH_MSG_DEBUG("eFEXNtupleWriter::loadTruthJets() Unable to determine truth jet container");
      m_load_truth_jet = false;
      return StatusCode::SUCCESS;
    }
  }
  for (unsigned i=0; i!=truth_jets->size(); i++) {
    const xAOD::Jet* each_jet = (*truth_jets)[i];
    if(each_jet->pt()<10000) continue;
    m_truth_jet_eta.push_back(each_jet->p4().Eta());
    m_truth_jet_phi.push_back(each_jet->p4().Phi());
    m_truth_jet_ET.push_back(each_jet->p4().Et());
  }
  return StatusCode::SUCCESS;
}

StatusCode LVL1::eFEXNtupleWriter::loadTruthTau() {
  m_truth_tauvisible_eta.clear();
  m_truth_tauvisible_phi.clear();
  m_truth_tauvisible_ET.clear();
  const xAOD::TruthEventContainer* truthEvents;
  CHECK( evtStore()->retrieve( truthEvents, "TruthEvents"));
  for (auto ite : *truthEvents) {
    int nParticle = ite->nTruthParticles();
    for(int i = 0; i < nParticle; i++) {
      const xAOD::TruthParticle* each_particle = ite->truthParticle(i);
      // ignore geant4
      if(HepMC::is_simulation_particle(each_particle)) continue;
      // select final state particles and decaying hadrons, muons or taus
      if (each_particle->status() != 1 && each_particle->status() != 2) continue;
      // select tau
      if (fabs(each_particle->pdgId()) != 15) continue;
      std::unique_ptr<TLorentzVector> p4_visible = visibleTauP4(each_particle);

      if (!p4_visible) break;
      m_truth_tauvisible_eta.push_back(p4_visible->Eta());
      m_truth_tauvisible_phi.push_back(p4_visible->Phi());
      m_truth_tauvisible_ET.push_back(p4_visible->Et());
    }
  }
  return StatusCode::SUCCESS;
}

std::unique_ptr<TLorentzVector> LVL1::eFEXNtupleWriter::visibleTauP4(const xAOD::TruthParticle* particle) {
  std::unique_ptr<TLorentzVector> psum(new TLorentzVector(0,0,0,0));
  // ignore documentation particles. Attempt to find the nOutgoingParticles() of a documentation particle 
  // causes crash.
  // It still gives the correct result, but I don't know why I have to do this.
  if (particle->status() == 3) {
    return psum;
  }
  const xAOD::TruthVertex* decay_vertex = particle->decayVtx();
  decay_vertex->nOutgoingParticles();
  for(uint i=0; i < decay_vertex->nOutgoingParticles(); i++) {
    const xAOD::TruthParticle* each_particle = decay_vertex->outgoingParticle(i);
    int pid = fabs(each_particle->pdgId());
    // particle that is not decayed further by the generator
    if (each_particle->status() == 1) {
      // ignore neutrinos
      if (pid ==  12 || pid ==  14 || pid ==  16) continue;
      // ignore leptonic decay events
      if (pid ==  11 || pid ==  13) return std::unique_ptr<TLorentzVector>(nullptr);
      (*psum) += each_particle->p4();
    }
    else{
      std::unique_ptr<TLorentzVector> p4_tem = visibleTauP4(each_particle);
      if (!p4_tem) return std::unique_ptr<TLorentzVector>(nullptr);
      (*psum) += (*p4_tem);
    }
  }
  return psum;
}

std::unique_ptr<TLorentzVector> LVL1::eFEXNtupleWriter::invisibleTauP4(const xAOD::TruthParticle* particle) {
  std::unique_ptr<TLorentzVector> psum(new TLorentzVector(0,0,0,0));
  // ignore documentation particles. Attempt to find the nOutgoingParticles() of a documentation particle 
  // causes crash.
  // It still gives the correct result, but I don't know why I have to do this.
  if (particle->status() == 3) {
    return psum;
  }
  const xAOD::TruthVertex* decay_vertex = particle->decayVtx();
  for (uint i=0; i < decay_vertex->nOutgoingParticles(); i++) {
    const xAOD::TruthParticle* each_particle = decay_vertex->outgoingParticle(i);
    int pid = fabs(each_particle->pdgId());
    // particle that is not decayed further by the generator
    if (each_particle->status() == 1) {
      // ignore leptonic decay events
      if (pid ==  11 || pid ==  13) return std::unique_ptr<TLorentzVector>(nullptr);
      // select neutrinos
      if (pid ==  12 || pid ==  14 || pid ==  16) (*psum) += each_particle->p4();
    }
    else {
      std::unique_ptr<TLorentzVector> p4_tem = invisibleTauP4(each_particle);
      if (!p4_tem) return std::unique_ptr<TLorentzVector>(nullptr);
      (*psum) += (*p4_tem);
    }
  }
  return psum;
}

const xAOD::TruthParticle* LVL1::eFEXNtupleWriter::getMother(const xAOD::TruthParticle* particle, int motherPid) {
   const xAOD::TruthVertex* productionVector = particle->prodVtx();
   if (!productionVector) return NULL;
   for (long unsigned int i = 0; i < productionVector->nIncomingParticles(); i++) {
      const xAOD::TruthParticle* mother = productionVector->incomingParticle(i);
      if (mother->pdgId()==motherPid) return mother;
      const xAOD::TruthParticle* grandmother = getMother(mother, motherPid);
      if (grandmother) return grandmother;
   }
   return NULL;
}

uint32_t LVL1::eFEXNtupleWriter::getbits(uint32_t in, int start, int end) {
    in <<= start - 1;
    in >>= 32 - end + start - 1;
    return in;
}
