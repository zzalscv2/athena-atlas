/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/src/AllocTest_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#include "DataModelTestDataRead/versions/AllocTest_v1.h"
#include "xAODCore/AuxStoreAccessorMacros.h"


namespace DMTest {


AUXSTORE_PRIMITIVE_SETTER_AND_GETTER (AllocTest_v1, int,    atInt1,  setAtInt1)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER (AllocTest_v1, int,    atInt2,  setAtInt2)


} // namespace DMTest
