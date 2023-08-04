/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgMessaging/MessageCheck.h>
#include <AsgTesting/UnitTest.h>
#include <cmath>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#ifdef XAOD_STANDALONE
#include <xAODRootAccess/Init.h>
#endif

//
// method implementations
//

using namespace asg::msgUserCode;

namespace asg
{
  StatusCode functionSuccess ()
  {
    return StatusCode::SUCCESS;
  }

  StatusCode functionFailure ()
  {
    return StatusCode::FAILURE;
  }

  StatusCode functionFailure (const std::string& message)
  {
    ANA_MSG_ERROR (message);
    return StatusCode::FAILURE;
  }

  StatusCode functionThrow ()
  {
    throw std::runtime_error ("runtime error");
  }


  // cppcheck-suppress syntaxError
  TEST (AssertTest, success_success)
  {
    ASSERT_SUCCESS (functionSuccess());
  }

  TEST (AssertTest, success_failure)
  {
    EXPECT_FATAL_FAILURE (ASSERT_SUCCESS (functionFailure()), "functionFailure()");
  }

  TEST (AssertTest, success_throw)
  {
    EXPECT_ANY_THROW (ASSERT_SUCCESS (functionThrow()));
  }


  TEST (AssertTest, failure_success)
  {
    EXPECT_FATAL_FAILURE (ASSERT_FAILURE (functionSuccess()), "functionSuccess()");
  }

  TEST (AssertTest, failure_failure)
  {
    ASSERT_FAILURE (functionFailure());
  }

  TEST (AssertTest, failure_throw)
  {
    EXPECT_ANY_THROW (ASSERT_FAILURE (functionThrow()));
  }



  TEST (AssertTest, failure_regex_success)
  {
    EXPECT_FATAL_FAILURE (ASSERT_FAILURE_REGEX (functionSuccess(), ".*match.*"), "functionSuccess()");
  }

  TEST (AssertTest, failure_regex_failure_match)
  {
    EXPECT_FAILURE_REGEX (functionFailure("match"),".*match.*");
  }

#ifdef XAOD_STANDALONE
  TEST (AssertTest, failure_regex_failure_missmatch)
#else
  TEST (AssertTest, DISABLED_failure_regex_failure_missmatch)
#endif
  {
    EXPECT_FATAL_FAILURE (ASSERT_FAILURE_REGEX (functionFailure("text 1"),".*different text.*"), "functionFailure");
  }

#ifdef XAOD_STANDALONE
  TEST (AssertTest, failure_regex_failure_missing)
#else
  TEST (AssertTest, DISABLED_failure_regex_failure_missing)
#endif
  {
    EXPECT_FATAL_FAILURE (ASSERT_FAILURE_REGEX (functionFailure(),".*match.*"), "functionFailure()");
  }

  TEST (AssertTest, failure_REGEX_throw)
  {
    EXPECT_ANY_THROW (ASSERT_FAILURE_REGEX (functionThrow(),".*match.*"));
  }


  TEST (AssertTest, throw_regex_match)
  {
    ASSERT_THROW_REGEX (throw std::runtime_error ("a matched runtime exception"), "a matched runtime exception");
  }

  TEST (AssertTest, throw_regex_match_preamble)
  {
    ASSERT_THROW_REGEX (throw std::runtime_error ("pre-amble, a matched runtime exception"), "a matched runtime exception");
  }

  TEST (AssertTest, throw_regex_match_postamble)
  {
    ASSERT_THROW_REGEX (throw std::runtime_error ("a matched runtime exception, post-amble"), "a matched runtime exception");
  }

  TEST (AssertTest, throw_regex_missmatch)
  {
    EXPECT_FATAL_FAILURE (ASSERT_THROW_REGEX (throw std::runtime_error ("a miss-matched runtime exception"), "a matched runtime exception"), "a matched runtime exception");
    EXPECT_FATAL_FAILURE (ASSERT_THROW_REGEX (throw std::runtime_error ("a miss-matched runtime exception"), "a matched runtime exception"), "a miss-matched runtime exception");
  }

  TEST (AssertTest, throw_regex_nonstd)
  {
    EXPECT_FATAL_FAILURE (ASSERT_THROW_REGEX (throw std::string ("a non std::exception"), "a matched runtime exception"), "threw an exception that didn't derive from std::exception");
  }

  TEST (AssertTest, match_regex_match)
  {
    ASSERT_MATCH_REGEX ("[a-z]x[a-z]", "aaxzz");
  }

  TEST (AssertTest, match_regex_missmatch)
  {
    EXPECT_FATAL_FAILURE (ASSERT_MATCH_REGEX ("[a-z]x[a-z]", "a1xzz"), "[a-z]x[a-z]");
  }



  TEST (ExpectTest, success_success)
  {
    EXPECT_SUCCESS (functionSuccess());
  }

  TEST (ExpectTest, success_failure)
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_SUCCESS (functionFailure()), "functionFailure()");
  }

  TEST (ExpectTest, success_throw)
  {
    EXPECT_ANY_THROW (EXPECT_SUCCESS (functionThrow()));
  }


  TEST (ExpectTest, failure_success)
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_FAILURE (functionSuccess()), "functionSuccess()");
  }

  TEST (ExpectTest, failure_failure)
  {
    EXPECT_FAILURE (functionFailure());
  }

  TEST (ExpectTest, failure_throw)
  {
    EXPECT_ANY_THROW (EXPECT_FAILURE (functionThrow()));
  }



  TEST (ExpectTest, failure_regex_success)
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_FAILURE_REGEX (functionSuccess(), ".*match.*"), "functionSuccess()");
  }

  TEST (ExpectTest, failure_regex_failure_match)
  {
    EXPECT_FAILURE_REGEX (functionFailure("match"),".*match.*");
  }

#ifdef XAOD_STANDALONE
  TEST (ExpectTest, failure_regex_failure_missmatch)
#else
  TEST (ExpectTest, DISABLED_failure_regex_failure_missmatch)
#endif
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_FAILURE_REGEX (functionFailure("text 1"),".*different text.*"), "functionFailure");
  }

#ifdef XAOD_STANDALONE
  TEST (ExpectTest, failure_regex_failure_missing)
#else
  TEST (ExpectTest, DISABLED_failure_regex_failure_missing)
#endif
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_FAILURE_REGEX (functionFailure(),".*match.*"), "functionFailure()");
  }

  TEST (ExpectTest, failure_REGEX_throw)
  {
    EXPECT_ANY_THROW (EXPECT_FAILURE_REGEX (functionThrow(),".*match.*"));
  }


  TEST (ExpectTest, throw_regex_match)
  {
    EXPECT_THROW_REGEX (throw std::runtime_error ("a matched runtime exception"), "a matched runtime exception");
  }

  TEST (ExpectTest, throw_regex_match_preamble)
  {
    EXPECT_THROW_REGEX (throw std::runtime_error ("pre-amble, a matched runtime exception"), "a matched runtime exception");
  }

  TEST (ExpectTest, throw_regex_match_postamble)
  {
    EXPECT_THROW_REGEX (throw std::runtime_error ("a matched runtime exception, post-amble"), "a matched runtime exception");
  }

  TEST (ExpectTest, throw_regex_missmatch)
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_THROW_REGEX (throw std::runtime_error ("a miss-matched runtime exception"), "a matched runtime exception"), "a matched runtime exception");
    EXPECT_NONFATAL_FAILURE (EXPECT_THROW_REGEX (throw std::runtime_error ("a miss-matched runtime exception"), "a matched runtime exception"), "a miss-matched runtime exception");
  }

  TEST (ExpectTest, throw_regex_nonstd)
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_THROW_REGEX (throw std::string ("a non std::exception"), "a matched runtime exception"), "threw an exception that didn't derive from std::exception");
  }

  TEST (ExpectTest, match_regex_match)
  {
    EXPECT_MATCH_REGEX ("[a-z]x[a-z]", "aaxzz");
  }

  TEST (ExpectTest, match_regex_missmatch)
  {
    EXPECT_NONFATAL_FAILURE (EXPECT_MATCH_REGEX ("[a-z]x[a-z]", "a1xzz"), "[a-z]x[a-z]");
  }
}

int main (int argc, char **argv)
{
#ifdef XAOD_STANDALONE
  StatusCode::enableFailure();
  ANA_CHECK (xAOD::Init ());
#endif
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
