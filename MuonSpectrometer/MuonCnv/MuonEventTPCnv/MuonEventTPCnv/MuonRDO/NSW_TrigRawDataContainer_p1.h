/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef NSW_TRIGRAWDATACONTAINER_P1_H
#define NSW_TRIGRAWDATACONTAINER_P1_H

#include <vector>
#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawData_p1.h"

namespace Muon {
  class NSW_TrigRawDataContainer_p1 : public std::vector<NSW_TrigRawData_p1> {
    public:
      NSW_TrigRawDataContainer_p1() = default;
  };
}

#endif
