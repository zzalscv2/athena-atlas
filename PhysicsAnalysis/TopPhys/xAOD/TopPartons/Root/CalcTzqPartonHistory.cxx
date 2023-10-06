/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TopPartons/CalcTzqPartonHistory.h"
#include "TopPartons/CalcTopPartonHistory.h"
#include "TopConfiguration/TopConfig.h"
#include "TopPartons/PartonHistoryUtils.h"


namespace top {
  using PartonHistoryUtils::decorateWithMPtPhi;

  CalcTzqPartonHistory::CalcTzqPartonHistory(const std::string& name) : CalcTopPartonHistory(name) {}
  const xAOD::TruthParticle* CalcTzqPartonHistory::findAfterGamma(const xAOD::TruthParticle* particle) {
    bool isAfterGamma(false);
    const int particle_ID = particle->pdgId();
    int forLoop;

    while (!isAfterGamma) {
      forLoop = 0;
      for (size_t j = 0; j < particle->nChildren(); j++) {
        const xAOD::TruthParticle* tmp_children = particle->child(j);
        if (tmp_children && tmp_children->pdgId() == particle_ID && tmp_children->pdgId() != 22) {
          particle = particle->child(j);
          forLoop++;
          break;
        }//if
      }//for

      if (forLoop == 0) isAfterGamma = true;
    }//while
    return particle;
  }

  const xAOD::TruthParticle* CalcTzqPartonHistory::getFlavourSibling(const xAOD::TruthParticle* particle)
  {
    const auto & parent = particle->parent(0);
    if (!parent) return nullptr;

    for (size_t i=0; i<parent->nChildren(); ++i)
      {
        const auto & sibling_candidate = parent->child(i);
        if (!sibling_candidate) continue;
        if (sibling_candidate->pdgId() == -particle->pdgId())
          {
            return sibling_candidate;
          }
      }

    return nullptr;
  }

  int CalcTzqPartonHistory::sign(int a) {
    if (a < 0) {
      return -1;
    } else return 1;
  }

  bool CalcTzqPartonHistory::bottom(const xAOD::TruthParticleContainer* truthParticles, int start) {
    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != start) {
        continue;
      }
      tZ.b_p4 = particle->p4();
      tZ.b_pdgId = particle->pdgId();
      return true;
    }
    return false;
  }

  bool CalcTzqPartonHistory::Zllqq(const xAOD::TruthParticleContainer* truthParticles, int start) {

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != start || particle->nChildren() != 2) {
        continue;
      }
      tZ.Z_p4 = particle->p4();
      
      for (size_t k = 0; k < particle->nChildren(); k++) {
		
        const xAOD::TruthParticle* ZChildren = particle->child(k);
        if (ZChildren->pdgId() > 0) {
	  tZ.Zdecay1_p4 = ZChildren->p4();
	  tZ.Zdecay1_pdgId = ZChildren->pdgId();
	  tZ.Zdecay1_status = ZChildren->status();
        } else {
	  tZ.Zdecay2_p4 = ZChildren->p4();
	  tZ.Zdecay2_pdgId = ZChildren->pdgId();
	  tZ.Zdecay1_status = ZChildren->status();
        }
       } //for  
       // here we ask to return true if it identifies the particle and its children correctly.
      return true; 
    }
    return false;  
  }

  bool CalcTzqPartonHistory::FindLostZ(const xAOD::TruthParticleContainer* truthParticles)
  {
    for (const xAOD::TruthParticle * particle: *truthParticles)
      {
        if (std::abs(particle->pdgId()) > 19) continue;
        if (particle->pdgId() < 0) continue;
        if (particle->nParents() != 1) continue;
        const xAOD::TruthParticle * parent = particle->parent(0);
        if (!parent) continue;
        if (parent->pdgId() != 23) continue;
        const auto* sibling = getFlavourSibling(particle); // look for fermion with sibling
        if (!sibling) continue;

        tZ.Zdecay1_p4 = particle->p4();
        tZ.Zdecay1_pdgId = particle->pdgId();
        tZ.Zdecay1_status = particle->status();
        if (std::abs(tZ.Zdecay1_pdgId) == 15)
          {
	    bool isOk;
            tZ.Zdecay1_tau_isHadronic = PartonHistoryUtils::TauIsHadronic(particle,isOk);
          }

        tZ.Zdecay2_p4 = sibling->p4();
        tZ.Zdecay2_pdgId = sibling->pdgId();
        tZ.Zdecay2_status = sibling->status();
        if (std::abs(tZ.Zdecay2_pdgId) == 15) {
	  bool isOk;
          tZ.Zdecay2_tau_isHadronic = PartonHistoryUtils::TauIsHadronic(sibling,isOk);
        }

        tZ.Z_p4 = (tZ.Zdecay1_p4+tZ.Zdecay2_p4);
        return true;
      }

    return false;
  }


  void CalcTzqPartonHistory::FillZ(const xAOD::TruthParticle* particle)
  {
    tZ.Z_p4 = particle->p4();

    for (size_t k = 0; k < particle->nChildren(); k++) {
      const xAOD::TruthParticle* ZChildren = particle->child(k);
      if (ZChildren->pdgId() > 0) {
        tZ.Zdecay1_p4 = ZChildren->p4();
        tZ.Zdecay1_pdgId = ZChildren->pdgId();
        tZ.Zdecay1_status = ZChildren->status();

        // Tautau channel -> Check whether the Tau is hadronic or leptonic
        if (std::abs(tZ.Zdecay1_pdgId) == 15) {
	  bool isOk;
          tZ.Zdecay1_tau_isHadronic = PartonHistoryUtils::TauIsHadronic(ZChildren,isOk);
        }
      } else {
        tZ.Zdecay2_p4 = ZChildren->p4();
        tZ.Zdecay2_pdgId = ZChildren->pdgId();
        tZ.Zdecay2_status = ZChildren->status();

        // Tautau channel -> Check whether the Tau is hadronic or leptonic
        if (std::abs(tZ.Zdecay2_pdgId) == 15) {
	  bool isOk;
          tZ.Zdecay2_tau_isHadronic = PartonHistoryUtils::TauIsHadronic(ZChildren,isOk);
        }
      }
    }

    return;
  }

  bool CalcTzqPartonHistory::FindTZQVertex(const xAOD::TruthParticleContainer* truthParticles)
  {
    bool foundZ = false;
    bool foundQ = false;

    for (const xAOD::TruthParticle * particle : *truthParticles)
      {
        // Step 1: find top
        if (std::abs(particle->pdgId()) != 6) continue;

        // Step 2: go to parent
        const xAOD::TruthParticle * parent = particle->parent(0);
        if (!parent) continue;

        // Step 3: loop over children
        for (size_t i=0; i<parent->nChildren(); i++)
          {
            const xAOD::TruthParticle * child = parent->child(i);
            if (!child) continue;
            if (std::abs(child->pdgId()) == 6) continue;
            if (child->pdgId() == 23)
              {
                child = PartonHistoryUtils::findAfterFSR(child);
                if (child->nChildren() == 2)
                  {
                    foundZ = true;
                    FillZ(child);
                  }
                else
                  {
                    foundZ = false;
                  }
              }
            else if (std::abs(child->pdgId()) == 5)
              {
                tZ.b_p4 = child->p4();
                tZ.b_pdgId = child->pdgId();
              }
            else if (std::abs(child->pdgId()) < 5)
              {
                foundQ = true;
                tZ.q_p4 = child->p4();
                tZ.q_pdgId = child->pdgId();
              }
            else if (child->pdgId() == 25 )
              {
                // some events are tHq so reset and return
                tZ.reset();
                tZ.isThq = true;
                return true;
              }
            else
              {
                // looking for offshell z
                if (child->pdgId() < 0) continue;
                if (child->pdgId() > 19) continue;
                const auto* sibling = getFlavourSibling(child); // look for fermion with sibling
                if (!sibling) continue;
                foundZ = true;
                tZ.Zdecay1_p4 = child->p4();
                tZ.Zdecay1_pdgId = child->pdgId();
                tZ.Zdecay1_status = child->status();
                if (std::abs(tZ.Zdecay1_pdgId) == 15)
                  {
		    bool isOk;
                    tZ.Zdecay1_tau_isHadronic = PartonHistoryUtils::TauIsHadronic(child,isOk);
                  }

                tZ.Zdecay2_p4 = sibling->p4();
                tZ.Zdecay2_pdgId = sibling->pdgId();
                tZ.Zdecay2_status = sibling->status();
                if (std::abs(tZ.Zdecay2_pdgId) == 15)
                  {
		    bool isOk;
                    tZ.Zdecay2_tau_isHadronic = PartonHistoryUtils::TauIsHadronic(sibling,isOk);
                  }

                tZ.Z_p4 = (tZ.Zdecay1_p4+tZ.Zdecay2_p4);
              }
          }
        if (foundZ&&foundQ) return true;
      }

    if(!foundZ&&foundQ)
      {
        foundZ = FindLostZ(truthParticles);
      }

    return (foundZ&&foundQ);
  }

  bool CalcTzqPartonHistory::FindLostW(const xAOD::TruthParticleContainer *truthParticles,
                                       TLorentzVector &Wdecay1_p4, int &Wdecay1_pdgId,
                                       TLorentzVector &Wdecay2_p4, int &Wdecay2_pdgId)
  {
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;

    for (const xAOD::TruthParticle *particle : *truthParticles)
      {
        if (std::abs(particle->pdgId()) == 24 && particle->nParents() == 0)
          {
            particle = PartonHistoryUtils::findAfterFSR(particle);
            for (size_t l=0; l<particle->nChildren(); l++)
              {
                const xAOD::TruthParticle *p = particle->child(l);
                if (!p) continue;
                if (std::abs(p->pdgId()) < 17)
                  {
                    if (p->pdgId() > 0)
                      {
                        Wdecay1_p4 = p->p4();
                        Wdecay1_pdgId = p->pdgId();
                        hasWdecayProd1 = true;
                      }
                    else
                      {
                        Wdecay2_p4 = p->p4();
                        Wdecay2_pdgId = p->pdgId();
                        hasWdecayProd2 = true;
                      }
                  }
                if (hasWdecayProd1 && hasWdecayProd2) return true;
              }
          }
      }

    return false;
  }

  bool CalcTzqPartonHistory::FindLostTop(const xAOD::TruthParticleContainer *truthParticles,
                                         TLorentzVector &t_beforeFSR_p4, TLorentzVector &t_afterFSR_p4,
                                         TLorentzVector &W_p4, TLorentzVector &b_p4, TLorentzVector &Wdecay1_p4,
                                         int &Wdecay1_pdgId, TLorentzVector &Wdecay2_p4, int &Wdecay2_pdgId)
  {
    bool hasT = false;
    bool hasW = false;
    bool hasB = false;
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;

    for (const xAOD::TruthParticle *particle : *truthParticles)
      {
        if (std::abs(particle->pdgId()) != 6) continue;
        if (PartonHistoryUtils::hasParticleIdenticalParent(particle)) continue; // keeping only top before FSR

        t_beforeFSR_p4 = particle->p4();
        hasT = true;

        particle = PartonHistoryUtils::findAfterFSR(particle);
        t_afterFSR_p4 = particle->p4();

        for (size_t j=0; j<particle->nChildren(); j++)
          {
            const xAOD::TruthParticle *child = particle->child(j);
            if (!child) continue;
            if (std::abs(child->pdgId()) == 24)
              {
                W_p4 = child->p4();
                hasW = true;
                child = PartonHistoryUtils::findAfterFSR(child);
                for (size_t k=0; k<child->nChildren(); k++)
                  {
                    const xAOD::TruthParticle *Wchild = child->child(k);
                    if (!Wchild) continue;
                    if (std::abs(Wchild->pdgId()) < 17)
                      {
                        if (Wchild->pdgId() > 0)
                          {
                            Wdecay1_p4 = Wchild->p4();
                            Wdecay1_pdgId = Wchild->pdgId();
                            hasWdecayProd1 = true;
                          }
                        else
                          {
                            Wdecay2_p4 = Wchild->p4();
                            Wdecay2_pdgId = Wchild->pdgId();
                            hasWdecayProd2 = true;
                          }
                      }
                  }
              }
            else if (std::abs(child->pdgId()) == 5)
              {
                b_p4 = child->p4();
                hasB = true;
              }
          }
        if (hasT && hasW && hasB && hasWdecayProd1 && hasWdecayProd2) return true;
      }

    if (!hasWdecayProd1 || !hasWdecayProd2)
      {
        hasWdecayProd1 = hasWdecayProd2 = FindLostW(truthParticles, Wdecay1_p4, Wdecay1_pdgId, Wdecay2_p4, Wdecay2_pdgId);
      }

    return (hasT && hasW && hasB && hasWdecayProd1 && hasWdecayProd2);
  }

  void CalcTzqPartonHistory::TZHistorySaver(const xAOD::TruthParticleContainer* truthParticles,
                                            xAOD::PartonHistory* TzqPartonHistory) {
    TzqPartonHistory->IniVarTzq();
    TLorentzVector t_before, t_after, t_after_SC;
    TLorentzVector Wp;
    TLorentzVector b;
    TLorentzVector WpDecay1;
    TLorentzVector WpDecay2;
    int WpDecay1_pdgId{};
    int WpDecay2_pdgId{};

    bool event_top = CalcTopPartonHistory::topWb(truthParticles, 6, t_before, t_after, Wp, b, WpDecay1, WpDecay1_pdgId,
                                                 WpDecay2, WpDecay2_pdgId);
    bool event_top_SC = CalcTopPartonHistory::topAfterFSR_SC(truthParticles, 6, t_after_SC);
    bool event_topbar = CalcTopPartonHistory::topWb(truthParticles, -6, t_before, t_after, Wp, b, WpDecay1,
                                                    WpDecay1_pdgId, WpDecay2, WpDecay2_pdgId);
    bool event_topbar_SC = CalcTopPartonHistory::topAfterFSR_SC(truthParticles, -6, t_after_SC);

    if (!event_top && !event_topbar) {
      event_top = FindLostTop(truthParticles,
                              t_before, t_after,
                              Wp, b,
                              WpDecay1, WpDecay1_pdgId,
                              WpDecay2, WpDecay2_pdgId);
    }

    tZ.reset();
    bool event_ZandQ = FindTZQVertex(truthParticles);

    if (event_ZandQ) {
      if (event_top || event_topbar) {
        decorateWithMPtPhi(TzqPartonHistory, "MC_t_beforeFSR", t_before);
        fillEtaBranch(TzqPartonHistory, "MC_t_beforeFSR_eta", t_before);

        decorateWithMPtPhi(TzqPartonHistory, "MC_t_afterFSR", t_after);
        fillEtaBranch(TzqPartonHistory, "MC_t_afterFSR_eta", t_after);

        if (event_top_SC || event_topbar_SC) {
          decorateWithMPtPhi(TzqPartonHistory, "MC_t_afterFSR_SC", t_after_SC);
          fillEtaBranch(TzqPartonHistory, "MC_t_afterFSR_SC_eta", t_after_SC);
        }
        decorateWithMPtPhi(TzqPartonHistory, "MC_W_from_t", Wp);
        fillEtaBranch(TzqPartonHistory, "MC_W_from_t_eta", Wp);

        decorateWithMPtPhi(TzqPartonHistory, "MC_b_from_t", b);
        fillEtaBranch(TzqPartonHistory, "MC_b_from_t_eta", b);

        decorateWithMPtPhi(TzqPartonHistory, "MC_Wdecay1_from_t", WpDecay1);
        TzqPartonHistory->auxdecor< int >("MC_Wdecay1_from_t_pdgId") = WpDecay1_pdgId;
        fillEtaBranch(TzqPartonHistory, "MC_Wdecay1_from_t_eta", WpDecay1);

        decorateWithMPtPhi(TzqPartonHistory, "MC_Wdecay2_from_t", WpDecay2);
        TzqPartonHistory->auxdecor< int >("MC_Wdecay2_from_t_pdgId") = WpDecay2_pdgId;
        fillEtaBranch(TzqPartonHistory, "MC_Wdecay2_from_t_eta", WpDecay2);

        //Z-Variables
        decorateWithMPtPhi(TzqPartonHistory, "MC_Z", tZ.Z_p4);
        fillEtaBranch(TzqPartonHistory, "MC_Z_eta", tZ.Z_p4);

	//First Decay (Zdecay1)
        decorateWithMPtPhi(TzqPartonHistory, "MC_Zdecay1", tZ.Zdecay1_p4);
        TzqPartonHistory->auxdecor< int >("MC_Zdecay1_pdgId") = tZ.Zdecay1_pdgId;
        TzqPartonHistory->auxdecor< int >("MC_Zdecay1_status") = tZ.Zdecay1_status;
        fillEtaBranch(TzqPartonHistory, "MC_Zdecay1_eta", tZ.Zdecay1_p4);
        if (std::abs(tZ.Zdecay1_pdgId) == 15)
          {
            TzqPartonHistory->auxdecor<int>  ("MC_Zdecay1_tau_isHadronic") = tZ.Zdecay1_tau_isHadronic;
            decorateWithMPtPhi(TzqPartonHistory, "MC_Zdecay1_tauvis", tZ.Zdecay1_tauvis_p4);
            fillEtaBranch(TzqPartonHistory, "MC_Zdecay1_tauvis_eta", tZ.Zdecay1_tauvis_p4);
          }

        //Second Decay (Zdecay2)
        decorateWithMPtPhi(TzqPartonHistory, "MC_Zdecay2", tZ.Zdecay2_p4);
        TzqPartonHistory->auxdecor< int >("MC_Zdecay2_pdgId") = tZ.Zdecay2_pdgId;
        TzqPartonHistory->auxdecor< int >("MC_Zdecay2_status") = tZ.Zdecay2_status;
        fillEtaBranch(TzqPartonHistory, "MC_Zdecay2_eta", tZ.Zdecay2_p4);
        if (std::abs(tZ.Zdecay2_pdgId) == 15)
          {
            TzqPartonHistory->auxdecor<int>  ("MC_Zdecay2_tau_isHadronic") = tZ.Zdecay2_tau_isHadronic;
            decorateWithMPtPhi(TzqPartonHistory, "MC_Zdecay2_tauvis", tZ.Zdecay2_tauvis_p4);
            fillEtaBranch(TzqPartonHistory, "MC_Zdecay2_tauvis_eta", tZ.Zdecay2_tauvis_p4);
          }

        //b quark
        decorateWithMPtPhi(TzqPartonHistory, "MC_b", tZ.b_p4);
        TzqPartonHistory->auxdecor< int >("MC_b_pdgId") = tZ.b_pdgId;
        fillEtaBranch(TzqPartonHistory, "MC_b_eta", tZ.b_p4);

        //q quark
        decorateWithMPtPhi(TzqPartonHistory, "MC_q", tZ.q_p4);
        TzqPartonHistory->auxdecor< int >("MC_q_pdgId") = tZ.q_pdgId;
        fillEtaBranch(TzqPartonHistory, "MC_q_eta", tZ.q_p4);

        TzqPartonHistory->auxdecor< int >("isThqEvent") = tZ.isThq;
      }
    }
  }

  StatusCode CalcTzqPartonHistory::execute() {
    //Get the Truth Particles
    const xAOD::TruthParticleContainer* truthParticles(nullptr);

    if(m_config->getDerivationStream() == "PHYS") //in DAOD_PHYS we don't have the truth particles container
    {
      // To obtain both the top and the Z boson, we need the collections for both
      std::vector<std::string> collections = {"TruthTop", "TruthBosonsWithDecayParticles", "HardScatterParticles"};
      ATH_CHECK(buildContainerFromMultipleCollections(collections,"AT_TZqPartonHistory_TruthParticles"));
      ATH_CHECK(evtStore()->retrieve(truthParticles, "AT_TZqPartonHistory_TruthParticles"));
      
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
    xAOD::PartonHistory* TzqPartonHistory = new xAOD::PartonHistory {};
    partonCont->push_back(TzqPartonHistory);

    // Recover the parton history for TZ events
    TZHistorySaver(truthParticles, TzqPartonHistory);

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
