//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_GLOBALSIMDEFS_H
#define GLOBSIM_GLOBALSIMDEFS_H

/*
 * This file is a common source of 'using' statements required by
 * various GlobalSim translation units.
 *
 */

#include "GlobalData.h"
#include "L1TopoEvent/InputTOBArray.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoInterfaces/Count.h"
#include "L1TopoInterfaces/Decision.h"

namespace GlobalSim {
  using GSInputTOBArray = GlobalData<const TCS::InputTOBArray*>;
  using GSTOBArray = GlobalData<TCS::TOBArray>;
  using GSTOBArrayPtrVec = GlobalData<std::vector<TCS::TOBArray*>>;
  using GSCount = GlobalData<TCS::Count>;
  using GSDecision = GlobalData<TCS::Decision>;
}
#endif
