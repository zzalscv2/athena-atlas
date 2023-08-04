/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgMessaging/MessageCheck.h>
#include <EventLoop/LocalDriver.h>
#include <EventLoopTest/UnitTestConfig.h>
#include <EventLoopTest/UnitTestFixture.h>
#include <gtest/gtest.h>

#ifdef ROOTCORE
#include <xAODRootAccess/Init.h>
#endif

//
// method implementations
//

using namespace asg::msgUserCode;

struct MyUnitTestConfig : public EL::UnitTestConfig
{
  MyUnitTestConfig ()
  {
    static const std::shared_ptr<EL::LocalDriver>
      driver (new EL::LocalDriver);
    m_driver = driver;
  }
};

namespace EL
{
  // this has to be SLOW, since it requires the entire release to have
  // been build for it to work
  INSTANTIATE_TEST_SUITE_P(SLOW_LocalDriverTest, UnitTestFixture,
                           ::testing::Values(MyUnitTestConfig()));
}

int main (int argc, char **argv)
{
#ifdef ROOTCORE
  StatusCode::enableFailure();
  ANA_CHECK (xAOD::Init ());
#endif
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
