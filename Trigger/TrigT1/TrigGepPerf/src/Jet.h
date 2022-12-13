/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_JET_H
#define TRIGL0GEPPERF_JET_H

#include "TLorentzVector.h"

namespace Gep{
  struct Jet
  {
   
    TLorentzVector vec;    
    std::vector<int> constituentsIndices;
    int nConstituents;
    float radius {0};
    float seedEta {0};
    float seedPhi {0};
    float seedEt {0};
    
  };
}

#endif //TRIGL0GEPPERF_CUSTOMJET_H
