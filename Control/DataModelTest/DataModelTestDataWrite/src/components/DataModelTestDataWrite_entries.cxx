/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataWrite/src/components/DataModelTestDataWrite_entries.cxx
 * @author snyder@bnl.gov
 * @date Nov 2005
 * @brief Gaudi algorithm factory declarations.
 */

#include "../DMTestWrite.h"
#include "../xAODTestWrite.h"
#include "../xAODTestWriteHVec.h"
#include "../xAODTestWriteCView.h"
#include "../xAODTestWriteCVecConst.h"
#include "../xAODTestWriteSymlinks.h"
#include "../HLTResultWriter.h"
#include "../xAODTestWriteCInfoTool.h"
#include "../AllocTestReadWithoutAlloc.h"
#include "../AllocTestWriteWithoutAlloc.h"

DECLARE_COMPONENT( DMTest::DMTestWrite )
DECLARE_COMPONENT( DMTest::xAODTestWrite )
DECLARE_COMPONENT( DMTest::xAODTestWriteHVec )
DECLARE_COMPONENT( DMTest::xAODTestWriteCView )
DECLARE_COMPONENT( DMTest::xAODTestWriteCVecConst )
DECLARE_COMPONENT( DMTest::xAODTestWriteSymlinks )
DECLARE_COMPONENT( DMTest::HLTResultWriter )

DECLARE_COMPONENT( DMTest::xAODTestWriteCInfoTool )
DECLARE_COMPONENT( DMTest::AllocTestReadWithoutAlloc )
DECLARE_COMPONENT( DMTest::AllocTestWriteWithoutAlloc )
