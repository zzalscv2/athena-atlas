/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TruthD3PDMaker/src/hepMCInheritance.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2009
 * @brief Declare inheritance relationships for HepMC classes.
 *
 * Eventually, these should be moved to the EDM classes.
 */

#include "GeneratorObjects/McEventCollection.h"
#include "Navigation/IAthenaBarCode.h"
#include "AtlasHepMC/GenEvent.h"
#include "AthenaKernel/BaseInfo.h"

SG_ADD_BASE (McEventCollection, DataVector<HepMC::GenEvent>);
