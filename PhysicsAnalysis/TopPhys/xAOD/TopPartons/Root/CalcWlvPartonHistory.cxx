/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#include "TopPartons/CalcWlvPartonHistory.h"
#include "TopConfiguration/TopConfig.h"
#include "TopPartons/PartonHistoryUtils.h"

namespace top {
  using PartonHistoryUtils::decorateWithMPtPhi;
  CalcWlvPartonHistory::CalcWlvPartonHistory(const std::string& name) : CalcTopPartonHistory(name) {}

  void CalcWlvPartonHistory::WlvHistorySaver(const xAOD::TruthParticleContainer* truthParticles,
                                             xAOD::PartonHistory* wlvPartonHistory) {
    wlvPartonHistory->IniVarWlv();

    TLorentzVector W;
    TLorentzVector WDecay1;
    TLorentzVector WDecay2;
    int WDecay1_pdgId{};
    int WDecay2_pdgId{};
    bool goodevent = CalcTopPartonHistory::Wlv(truthParticles, W, WDecay1, WDecay1_pdgId, WDecay2, WDecay2_pdgId);


    if (goodevent) {
      decorateWithMPtPhi(wlvPartonHistory, "MC_W",W);
      fillEtaBranch(wlvPartonHistory, "MC_W_eta", W);

      decorateWithMPtPhi(wlvPartonHistory, "MC_l",WDecay1);
      wlvPartonHistory->auxdecor< int >("MC_l_pdgId") = WDecay1_pdgId;
      fillEtaBranch(wlvPartonHistory, "MC_l_eta", WDecay1);

      decorateWithMPtPhi(wlvPartonHistory, "MC_v",WDecay2);
      wlvPartonHistory->auxdecor< int >("MC_v_pdgId") = WDecay2_pdgId;
      fillEtaBranch(wlvPartonHistory, "MC_v_eta", WDecay2);
    }//if
  }

  StatusCode CalcWlvPartonHistory::execute() {
    // Get the Truth Particles
    const xAOD::TruthParticleContainer* truthParticles(nullptr);

    if(m_config->getDerivationStream() == "PHYS") //in DAOD_PHYS we don't have the truth particles container
    {
      // To obtain the W, leptons and the neutrino, we need the collections for all
      std::vector<std::string> collections = {"HardScatterParticles", "TruthBosonsWithDecayParticles", "TruthNeutrinos"};
      ATH_CHECK(buildContainerFromMultipleCollections(collections,"AT_WlvPartonHistory_TruthParticles"));
      ATH_CHECK(evtStore()->retrieve(truthParticles, "AT_WlvPartonHistory_TruthParticles"));
      
      //we need to be able to navigate from the Ws to their decayProducts, see CalcTopPartonHistory.h for details
      ATH_CHECK(linkBosonCollections());
    }
    else //otherwise we retrieve the container as usual
    {
      ATH_CHECK(evtStore()->retrieve(truthParticles, m_config->sgKeyMCParticle()));
    }

    // Create the partonHistory xAOD object
    //cppcheck-suppress uninitvar
    xAOD::PartonHistoryAuxContainer* partonAuxCont = new xAOD::PartonHistoryAuxContainer {};
    //cppcheck-suppress uninitvar
    xAOD::PartonHistoryContainer* partonCont = new xAOD::PartonHistoryContainer {};
    partonCont->setStore(partonAuxCont);
    //cppcheck-suppress uninitvar
    xAOD::PartonHistory* wlvPartonHistory = new xAOD::PartonHistory {};
    partonCont->push_back(wlvPartonHistory);

    // Recover the parton history for wlv events
    WlvHistorySaver(truthParticles, wlvPartonHistory);

    // Save to StoreGate / TStore
    std::string outputSGKey = m_config->sgKeyTopPartonHistory();
    std::string outputSGKeyAux = outputSGKey + "Aux.";

    StatusCode save = evtStore()->tds()->record(partonCont, outputSGKey);
    StatusCode saveAux = evtStore()->tds()->record(partonAuxCont, outputSGKeyAux);
    if (!save || !saveAux) {
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }
}
