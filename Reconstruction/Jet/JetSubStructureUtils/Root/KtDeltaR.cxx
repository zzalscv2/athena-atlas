/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureUtils/KtDeltaR.h"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "FourMomUtils/xAODP4Helpers.h"
#include <math.h>
#include <cmath>
using namespace std;
using namespace JetSubStructureUtils;

double KtDeltaR::result(const fastjet::PseudoJet &jet) const
{
  vector<fastjet::PseudoJet> constit_pseudojets = jet.constituents();
  if ( constit_pseudojets.size() < 2 ) {
    //Jet has fewer than 2 constituents.
    return 0.0;
  }

  fastjet::JetDefinition jetdef = fastjet::JetDefinition(fastjet::kt_algorithm, m_jetrad);
  fastjet::ClusterSequence cs(constit_pseudojets, jetdef);
  std::vector<fastjet::PseudoJet> outjets = cs.exclusive_jets(2);
  if ( outjets.size() < 2 ) {
    //"Fewer than two subjets found.
    return 0.0;
  }

  return xAOD::P4Helpers::deltaR(outjets[0].eta(), outjets[0].phi(),
				 outjets[1].eta(), outjets[1].phi());
}
