/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 */

// $Id: CalcTopPartonHistory.cxx 800464 2017-03-13 18:06:24Z tpelzer $
#include "TopPartons/CalcTopPartonHistory.h"
#include "TopPartons/PartonHistory.h"
#include "TopConfiguration/TopConfig.h"
#include "TopPartons/CalcTtbarPartonHistory.h"
#include "TopPartons/PartonHistoryUtils.h"
#include "xAODTruth/TruthVertex.h"

namespace top {
  CalcTopPartonHistory::CalcTopPartonHistory(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr) {
    declareProperty("config", m_config);
  }
  
  StatusCode CalcTopPartonHistory::buildContainerFromMultipleCollections(const std::vector<std::string> &collections, const std::string& out_contName)
  {
    ConstDataVector<DataVector<xAOD::TruthParticle_v1> > *out_cont = new ConstDataVector<DataVector<xAOD::TruthParticle_v1> > (SG::VIEW_ELEMENTS);
    
    for(const std::string& collection : collections)
    {
      const xAOD::TruthParticleContainer* cont=nullptr;
      ATH_CHECK(evtStore()->retrieve(cont,collection));
      for(const xAOD::TruthParticle* p : *cont) out_cont->push_back(p);
    }
    
    //we give control of the container to the store, because in this way we are able to retrieve it as a const data vector, see https://twiki.cern.ch/twiki/bin/view/AtlasComputing/DataVector#ConstDataVector
    xAOD::TReturnCode save = evtStore()->tds()->record(out_cont,out_contName);
    if (!save) return StatusCode::FAILURE;

    return StatusCode::SUCCESS;
  }
  
  StatusCode CalcTopPartonHistory::linkBosonCollections() 
  {
    return decorateCollectionWithLinksToAnotherCollection("TruthBoson","TruthBosonsWithDecayParticles","AT_linkToTruthBosonsWithDecayParticles");
  }
  StatusCode CalcTopPartonHistory::decorateCollectionWithLinksToAnotherCollection(const std::string &collectionToDecorate, const std::string &collectionToLink, const std::string &nameOfDecoration)
  {
    const xAOD::TruthParticleContainer* cont1(nullptr);
    const xAOD::TruthParticleContainer* cont2(nullptr);
    ATH_CHECK(evtStore()->retrieve(cont1,collectionToDecorate));
    ATH_CHECK(evtStore()->retrieve(cont2,collectionToLink));

    for(const xAOD::TruthParticle *p : *cont1)
    {
      
      const xAOD::TruthParticle* link =0;
      for(const xAOD::TruthParticle *p2 : *cont2)
      {
        if(p->pdgId()==p2->pdgId() && p->barcode()==p2->barcode())
        {
          link=p2;
          break;
        }
      } 
      p->auxdecor<const xAOD::TruthParticle*>(nameOfDecoration)=link;
      
    }
    return StatusCode::SUCCESS;
  }
  
  const xAOD::TruthParticle* CalcTopPartonHistory::getTruthParticleLinkedFromDecoration(const xAOD::TruthParticle* part, const std::string &decorationName)
  {
    if(!part->isAvailable<const xAOD::TruthParticle*>(decorationName)) return part;
  
    const xAOD::TruthParticle* link=part->auxdecor<const xAOD::TruthParticle*>(decorationName);
    if(link) return link;
    
    return part;
  }

  ///Store the four-momentum of the post-FSR top or anti-top found using statusCodes
  ///This would only work if there is at most one "true" top of each charge (i.e. won't work for SS tops or 4 tops)
  ///This code was adapted from the 7TeV parton-level differential ttbar routine:
  // https://svnweb.cern.ch/trac/atlasphys-top/browser/Physics/Top/Software/MCvalidation/Rivet/Rivet2.X/trunk/routines/ATLAS_2014_I1304289/ATLAS_2014_I1304289.cc
  bool CalcTopPartonHistory::topAfterFSR_SC(const xAOD::TruthParticleContainer* truthParticles, int start,
                                            TLorentzVector& top_afterFSR_SC_p4) {
    /// Step1: create vectors of particles of each status codes
    // Vectors to hold any status=3 (anti-)top quarks (Pythia 6)
    std::vector<const xAOD::TruthParticle*> v_status3_top;
    // Vectors to hold any status=155 (anti-)top quarks (Herwig 6)
    std::vector<const xAOD::TruthParticle*> v_status155_top;
    // Vectors to hold any status=11 (anti-)top quarks for Herwig++
    std::vector<const xAOD::TruthParticle*> v_status11_top;
    // Vectors to hold any status=22 (anti-)top quarks
    std::vector<const xAOD::TruthParticle*> v_statusOther_top;

    /// Step2: loop on the container of particles and fill the above created vectors
    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != start) continue; // only keep particles of a given pdgID (e.g. 6 or -6)

      if (particle->status() == 3) {
        v_status3_top.push_back(particle);
      } else if (particle->status() == 155) {
        v_status155_top.push_back(particle);
      } else if (particle->status() == 11) {// for Herwig++: take only the tops that decay into Wb!!!
        if (!particle->hasDecayVtx()) continue;
        const xAOD::TruthVertex* vertex = particle->decayVtx();
        if (vertex == nullptr) continue;
        if (vertex->nOutgoingParticles() == 2) v_status11_top.push_back(particle);
      } else {
        v_statusOther_top.push_back(particle);
      }
    }

    /// Step3: for some of the statuscodes, keep only the last of the vector
    // If there are more than 1 status 3 tops or anti-tops, only keep the last one put into the vector
    if (v_status3_top.size() > 1) {
      v_status3_top = std::vector<const xAOD::TruthParticle*>(v_status3_top.end() - 1, v_status3_top.end());
    }
    // If there are more than 1 status 11 tops or anti-tops, only keep the last one put into the vector
    if (v_status11_top.size() > 1) {
      v_status11_top = std::vector<const xAOD::TruthParticle*>(v_status11_top.end() - 1, v_status11_top.end());
    }
    // Rach: check for Pythia 8 as well
    // If there are more than 1 status 3 tops or anti-tops, only keep the last one put into the vector
    if (v_statusOther_top.size() > 1) {
      v_statusOther_top = std::vector<const xAOD::TruthParticle*>(v_statusOther_top.end() - 1, v_statusOther_top.end());
    }

    /// Step4: chose which statuscode to take according to what is found in the event
    const xAOD::TruthParticle* top = nullptr;
    // If there are status 3 tops and no status 155 tops this is probably a Pythia event, so used the status 3s.
    if (v_status3_top.size() == 1 && v_status155_top.size() == 0) {
      top = v_status3_top[0];
    }
    // If there are status 155 tops this must be a Herwig event, so use the status 155s.
    if (v_status155_top.size() == 1 && v_status3_top.size() == 0) {
      top = v_status155_top[0];
    }
    // If there are status 11 tops this must be a Herwig event, so use the status 11s.
    if (v_status11_top.size() == 1 && v_status3_top.size() == 0) {
      top = v_status11_top[0];
    }
    // If there are tops with other status this must be a Pythia8 event, so use them.
    if (v_statusOther_top.size() == 1 && v_status155_top.size() == 0 && v_status3_top.size() == 0  && v_status11_top.size() == 0) {
      top = v_statusOther_top[0];
    }

    /// Step5: if everything worked, set the 4-vector to its value and return true
    if (top != nullptr) {
      top_afterFSR_SC_p4 = top->p4();
      return true;
    }
    return false;
  }

  // for b coming from W'->tb
  bool CalcTopPartonHistory::b(const xAOD::TruthParticleContainer* truthParticles,
                               TLorentzVector& b_beforeFSR, TLorentzVector& b_afterFSR) {
    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (std::abs(particle->pdgId()) != 5) continue;

      bool skipit(false);
      for (size_t i = 0; i < particle->nParents(); i++) {
        const xAOD::TruthParticle* parent = particle->parent(i);
        if (parent && (parent->isTop() || std::abs(parent->pdgId()) == 5)) {
          skipit = true;
          break;
        }//if
      }//for

      if (skipit) continue;

      b_beforeFSR = particle->p4();
      b_afterFSR = PartonHistoryUtils::findAfterFSR(particle)->p4();

      return true;
    }


    return false;
  }

  // The function topWb has been overloaded to include aditional information when the W from t decays to a tau.
  bool CalcTopPartonHistory::topWb(const xAOD::TruthParticleContainer* truthParticles,
                                   int start, TLorentzVector& t_beforeFSR_p4, TLorentzVector& t_afterFSR_p4,
                                   TLorentzVector& W_p4,
                                   TLorentzVector& b_p4, TLorentzVector& Wdecay1_p4,
                                   int& Wdecay1_pdgId, TLorentzVector& Wdecay2_p4, int& Wdecay2_pdgId,
                                   TLorentzVector& tau_decay_from_W_p4, int& tau_decay_from_W_isHadronic,  TLorentzVector& tauvis_decay_from_W_p4) {
    bool hasT = false;
    bool hasW = false;
    bool hasB = false;
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;
    
    bool store_tau_info = true;
    if (tau_decay_from_W_isHadronic == -9999){store_tau_info = false;}

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != start) continue;

      if (PartonHistoryUtils::hasParticleIdenticalParent(particle)) continue; // kepping only top before FSR

      t_beforeFSR_p4 = particle->p4(); // top before FSR
      hasT = true;

      // demanding the last tops after FSR
      particle = PartonHistoryUtils::findAfterFSR(particle);
      t_afterFSR_p4 = particle->p4(); // top after FSR
      
      for (size_t k = 0; k < particle->nChildren(); k++) {
        const xAOD::TruthParticle* topChildren = particle->child(k);

        if (std::abs(topChildren->pdgId()) == 24) {
          W_p4 = topChildren->p4();  // W boson after FSR
          hasW = true;
          
          // demanding the last W after FSR
          topChildren = PartonHistoryUtils::findAfterFSR(topChildren);          
          for (size_t q = 0; q < topChildren->nChildren(); ++q) {
	    const xAOD::TruthParticle* WChildren = topChildren->child(q);
            if (std::abs(WChildren->pdgId()) < 17 && store_tau_info == true){
	      // When W decays leptonically, Wdecay1 stores the lepton and Wdecay2 the neutrino
	      if (std::abs(WChildren->pdgId()) == 11 || std::abs(WChildren->pdgId()) == 13 || std::abs(WChildren->pdgId()) == 15){
		Wdecay1_p4 = WChildren->p4();                                  
                Wdecay1_pdgId = WChildren->pdgId();
		const xAOD::TruthParticle* WChildrenAfterFSR = PartonHistoryUtils::findAfterFSR(WChildren);
		tau_decay_from_W_isHadronic = PartonHistoryUtils::TauIsHadronic(WChildren, tauvis_decay_from_W_p4);
		if (std::abs(Wdecay1_pdgId) == 15){
		  tau_decay_from_W_p4 = WChildrenAfterFSR->p4();
		}
                hasWdecayProd1 = true;
	      }
	      if (std::abs(WChildren->pdgId()) == 12 || std::abs(WChildren->pdgId()) == 14 || std::abs(WChildren->pdgId()) == 16){
		Wdecay2_p4 = WChildren->p4();
                Wdecay2_pdgId = WChildren->pdgId();
                hasWdecayProd2 = true;
	      }
	      if (std::abs(WChildren->pdgId()) < 11){ // W does not decay leptonically
		if (WChildren->pdgId() > 0) {
		  Wdecay1_p4 = WChildren->p4();
		  Wdecay1_pdgId = WChildren->pdgId();
		  tau_decay_from_W_isHadronic = -99;
		  tau_decay_from_W_p4 = WChildren->p4();
		  hasWdecayProd1 = true;
		}else{
		  Wdecay2_p4 = WChildren->p4();
		  Wdecay2_pdgId = WChildren->pdgId();
		  hasWdecayProd2 = true;
		}//else
	      }// if not leptonic decay
	    }// end of if store_tau_info ==  true
	    if (std::abs(WChildren->pdgId()) < 17 && store_tau_info == false){
	      if (WChildren->pdgId() > 0) {
                Wdecay1_p4 = WChildren->p4();
                Wdecay1_pdgId = WChildren->pdgId();
                hasWdecayProd1 = true;
              } else {
                Wdecay2_p4 = WChildren->p4();
                Wdecay2_pdgId = WChildren->pdgId();
                hasWdecayProd2 = true;
              }
	    } // end of if store_tau_info == false
          }//end of for
        } else if (abs(topChildren->pdgId()) == 5) {
          b_p4 = topChildren->p4();
          hasB = true;
        } //else if
      } //for (size_t k=0; k < particle->nChildren(); k++)

      if (hasT && hasW && hasB && hasWdecayProd1 && hasWdecayProd2) return true;
      
    } //for (const xAOD::TruthParticle* particle : *truthParticles)
    
    return false;
  }



  bool CalcTopPartonHistory::topWb(const xAOD::TruthParticleContainer* truthParticles,
                                   int start, TLorentzVector& t_beforeFSR_p4, TLorentzVector& t_afterFSR_p4,
                                   TLorentzVector& W_p4,
                                   TLorentzVector& b_p4, TLorentzVector& Wdecay1_p4,
                                   int& Wdecay1_pdgId, TLorentzVector& Wdecay2_p4, int& Wdecay2_pdgId) {
    TLorentzVector tau_decay_from_W_p4;
    int tau_decay_from_W_isHadronic = -9999;
    TLorentzVector tauvis_decay_from_W_p4;
    return topWb(truthParticles, start, t_beforeFSR_p4, t_afterFSR_p4, W_p4, b_p4, Wdecay1_p4,Wdecay1_pdgId, Wdecay2_p4, Wdecay2_pdgId, tau_decay_from_W_p4, tau_decay_from_W_isHadronic, tauvis_decay_from_W_p4);

    return false;
  }

 
  bool CalcTopPartonHistory::topWq(const xAOD::TruthParticleContainer* truthParticles,
                                   int start, TLorentzVector& t_beforeFSR_p4, TLorentzVector& t_afterFSR_p4,
                                   TLorentzVector& W_p4,
                                   TLorentzVector& q_p4, int& q_pdgId, TLorentzVector& Wdecay1_p4,
                                   int& Wdecay1_pdgId, TLorentzVector& Wdecay2_p4, int& Wdecay2_pdgId) {
    bool hasT = false;
    bool hasW = false;
    bool hasQ = false;
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != start) continue;

      if (PartonHistoryUtils::hasParticleIdenticalParent(particle)) continue; // kepping only top before FSR

      t_beforeFSR_p4 = particle->p4(); // top before FSR
      hasT = true;

      // demanding the last tops after FSR
      particle = PartonHistoryUtils::findAfterFSR(particle);
      t_afterFSR_p4 = particle->p4(); // top after FSR

      for (size_t k = 0; k < particle->nChildren(); k++) {
        const xAOD::TruthParticle* topChildren = particle->child(k);

        if (std::abs(topChildren->pdgId()) == 24) {
          W_p4 = topChildren->p4();  // W boson after FSR
          hasW = true;

          // demanding the last W after FSR
          topChildren = PartonHistoryUtils::findAfterFSR(topChildren);

          for (size_t q = 0; q < topChildren->nChildren(); ++q) {
            const xAOD::TruthParticle* WChildren = topChildren->child(q);
            if (std::abs(WChildren->pdgId()) < 17) {
              if (WChildren->pdgId() > 0) {
                Wdecay1_p4 = WChildren->p4();
                Wdecay1_pdgId = WChildren->pdgId();
                hasWdecayProd1 = true;
              } else {
                Wdecay2_p4 = WChildren->p4();
                Wdecay2_pdgId = WChildren->pdgId();
                hasWdecayProd2 = true;
              }//else
            }//if
          }//for
        } else if (std::abs(topChildren->pdgId()) == 5 || std::abs(topChildren->pdgId()) == 3 || std::abs(topChildren->pdgId()) == 1) {
          q_p4 = topChildren->p4();
          q_pdgId = topChildren->pdgId();
          hasQ = true;
        } //else if
      } //for (size_t k=0; k < particle->nChildren(); k++)
      if (hasT && hasW && hasQ && hasWdecayProd1 && hasWdecayProd2) return true;
    } //for (const xAOD::TruthParticle* particle : *truthParticles)

    return false;
  }

  bool CalcTopPartonHistory::Wlv(const xAOD::TruthParticleContainer* truthParticles,
                                 TLorentzVector& W_p4,
                                 TLorentzVector& Wdecay1_p4, int& Wdecay1_pdgId,
                                 TLorentzVector& Wdecay2_p4, int& Wdecay2_pdgId) {
    bool hasW = false;
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (std::abs(particle->pdgId()) != 24) continue;
      //std::cout << "PDGID: " << particle->pdgId() << std::endl;

      // demanding the last W after FSR
      particle = PartonHistoryUtils::findAfterFSR(particle);
      W_p4 = particle->p4();  // W boson after FSR
      hasW = true;

      for (size_t k = 0; k < particle->nChildren(); k++) {
        const xAOD::TruthParticle* WChildren = particle->child(k);
        if (std::abs(WChildren->pdgId()) < 17) {
          if (WChildren->pdgId() % 2 == 1) { // charged lepton in the Wlv case
            Wdecay1_p4 = WChildren->p4();
            Wdecay1_pdgId = WChildren->pdgId();
            hasWdecayProd1 = true;
          } else {// neutral lepton in the Wlv case
            Wdecay2_p4 = WChildren->p4();
            Wdecay2_pdgId = WChildren->pdgId();
            hasWdecayProd2 = true;
          }//else
        }//if
      } //for (size_t k=0; k < particle->nChildren(); k++)

      if (hasW && hasWdecayProd1 && hasWdecayProd2) return true;
    } //for (const xAOD::TruthParticle* particle : *truthParticles)


    return false;
  }

  // for Wt ST events, find W that is not from top
  bool CalcTopPartonHistory::Wt_W(const xAOD::TruthParticleContainer* truthParticles,
                                  TLorentzVector& W_p4, int& W_pdgId, TLorentzVector& Wdecay1_p4,
                                  int& Wdecay1_pdgId, TLorentzVector& Wdecay2_p4, int& Wdecay2_pdgId) {
    bool hasW = false;
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle == nullptr) continue;
      if (std::abs(particle->pdgId()) != 24) continue; // W boson

      // need to check if the W is from top
      // identify the first in chain and check
      // if that particle has top as parent
      if (PartonHistoryUtils::hasParticleIdenticalParent(particle)) continue; // kepping only W before FSR

      bool isFromTop = false;
      // now we should have only the first W in chain
      for (size_t iparent = 0; iparent < particle->nParents(); ++iparent) {
        if (particle->parent(iparent) == nullptr) continue;
        if (std::abs(particle->parent(iparent)->pdgId()) == 6) { // has top as parent
          isFromTop = true;
          break;
        }
      }

      if (isFromTop) continue;
      else {
        particle = PartonHistoryUtils::findAfterFSR(particle);
        W_p4 = particle->p4();
        W_pdgId = particle->pdgId();
        hasW = true;
      }

      // check the decay products of the W
      for (size_t q = 0; q < particle->nChildren(); ++q) {
        const xAOD::TruthParticle* WChildren = particle->child(q);
        if (WChildren == nullptr) continue;
        if (std::abs(WChildren->pdgId()) < 17) {
          if (WChildren->pdgId() > 0) {
            Wdecay1_p4 = WChildren->p4();
            Wdecay1_pdgId = WChildren->pdgId();
            hasWdecayProd1 = true;
          } else {
            Wdecay2_p4 = WChildren->p4();
            Wdecay2_pdgId = WChildren->pdgId();
            hasWdecayProd2 = true;
          }//else
        }//if
      }//for

      if (hasW && hasWdecayProd1 && hasWdecayProd2) return true;
    } // loop over truth particles

    return false;
  }

  // for Wt ST events, find b that is not from top
  bool CalcTopPartonHistory::Wt_b(const xAOD::TruthParticleContainer* truthParticles,
                                  TLorentzVector& b_beforeFSR, TLorentzVector& b_afterFSR,
                                  int& b_pdgId) {
    bool hasB = false;

    // identify "other" b quark that is not from radiation but from ME (Wtb)
    // logic is simple: search for b quark that doesn't have top, proton, or
    // nullptr as parent

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle == nullptr) continue;
      if (std::abs(particle->pdgId()) != 5) continue;

      for (size_t iparent = 0; iparent < particle->nParents(); ++iparent) {
        if (particle->parent(iparent) == nullptr) continue;

        // we dont want b-quarks that have b as parent
        if (std::abs(particle->parent(iparent)->pdgId()) == 5) continue;

        // we dont want b-quarks that come from top
        if (std::abs(particle->parent(iparent)->pdgId()) == 6) continue;

        // we dont want b-quarks that come from W
        if (std::abs(particle->parent(iparent)->pdgId()) == 24) continue;

        // we dont want b-quarks that come from proton
        if (std::abs(particle->parent(iparent)->pdgId()) == 2212) continue;

        hasB = true;
        b_beforeFSR = particle->p4();
        b_pdgId = particle->pdgId();

        // find after FSR
        particle = PartonHistoryUtils::findAfterFSR(particle);
        b_afterFSR = particle->p4();
      }
    }


    if (hasB) return true;

    return false;
  }

  // for ttbar + photon events
  bool CalcTopPartonHistory::topPhWb(const xAOD::TruthParticleContainer* truthParticles, int topId,
                                     TLorentzVector& t_beforeFSR_p4, TLorentzVector& t_afterFSR_p4,
                                     TLorentzVector& Ph_p4, TLorentzVector& W_p4, TLorentzVector& b_p4,
                                     TLorentzVector& Wdecay1_p4, int& Wdecay1_pdgId, TLorentzVector& Wdecay2_p4,
                                     int& Wdecay2_pdgId, bool& has_ph, int& BranchType, int& IniPartonType,
                                     bool& missingTop) {
    bool hasT = false;
    bool hasW = false;
    bool hasAbsentW = false;
    bool hasB = false;

    has_ph = false;
    bool ph_W = false;
    bool ph_Top = false;
    bool ph_ISR = false;
    bool ph_b = false;
    bool hasWdecayProd1 = false;
    bool hasWdecayProd2 = false;
    missingTop = false;

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != topId) continue;

      if (PartonHistoryUtils::hasParticleIdenticalParent(particle)) continue; // kepping only top before FSR
      BranchType = -1;// 10(50): leptonic(hadronic), 12(52):topRad, 14(54):Wrad, 15(55):ISR, 18(58):b
      IniPartonType = -1;

      // finding siblings
      for (size_t iparent = 0; iparent < particle->nParents(); iparent++) {
        if (std::abs(particle->parent(iparent)->pdgId()) == 21) {
          IniPartonType = 1;
        } // gg fusion
        else if (std::abs(particle->parent(iparent)->pdgId()) < 6) {
          IniPartonType = 2;
        } //qq annihilation

        for (size_t ichild = 0; ichild < particle->parent(iparent)->nChildren(); ichild++) {
          if (particle->parent(iparent)->child(ichild)->pdgId() == 22) {
            const xAOD::TruthParticle* photon = PartonHistoryUtils::findAfterFSR(particle->parent(iparent)->child(ichild));
            Ph_p4 = photon->p4();
            has_ph = true;
            ph_ISR = true;
          }
          if (!missingTop &&
              (std::abs(particle->parent(iparent)->child(ichild)->pdgId()) == 5 ||
               std::abs(particle->parent(iparent)->child(ichild)->pdgId()) == 24)) {
            missingTop = true;
          }
        }
      }

      t_beforeFSR_p4 = particle->p4(); // top before FSR
      hasT = true;
      // demanding the last tops after FSR
      particle = PartonHistoryUtils::findAfterFSR(particle);
      t_afterFSR_p4 = particle->p4(); // top after FSR

      for (size_t k = 0; k < particle->nChildren(); k++) {// top children
        const xAOD::TruthParticle* topChildren = particle->child(k);

        if (std::abs(topChildren->pdgId()) == 24) {
          W_p4 = topChildren->p4();  // W boson before FSR
          hasW = true;

          // demanding the last W after FSR
          topChildren = PartonHistoryUtils::findAfterFSR(topChildren);

          for (size_t q = 0; q < topChildren->nChildren(); q++) {// W children
            const xAOD::TruthParticle* WChildren = topChildren->child(q);
            if (std::abs(WChildren->pdgId()) > 0 && std::abs(WChildren->pdgId()) < 17) {
              if (std::abs(WChildren->pdgId()) < 7) {
                BranchType = 50;
              }// hadronic
              else if (std::abs(WChildren->pdgId()) > 10 && std::abs(WChildren->pdgId()) < 17) {
                BranchType = 10;
              }// leptonic
              if (WChildren->pdgId() > 0) {
                WChildren = PartonHistoryUtils::findAfterFSR(WChildren);
                Wdecay1_p4 = WChildren->p4();
                Wdecay1_pdgId = WChildren->pdgId();
                hasWdecayProd1 = true;
              } else {
                WChildren = PartonHistoryUtils::findAfterFSR(WChildren);
                Wdecay2_p4 = WChildren->p4();
                Wdecay2_pdgId = WChildren->pdgId();
                hasWdecayProd2 = true;
              }//else
            } else if (std::abs(WChildren->pdgId()) == 22) {// photon
              // JUST FOR EXTRA SAFETY (not necessary)
              // check if there exists a photon already
              // if it does, check the photon's Pt
              // if found harder then consider, else do nothing
              if (has_ph) {
                if (WChildren->p4().Pt() > Ph_p4.Pt()) {
                  ph_W = true;
                  ph_ISR = false;
                  ph_Top = false;
                  ph_b = false;
                  WChildren = PartonHistoryUtils::findAfterFSR(WChildren);
                  Ph_p4 = WChildren->p4();
                }
              } else {
                has_ph = true;
                ph_W = true;
                WChildren = PartonHistoryUtils::findAfterFSR(WChildren);
                Ph_p4 = WChildren->p4();
              }
            }
          }// W children
        } else if (std::abs(topChildren->pdgId()) == 5) { // b
          hasB = true;
          topChildren = PartonHistoryUtils::findAfterFSR(topChildren);// b After FSR
          b_p4 = topChildren->p4();
          // In MG5 generation of ttgamma it is not expected to have any b radiation 'recorded'
          for (size_t b = 0; b < topChildren->nChildren(); b++) {// b Children
            const xAOD::TruthParticle* bChildren = topChildren->child(b);
            if (bChildren && bChildren->pdgId() == 22) {
              // JUST FOR EXTRA SAFETY (not necessary)
              if (has_ph) {
                if (bChildren->p4().Pt() > Ph_p4.Pt()) {
                  ph_b = true;
                  ph_ISR = false;
                  ph_Top = false;
                  ph_W = false;
                  bChildren = PartonHistoryUtils::findAfterFSR(bChildren);
                  Ph_p4 = bChildren->p4();
                }
              } else {
                has_ph = true;
                ph_b = true;
                bChildren = PartonHistoryUtils::findAfterFSR(bChildren);
                Ph_p4 = bChildren->p4();
              }
            }
          }
        } else if (std::abs(topChildren->pdgId()) == 22) {
          // JUST FOR EXTRA SAFETY (not necessary)
          if (has_ph) {
            if (topChildren->p4().Pt() > Ph_p4.Pt()) {
              topChildren = PartonHistoryUtils::findAfterFSR(topChildren);
              Ph_p4 = topChildren->p4();
              ph_Top = true;
            }
          } else {
            topChildren = PartonHistoryUtils::findAfterFSR(topChildren);
            Ph_p4 = topChildren->p4();
            has_ph = true;
            ph_Top = true;
            ph_W = false;
            ph_ISR = false;
            ph_b = false;
          }
        }
        // sometimes the W is not recorded and the W products are recorded as top products
        else if (std::abs(topChildren->pdgId()) <= 4 || (std::abs(topChildren->pdgId()) > 10 && std::abs(topChildren->pdgId()) < 17)) {
          hasW = true;
          hasAbsentW = true;
          if (abs(topChildren->pdgId()) < 7) {
            BranchType = 50;
          }// hadronic
          else if (std::abs(topChildren->pdgId()) > 10 && std::abs(topChildren->pdgId()) < 17) {
            BranchType = 10;
          }// leptonic
          if (topChildren->pdgId() > 0) {
            topChildren = PartonHistoryUtils::findAfterFSR(topChildren);
            Wdecay1_p4 = topChildren->p4();
            Wdecay1_pdgId = topChildren->pdgId();
            hasWdecayProd1 = true;
          } else {
            topChildren = PartonHistoryUtils::findAfterFSR(topChildren);
            Wdecay2_p4 = topChildren->p4();
            Wdecay2_pdgId = topChildren->pdgId();
            hasWdecayProd2 = true;
          }//else
          W_p4 = W_p4 + topChildren->p4();
        }// if top children
      } // for top children

      // BranchType Determination if there is a photon
      if (hasAbsentW && (ph_Top || ph_W)) {
        BranchType = -1;
      }// if the W is not recorded and still the photon is from the top, the source of the photon is then ambiguous
       // among top and W. BranchType would be +1. Category would be 0.
      if (has_ph && ph_Top) {
        BranchType = BranchType + 2;
      }
      if (has_ph && ph_W) {
        BranchType = BranchType + 4;
      }
      if (has_ph && ph_ISR) {
        BranchType = BranchType + 5;
      }
      if (has_ph && ph_b) {
        BranchType = BranchType + 8;
      }

      if (hasT && hasW && hasB && hasWdecayProd1 && hasWdecayProd2 && BranchType != -1) return true;
    }// particle

    return false;
  }

  // for tttt events
  bool CalcTopPartonHistory::tttt(const xAOD::TruthParticleContainer* truthParticles, std::array<int,4> &top_pdgId, 
				  std::array<TLorentzVector,4> &top_beforeFSR_p4, std::array<TLorentzVector,4> &top_afterFSR_p4, 
				  std::array<TLorentzVector,4> &b_p4, std::array<TLorentzVector,4> &W_p4, 
				  std::array<int,4> &Wdecay1_pdgId, std::array<int,4> &Wdecay2_pdgId, 
				  std::array<TLorentzVector,4> &Wdecay1_p4, std::array<TLorentzVector,4> &Wdecay2_p4) {

    int n_top = 0;

    // Loop over the truth event record
    for (const auto* const particle : *truthParticles){
      if( std::abs(particle->pdgId()) != 6 ) continue;

      // For Sherpa 2.2.10 samples : 
      // So if you want to select parton-level event kinematics, you should always use status 20 if available in the event and otherwise status 3.
      if( std::abs(particle->status()) == 20 && n_top >=4 ) n_top=0; // Re-fill the top-quarks kinematic

      if(PartonHistoryUtils::hasParticleIdenticalParent(particle)) continue; // kepping only top before FSR
      top_pdgId[n_top] = particle->pdgId();
      top_beforeFSR_p4[n_top] = particle->p4();

      // demanding the last top quark after FSR
      const xAOD::TruthParticle* top_afterFSR = PartonHistoryUtils::findAfterFSR(particle);

      if(top_afterFSR == nullptr){
	ATH_MSG_WARNING("Top quark after FSR not found.");
	return false;
      }

      top_afterFSR_p4[n_top] = top_afterFSR->p4();

      // looping over top quark children
      for (size_t k = 0; k < top_afterFSR->nChildren(); k++) {
	const xAOD::TruthParticle* topChildren = top_afterFSR->child(k);

	if (std::abs(topChildren->pdgId()) == 24) { // W-boson
	  W_p4[n_top] = topChildren->p4();

	  // demanding the last W after FSR
	  const xAOD::TruthParticle* W_afterFSR = PartonHistoryUtils::findAfterFSR(topChildren);

	  // Extracting W decay particles if there are two children
	  if( W_afterFSR->nChildren() == 2 ){
	    const xAOD::TruthParticle* Wdecay1 = W_afterFSR->child(0);
	    Wdecay1_p4[n_top] = Wdecay1->p4();
	    Wdecay1_pdgId[n_top] = Wdecay1->pdgId();
	    const xAOD::TruthParticle* Wdecay2 = W_afterFSR->child(1);
	    Wdecay2_p4[n_top] = Wdecay2->p4();
	    Wdecay2_pdgId[n_top] = Wdecay2->pdgId();
	  }
	  else{
	    ATH_MSG_WARNING("W decays not found.");
	    return false;
	  }

	}
	else if (abs(topChildren->pdgId()) == 5) { // b-quark
	  b_p4[n_top] = topChildren->p4();
	}	
      } // top quark children loop

      n_top++;
    }

    // Check the number of top quarks
    if( n_top != 4 ){
      ATH_MSG_WARNING("The truth event record contains " << n_top << " top quarks.");
      return false;
    }

    return true;
  }


  StatusCode CalcTopPartonHistory::execute() {
    // Get the Truth Particles
    const xAOD::TruthParticleContainer* truthParticles(nullptr);

    ATH_CHECK(evtStore()->retrieve(truthParticles, m_config->sgKeyMCParticle()));

    // Create the partonHistory xAOD object
    xAOD::PartonHistoryAuxContainer* partonAuxCont = new xAOD::PartonHistoryAuxContainer {};
    xAOD::PartonHistoryContainer* partonCont = new xAOD::PartonHistoryContainer {};
    partonCont->setStore(partonAuxCont);

    xAOD::PartonHistory* partonHistory = new xAOD::PartonHistory {};
    partonCont->push_back(partonHistory);

    // Save to StoreGate / TStore
    std::string outputSGKey = m_config->sgKeyTopPartonHistory();
    std::string outputSGKeyAux = outputSGKey + "Aux.";

    xAOD::TReturnCode save = evtStore()->tds()->record(partonCont, outputSGKey);
    xAOD::TReturnCode saveAux = evtStore()->tds()->record(partonAuxCont, outputSGKeyAux);
    if (!save || !saveAux) {
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  void CalcTopPartonHistory::fillEtaBranch(xAOD::PartonHistory* partonHistory, std::string branchName,
                                           TLorentzVector& tlv) {
    if (tlv.CosTheta() == 1.) partonHistory->auxdecor< float >(branchName) = 1000.;
    else if (tlv.CosTheta() == -1.) partonHistory->auxdecor< float >(branchName) = 1000.;
    else partonHistory->auxdecor< float >(branchName) = tlv.Eta();
    return;
  }
}
