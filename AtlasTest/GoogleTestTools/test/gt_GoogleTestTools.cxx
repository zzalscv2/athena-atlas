/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "GoogleTestTools/InitGaudiGoogleTest.h"


namespace Athena_test {

  // Fixture class derived from InitGaudiGoogleTest
  // General pattern for fixture objects setting up Gaudi
  class InitGaudiGoogleTestTest : public InitGaudiGoogleTest {
  public:

    InitGaudiGoogleTestTest() 
    // Get usual Gaudi message blurb, comment out to silence Gaudi
    : InitGaudiGoogleTest( MSG::INFO ) 
    {}

    // Real Gaudi configuration should happen here
    //virtual void SetUp() override {}

  };
  
  // Test shows how to access Gaudis pointers
  // cppcheck-suppress syntaxError
  TEST_F( InitGaudiGoogleTestTest, basictest ) {
    EXPECT_TRUE( theApp != nullptr );
    EXPECT_TRUE( propMgr.isValid() );
    EXPECT_TRUE( svcLoc.isValid() );
    EXPECT_TRUE( svcMgr.isValid() );
  }

}


int main( int argc, char **argv ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

