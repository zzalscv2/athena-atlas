/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSIMDATA_P3_H
#define MUONSIMDATA_P3_H

#include <vector>
#include "GeneratorObjectsTPCnv/HepMcParticleLink_p3.h"
#include "MuonMCData_p1.h"

namespace Muon {
    class MuonSimData_p3 {

      public:
	MuonSimData_p3() : m_word(0), x(0.), y(0.), z(0.), t(0.) {};
	friend class MuonSimDataCnv_p3;
	int m_word;
	std::vector < std::pair < HepMcParticleLink_p3, MuonMCData_p1 > > m_deposits;
	float x;
	float y;
	float z;
	float t;
    };
}
#endif
