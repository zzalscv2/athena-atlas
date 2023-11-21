/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/src/components/DataModelTestDataRead_entries.cxx
 * @author snyder@bnl.gov
 * @date Nov 2005
 * @brief Gaudi algorithm factory declarations.
 */

#include "../DMTestRead.h"
#include "../xAODTestRead.h"
#include "../xAODTestReadCInfo.h"
#include "../xAODTestReadCView.h"
#include "../xAODTestReadHVec.h"
#include "../xAODTestFilterCVec.h"
#include "../xAODTestClearDecor.h"
#include "../xAODTestTypelessRead.h"
#include "../xAODTestShallowCopyHVec.h"
#include "../HLTResultReader.h"
#include "../AllocTestReadWithAlloc.h"
#include "../AllocTestWriteWithAlloc.h"

DECLARE_COMPONENT( DMTest::DMTestRead )
DECLARE_COMPONENT( DMTest::xAODTestRead )
DECLARE_COMPONENT( DMTest::xAODTestReadCInfo )
DECLARE_COMPONENT( DMTest::xAODTestReadCView )
DECLARE_COMPONENT( DMTest::xAODTestReadHVec )
DECLARE_COMPONENT( DMTest::xAODTestFilterCVec )
DECLARE_COMPONENT( DMTest::xAODTestClearDecor )
DECLARE_COMPONENT( DMTest::xAODTestTypelessRead )
DECLARE_COMPONENT( DMTest::xAODTestShallowCopyHVec )
DECLARE_COMPONENT( DMTest::HLTResultReader )
DECLARE_COMPONENT( DMTest::AllocTestReadWithAlloc )
DECLARE_COMPONENT( DMTest::AllocTestWriteWithAlloc )
