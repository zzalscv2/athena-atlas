/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

// Dear emacs, this is -*- c++ -*-
#ifndef XAODTRIGL1CALO_VERSIONS_JGTOWERCONTAINER_V1_H
#define XAODTRIGL1CALO_VERSIONS_JGTOWERCONTAINER_V1_H

// EDM include(s):
#include "AthContainers/DataVector.h"

// Local include(s):
#include "JGTower_v1.h"

namespace xAOD
{
  /// Define the JGTower as a simple DataVector
  typedef DataVector<xAOD::JGTower_v1> JGTowerContainer_v1;
} // namespace xAOD

#endif // XAODTRIGL1CALO_VERSIONS_JGTOWERCONTAINER_V1_H
