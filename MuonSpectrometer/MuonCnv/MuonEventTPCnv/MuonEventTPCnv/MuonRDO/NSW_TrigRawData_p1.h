/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef NSW_TRIGRAWDATA_P1_H
#define NSW_TRIGRAWDATA_P1_H

#include <vector>
#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataSegment_p1.h"

namespace Muon {
  class NSW_TrigRawData_p1 : public std::vector<NSW_TrigRawDataSegment_p1> {
    public:
      NSW_TrigRawData_p1() = default;

      uint16_t m_sectorId{0};
      char m_sectorSide{45};
      uint16_t m_bcId{0};
  };
}

#endif
