/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRUTHPARTICLELINK_H
#define XAODTRUTHPARTICLELINK_H

#include "GeneratorObjects/HepMcParticleLink.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "AthLinks/ElementLink.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <algorithm>

typedef std::pair<HepMcParticleLink,ElementLink<xAOD::TruthParticleContainer> > xAODTruthParticleLink;

struct SortTruthParticleLink {
  bool operator() (const xAODTruthParticleLink& l1, const xAODTruthParticleLink& l2 ) {
    return l1.first < l2.first;
  }
  bool operator() (const xAODTruthParticleLink* l1, const xAODTruthParticleLink* l2 ) {
    return operator()(*l1,*l2);
  }
};

class xAODTruthParticleLinkVector : public DataVector<xAODTruthParticleLink> {
public:
  ElementLink<xAOD::TruthParticleContainer> find( const HepMcParticleLink& hepMCLink ) const {
    auto result = std::lower_bound(begin(),end(), hepMCLink, [](const xAODTruthParticleLink* l, const HepMcParticleLink& hepLink ) { return l->first < hepLink;});
    if( result != end() ) {
         if( (*result)->first == hepMCLink ) return (*result)->second;
    }
    return ElementLink<xAOD::TruthParticleContainer>{}; 
  }
};

CLASS_DEF(xAODTruthParticleLinkVector, 143434583, 1)
#endif
