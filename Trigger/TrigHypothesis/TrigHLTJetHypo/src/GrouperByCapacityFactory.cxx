/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "GrouperByCapacityFactory.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/AllJetsGrouper.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/SingleJetGrouper.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/CombinationsGrouper.h"


std::unique_ptr<IJetGrouper> grouperByCapacityFactory(unsigned int cap,
						      const HypoJetCIter& b,
						      const HypoJetCIter& e){

  std::unique_ptr<IJetGrouper> pGrouper(nullptr);
  
  if (cap == 0) {
    throw std::runtime_error("groupByMultFactory - attempting ctrct grouper with mult == 0");
  } else if (cap == 1) {
    pGrouper.reset(new SingleJetGrouper(b, e));
  } else if (cap == std::numeric_limits<int>::max()) {
    pGrouper.reset(new AllJetsGrouper(b, e));
  } else {
    pGrouper.reset(new CombinationsGrouper(cap, b, e));
  }

  return pGrouper;
}
