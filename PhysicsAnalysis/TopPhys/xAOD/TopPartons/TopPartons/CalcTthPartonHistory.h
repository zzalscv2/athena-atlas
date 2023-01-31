/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ANALYSISTOP_TOPPARTONS_CALCTTHPARTONHISTORY_H
#define ANALYSISTOP_TOPPARTONS_CALCTTHPARTONHISTORY_H

// Framework include(s):
#include "TopPartons/CalcTopPartonHistory.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TopPartons/PartonHistory.h"

// forward declaration(s):
namespace top {
  class TopConfig;
}

namespace top {
  class CalcTthPartonHistory: public CalcTopPartonHistory {
  public:
    explicit CalcTthPartonHistory(const std::string& name);
    virtual ~CalcTthPartonHistory() = default;

    struct ttH_values {
      //Higgs
      TLorentzVector Higgs_p4;
      TLorentzVector decay1_p4;
      TLorentzVector decay2_p4;
      int decay1_pdgId;
      int decay2_pdgId;
      int tau_decay1_isHadronic;
      int tau_decay2_isHadronic;
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
    } ttH;

    //Storing parton history for ttbar resonance analysis
    CalcTthPartonHistory(const CalcTthPartonHistory& rhs) = delete;
    CalcTthPartonHistory(CalcTthPartonHistory&& rhs) = delete;
    CalcTthPartonHistory& operator = (const CalcTthPartonHistory& rhs) = delete;

    void tthHistorySaver(const xAOD::TruthParticleContainer* truthParticles, xAOD::PartonHistory* tthPartonHistory);

    ///Store the four-momentum of several particles in the Higgs decay chain
    bool HiggsAndDecay(const xAOD::TruthParticleContainer* truthParticles);

    virtual StatusCode execute();
  };
}

#endif
