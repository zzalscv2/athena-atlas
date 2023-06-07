/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENINTERFACES_GENINTERFACESDICT_H
#define GENINTERFACES_GENINTERFACESDICT_H

#if defined(__GCCXML__) and not defined(EIGEN_DONT_VECTORIZE)
#define EIGEN_DONT_VECTORIZE
#endif // __GCCXML__

// Includes for the dictionary generation:
#include "GenInterfaces/IEvgenOTFTopUpSvc.h"
#include "GenInterfaces/IHepMCWeightSvc.h"
#include "AtlasHepMC/SimpleVector.h"
#include <vector>

namespace HforToolDict {
  struct tmp {
    std::vector<HepMC::FourVector> v;
  };
}

#endif // GENINTERFACES_GENINTERFACESDICT_H
