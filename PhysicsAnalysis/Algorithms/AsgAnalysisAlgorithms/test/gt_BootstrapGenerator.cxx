/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <AsgTesting/UnitTest.h>
#include <AsgMessaging/MessageCheck.h>
#include <AsgAnalysisAlgorithms/BootstrapGeneratorAlg.h>

//
// unit test
//

using namespace asg::msgUserCode;

namespace CP
{

  // Test fixture for BootstrapGenerator
  class BootstrapGeneratorTest : public ::testing::Test
  {
  protected:
    BootstrapGenerator m_bootstrapGenerator;
  };

  TEST_F (BootstrapGeneratorTest, SeedGeneration)
  {
    //    BootstrapGenerator bootstrapGenerator;

    // Test seed generation with specific input values
    std::uint64_t eventNumber = 123;
    std::uint32_t runNumber = 456;
    std::uint32_t mcChannelNumber = 789;

    // Generate seed
    std::uint64_t seed = m_bootstrapGenerator.generateSeed(eventNumber, runNumber, mcChannelNumber);

    // Define the expected result based on the known hash function
    std::uint64_t expectedSeed = 8089831591138695645u;

    // Check if the generated seed matches the expected result
    EXPECT_EQ(seed, expectedSeed);
  }

  TEST_F (BootstrapGeneratorTest, WeightGenerationMC)
  {
    //    BootstrapGenerator bootstrapGenerator;

    // Test weight generation with specific input values
    std::uint64_t eventNumber = 306030154;
    std::uint32_t runNumber = 310000;
    std::uint32_t mcChannelNumber = 410470;

    // Generate and set seed
    m_bootstrapGenerator.setSeed(eventNumber, runNumber, mcChannelNumber);

    // Collect the first ten weights
    std::vector<std::uint8_t> weights;
    for (int i = 0; i < 10; i++)
      {
	weights.push_back( m_bootstrapGenerator.getBootstrap() );
      }

    // Check if the generated weights match the expected result
    ASSERT_THAT(weights, ::testing::ElementsAre(2,0,0,1,4,2,0,1,1,2));
  }

  TEST_F (BootstrapGeneratorTest, WeightGenerationData)
  {
    //    BootstrapGenerator bootstrapGenerator;

    // Test weight generation with specific input values
    std::uint64_t eventNumber = 3772712513;
    std::uint32_t runNumber = 438481;
    std::uint32_t mcChannelNumber = 0;

    // Generate and set seed
    m_bootstrapGenerator.setSeed(eventNumber, runNumber, mcChannelNumber);

    // Collect the first ten weights
    std::vector<std::uint8_t> weights;
    for (int i = 0; i < 10; i++)
      {
	weights.push_back( m_bootstrapGenerator.getBootstrap() );
      }

    // Check if the generated weights match the expected result
    ASSERT_THAT(weights, ::testing::ElementsAre(1,3,2,1,2,0,0,1,1,1));
  }
} // namespace

int main (int argc, char **argv)
{
#ifdef ROOTCORE
  StatusCode::enableFailure();
  ANA_CHECK (xAOD::Init ());
#endif
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
