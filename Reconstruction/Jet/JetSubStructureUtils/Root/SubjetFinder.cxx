/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureUtils/SubjetFinder.h"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include <iostream>

using namespace std;
using namespace JetSubStructureUtils;

SubjetFinder::SubjetFinder(fastjet::JetAlgorithm fj_jetalg, float jet_radius, float pt_min) :
  m_fj_jetalg(fj_jetalg), m_jetrad(jet_radius), m_ptmin(pt_min)
{
}

vector<fastjet::PseudoJet> SubjetFinder::result(const fastjet::PseudoJet &jet) const
{
  vector<fastjet::PseudoJet> constit_pseudojets = jet.constituents();
  vector<fastjet::PseudoJet> subjets;
  if(constit_pseudojets.size() == 0) { 
    cout << "Warning in SubjetFinder: jet has no constituents" << endl;
    return subjets;
  }

  fastjet::JetDefinition jet_def = fastjet::JetDefinition(m_fj_jetalg, m_jetrad,
      fastjet::E_scheme, fastjet::Best);
  fastjet::ClusterSequence *clust_seq = new fastjet::ClusterSequence(constit_pseudojets, jet_def);
  subjets = fastjet::sorted_by_pt(clust_seq->inclusive_jets(m_ptmin));
  clust_seq->delete_self_when_unused();
  return subjets;
}
