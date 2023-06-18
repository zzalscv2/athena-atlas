/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <iostream>
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"

std::ostream& operator<<(std::ostream& o, const FPGATrackSimCluster& cluster)
{
  return o << "Cluster formed from " << cluster.getHitList().size() << " hits";
}
