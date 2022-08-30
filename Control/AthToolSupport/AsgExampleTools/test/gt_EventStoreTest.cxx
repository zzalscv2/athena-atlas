//
// Copyright (C) 2002-20222 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "AsgExampleTools/EventStoreTestTool.h"

// Framework include(s).
#include "AsgTesting/UnitTest.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

// GoogleTest include(s).
#include <gtest/gtest.h>

GTEST_TEST( EventStoreTests, EventStoreTest ) {

   // Set up the framework components.
   xAOD::TStore store;
   xAOD::TEvent event;

   // Instantiate the tool, and run its test.
   asg::EventStoreTestTool tool( "EventStoreTestTool" );
   EXPECT_SUCCESS( tool.performTest() );
}

ATLAS_GOOGLE_TEST_MAIN
