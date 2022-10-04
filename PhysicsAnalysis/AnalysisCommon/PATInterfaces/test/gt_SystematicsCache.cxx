/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <PATInterfaces/SystematicsCache.h>

#include <AsgMessaging/AsgMessaging.h>
#include <AsgMessaging/MessageCheck.h>
#include <RootCoreUtils/Assert.h>
#include <cmath>
#include <gtest/gtest.h>

using namespace CP;

//
// method implementations
//

#define ASSERT_SUCCESS(x)			\
  ASSERT_EQ (asg::CheckHelper<decltype(x)>::successCode(), x)

#define ASSERT_FAILURE(x)			\
  ASSERT_EQ (asg::CheckHelper<decltype(x)>::failureCode(), x)

TEST (SystematicCacheTest, simple)
{
  asg::AsgMessaging parent ("test");
  CP::SystematicsCache<std::string> cache (&parent);
  unsigned calls = 0u;
  CP::SystematicSet affecting;
  affecting.insert (CP::SystematicVariation ("test1__continuous"));
  cache.initialize (affecting, [&] (const CP::SystematicSet& sys, std::string& result) noexcept -> StatusCode
  {
    result = "sys_" + sys.name();
    calls += 1;
    return StatusCode::SUCCESS;
  });
  const std::string *result = nullptr;
  ASSERT_SUCCESS (cache.get (CP::SystematicSet (), result));
  EXPECT_EQ (*result, "sys_");
  EXPECT_EQ (calls, 1u);
  ASSERT_SUCCESS (cache.get (CP::SystematicSet ("test1__1up"), result));
  EXPECT_EQ (*result, "sys_test1__1up");
  EXPECT_EQ (calls, 2u);
  ASSERT_SUCCESS (cache.get (CP::SystematicSet ("miss__1down"), result));
  EXPECT_EQ (*result, "sys_");
  EXPECT_EQ (calls, 2u);
}

TEST (SystematicCacheTest, add)
{
  using namespace asg::msgUserCode;
  asg::AsgMessaging parent ("test");
  CP::SystematicsCache<std::string> cache (&parent);
  CP::SystematicSet affecting;
  cache.initialize (affecting, [&] (const CP::SystematicSet& /*sys*/, std::string& /*result*/) noexcept -> StatusCode
  {
    ANA_MSG_ERROR ("requesting unknown systematic");
    return StatusCode::FAILURE;
  });
  ASSERT_SUCCESS (cache.add (CP::SystematicSet (), "sys_"));
  ASSERT_SUCCESS (cache.add (CP::SystematicSet ("test1__1up"), "sys_test1__1up"));
  const std::string *result = nullptr;
  ASSERT_SUCCESS (cache.get (CP::SystematicSet (), result));
  EXPECT_EQ (*result, "sys_");
  ASSERT_SUCCESS (cache.get (CP::SystematicSet ("test1__1up"), result));
  EXPECT_EQ (*result, "sys_test1__1up");
  ASSERT_SUCCESS (cache.get (CP::SystematicSet ("miss__1down"), result));
  EXPECT_EQ (*result, "sys_");
}

int main (int argc, char **argv)
{
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
