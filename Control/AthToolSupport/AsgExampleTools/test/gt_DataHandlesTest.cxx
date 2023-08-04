/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTools/AsgToolConfig.h>
#include <AsgMessaging/MessageCheck.h>
#include <AsgTesting/UnitTest.h>
#include <AsgExampleTools/IDataHandleTestTool.h>
#include <TClass.h>
#include <TFile.h>
#include <cmath>
#include <gtest/gtest.h>
#include <sstream>

#ifdef XAOD_STANDALONE
#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/TStore.h>
#else
#include <POOLRootAccess/TEvent.h>
#endif

#include <CxxUtils/checker_macros.h>
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // unit test

//
// method implementations
//

using namespace asg::msgUserCode;

namespace asg
{
  struct DataHandlesTest : public ::testing::Test
  {
    static void SetUpTestCase ()
    {
      // Access the dictionary
      ASSERT_NE (TClass::GetClass ("xAOD::MuonContainer"), nullptr);
      ASSERT_NE (TClass::GetClass ("asg::DataHandleTestTool"), nullptr);

      const char *test_file = getenv ("ASG_TEST_FILE_MC");
      ASSERT_NE (nullptr, test_file);
      file.reset (TFile::Open (test_file, "READ"));
      ASSERT_NE (nullptr, file);
#ifdef XAOD_STANDALONE
      event = std::make_unique<xAOD::TEvent>();
#else
      event = std::make_unique<POOL::TEvent>();
#endif
      ASSERT_SUCCESS (event->readFrom (file.get()));
      ASSERT_TRUE (event->getEntry (0) >= 0);
    }

    static void TearDownTestCase ()
    {
      event.reset ();
      file.reset ();
    }

    virtual void SetUp () override
    {
    }

    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ()
    {
      static unsigned index = 0;
      std::ostringstream str;
      str << "unique" << ++ index;
      return str.str();
    }

    static inline std::unique_ptr<TFile> file;
#ifdef XAOD_STANDALONE
    static inline std::unique_ptr<xAOD::TEvent> event;
    xAOD::TStore store;
#else
    static inline std::unique_ptr<POOL::TEvent> event;
#endif
    AsgToolConfig config {"asg::DataHandleTestTool/" + makeUniqueName()};
    std::shared_ptr<void> cleanup;
    ToolHandle<IDataHandleTestTool> tool;
  };



  // just test that a basic read handle access works
  TEST_F (DataHandlesTest, base_test)
  {
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // just test that reading unknown objects fails as it should
  TEST_F (DataHandlesTest, read_failure)
  {
    config.setPropertyFromString ("readFailure", "1");
    config.setPropertyFromString ("readKey", "MuonsFailure");
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // just test that reading unknown objects fails as it should
  TEST_F (DataHandlesTest, read_decor_failure)
  {
    config.setPropertyFromString ("readDecorFailure", "1");
    config.setPropertyFromString ("readDecorKey", "Muons.MISSING_PROPERTY");
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // test that the read-key-array works
  TEST_F (DataHandlesTest, read_array)
  {
    config.setPropertyFromString ("readKeyArray", "['Muons']");
    config.setPropertyFromString ("readArray", "1");
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // do a write handle test
  TEST_F (DataHandlesTest, write_handle)
  {
    std::string writeKey = "Muons" + makeUniqueName();
    config.setPropertyFromString ("writeKey", writeKey);
    config.setPropertyFromString ("doWriteName", writeKey);
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // do a write decor handle test
  TEST_F (DataHandlesTest, write_decor_handle)
  {
    std::string writeDecorKey = "deco_" + makeUniqueName();
    config.setPropertyFromString ("writeDecorKey", "Muons." + writeDecorKey);
    config.setPropertyFromString ("doWriteDecorName", writeDecorKey);
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // do an existing write decor handle test
  TEST_F (DataHandlesTest, write_decor_handle_existing)
  {
    config.setPropertyFromString ("writeDecorKeyExisting", "Muons.pt");
    config.setPropertyFromString ("doWriteDecorNameExisting", "pt");
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }



  // empty initial handles
  TEST_F (DataHandlesTest, empty_initial_handles)
  {
    config.setPropertyFromString ("readKeyEmpty", "Muons");
    config.setPropertyFromString ("readDecorKeyEmpty", "Muons.eta");
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    tool->runTest ();
  }
}

ATLAS_GOOGLE_TEST_MAIN
