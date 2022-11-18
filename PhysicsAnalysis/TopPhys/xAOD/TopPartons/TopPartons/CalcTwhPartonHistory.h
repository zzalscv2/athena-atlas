/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ANALYSISTOP_TOPPARTONS_CALCTwhPARTONHISTORY_H
#define ANALYSISTOP_TOPPARTONS_CALCTwhPARTONHISTORY_H


// Framework include(s):
#include "TopPartons/CalcTopPartonHistory.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TopPartons/PartonHistory.h"

// forward declaration(s):
namespace top {
  class TopConfig;
}

namespace top {
  class CalcTwhPartonHistory: public CalcTopPartonHistory {
  public:
    explicit CalcTwhPartonHistory(const std::string& name);
    virtual ~CalcTwhPartonHistory() {}

    struct tWH_values {
      
      //Higgs

      TLorentzVector Higgs_p4;
      TLorentzVector decay1_p4;
      TLorentzVector decay2_p4;
      int decay1_pdgId;
      int decay2_pdgId;
      int tau_decay1_isHadronic;
      int tau_decay2_isHadronic;
      TLorentzVector tau_decay1_p4;
      TLorentzVector tau_decay2_p4;
      TLorentzVector tauvis_decay1_p4;
      TLorentzVector tauvis_decay2_p4;    

      
      TLorentzVector decay1_from_decay1_p4;
      TLorentzVector decay2_from_decay1_p4;
      int decay1_from_decay1_pdgId;
      int decay2_from_decay1_pdgId;
      TLorentzVector decay1_from_decay2_p4;
      TLorentzVector decay2_from_decay2_p4;
      int decay1_from_decay2_pdgId;
      int decay2_from_decay2_pdgId;
      
      int tau_decay1_from_decay1_isHadronic;
      int tau_decay2_from_decay1_isHadronic;
      int tau_decay1_from_decay2_isHadronic;
      int tau_decay2_from_decay2_isHadronic;
      
      TLorentzVector tau_decay1_from_decay1_p4;
      TLorentzVector tau_decay2_from_decay1_p4;
      TLorentzVector tau_decay1_from_decay2_p4;
      TLorentzVector tau_decay2_from_decay2_p4;

      TLorentzVector tauvis_decay1_from_decay1_p4;
      TLorentzVector tauvis_decay2_from_decay1_p4;
      TLorentzVector tauvis_decay1_from_decay2_p4;
      TLorentzVector tauvis_decay2_from_decay2_p4;


      
    } tWH;
    //Storing parton history for ttbar resonance analysis
    CalcTwhPartonHistory(const CalcTwhPartonHistory& rhs) = delete;
    CalcTwhPartonHistory(CalcTwhPartonHistory&& rhs) = delete;
    CalcTwhPartonHistory& operator = (const CalcTwhPartonHistory& rhs) = delete;

    void THHistorySaver(const xAOD::TruthParticleContainer* truthParticles, xAOD::PartonHistory* TwhPartonHistory);

    ///Store the four-momentum of several particles in the Higgs decay chain
    bool HiggsAndDecay(const xAOD::TruthParticleContainer* truthParticles);

    //Store spetator quark information
    bool spectatorquark(const xAOD::TruthParticleContainer* truthParticles, TLorentzVector& spectatorquark_beforeFSR,
			TLorentzVector& spectatorquark_afterFSR, int& spectatorquark_pdgId, int& spectatorquark_status);
    

    // Store b-quark from gluon spliting information
    bool secondb(const xAOD::TruthParticleContainer* truthParticles, int start,
		 TLorentzVector& secondb_beforeFSR_p4, int& secondb_beforeFSR_pdgId,
		 TLorentzVector& secondb_afterFSR_p4, int& secondb_afterFSR_pdgId);

    virtual StatusCode execute();
  };
}

#endif
