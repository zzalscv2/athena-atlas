/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCSIMHITCOLLECTION_P2_H
#define CSCSIMHITCOLLECTION_P2_H

#include <vector>
#include "GeneratorObjectsTPCnv/HepMcParticleLink_p1.h"

namespace Muon {
class CSCSimHitCollection_p2   
{
 public:
  std::vector<int>      m_CSCid;
  std::vector<float>    m_globalTime;
  std::vector<float>    m_energyDeposit;
  std::vector<float>    m_stX, m_stY, m_stZ; //!< Represents m_hitStart in transient class
  std::vector<float>    m_enX, m_enY, m_enZ; //!< Represents m_hitEnd in transient class
  std::vector<int>      m_particleID; 
  std::vector<float>    m_meanTime;
  std::vector<HepMcParticleLink_p1>  m_partLink;
  std::vector<float>    m_kineticEnergy;
};
}

#endif

