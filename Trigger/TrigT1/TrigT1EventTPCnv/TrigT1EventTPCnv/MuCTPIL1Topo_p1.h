/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1EVENTTPCNV_MuCTPIL1Topo_P1_H
#define TRIGT1EVENTTPCNV_MuCTPIL1Topo_P1_H

// System include(s):
#include <vector>
#include <inttypes.h>

#include "TrigT1EventTPCnv/MuCTPIL1TopoCandidate_p1.h"

/**
 *   @short Persistent representation of MuCTPIL1Topo
 *
 *          This is the first version of the persistent representation(s)
 *          of MuCTPIL1Topo. It stores the same data as the transient one,
 *          without having the easy accessor functions.
 *
 *   @author Anil Sonay
 */

struct MuCTPIL1Topo_p1 {
  int m_bcidOffset {};
  std::vector<MuCTPIL1TopoCandidate_p1> m_muonTopoCandidates {};
};


#endif // TRIGT1EVENTTPCNV_MuCTPIL1Topo_P1_H
