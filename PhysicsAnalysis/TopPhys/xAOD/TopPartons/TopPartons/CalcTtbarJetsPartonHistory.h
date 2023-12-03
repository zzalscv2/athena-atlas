/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ANALYSISTOP_TOPPARTONS_CALCTTBARJETSPARTONHISTORY_H
#define ANALYSISTOP_TOPPARTONS_CALCTTBARJETSPARTONHISTORY_H

/**
 * @author John Morris <john.morris@cern.ch>
 * @author Silvestre Marino Romano <sromanos@cern.ch>
 * @author Samuel Calvet <scalvet@cern.ch>
 *
 * @brief CalcTtbarJetsPartonHistory
 *   Class derived from CalcTopPartonHistory, used to store ttbar variables
 *
 * $Revision: 658996 $
 * $Date: 2015-04-04 16:57:15 +0100 (Sat, 04 Apr 2015) $
 *
 **/

#include<tuple>

// Framework include(s):
#include "TopPartons/CalcTopPartonHistory.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TopPartons/PartonHistory.h"

// forward declaration(s):
namespace top {
  class TopConfig;
}

namespace top {
  class CalcTtbarJetsPartonHistory: public CalcTopPartonHistory {
  public:
    explicit CalcTtbarJetsPartonHistory(const std::string& name);
    virtual ~CalcTtbarJetsPartonHistory() {}

    //Storing parton history for ttbar resonance analysis
    CalcTtbarJetsPartonHistory(const CalcTtbarJetsPartonHistory& rhs) = delete;
    CalcTtbarJetsPartonHistory(CalcTtbarJetsPartonHistory&& rhs) = delete;
    CalcTtbarJetsPartonHistory& operator = (const CalcTtbarJetsPartonHistory& rhs) = delete;

    void ttbarjetsHistorySaver(const xAOD::TruthParticleContainer* truthParticles, xAOD::PartonHistory* ttbarjetsPartonHistory);
    void ttbarjetsTLorentzFill(xAOD::PartonHistory* ttbarjetsPartonHistory, TLorentzVector& p4, const std::string& name, FillBranchMethod FillMethod = FillBranchMethod::Regular);

    virtual StatusCode execute();

    typedef std::tuple<TLorentzVector,int,int,int> PartonInfo;
    typedef std::pair<PartonInfo,PartonInfo> PartonParentPair;
  };
}

#endif
