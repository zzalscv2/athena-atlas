/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTools/AnaToolHandle.h>
#include <AsgMessaging/MessageCheck.h>
#include <AsgMessaging/IMessagePrinter.h>
#include <AsgMessaging/MessagePrinterOverlay.h>
#include <AsgTesting/UnitTest.h>
#include <AsgTesting/MessagePrinterMock.h>
#include <cmath>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>
#include <gmock/gmock.h>

#ifdef XAOD_STANDALONE
#include <xAODRootAccess/Init.h>
#endif

using namespace testing;

using namespace asg::msgUserCode;

//
// unit test
//

namespace asg
{
  struct MessageTool : public AsgTool
  {
    MessageTool (const std::string& name)
      : AsgTool (name) {};

    void message_error (const std::string& text)
    {
      ANA_MSG_ERROR (text);
    }
    void message_info (const std::string& text)
    {
      ANA_MSG_INFO (text);
    }
    void message_debug (const std::string& text)
    {
      ANA_MSG_DEBUG (text);
    }
  };

  TEST (AsgTool, message_default)
  {
    MessageTool mytool ("my_tool");
    EXPECT_EQ (MSG::INFO, mytool.msg().level());
  }

  TEST (AsgTool, message_setLevel)
  {
    MessageTool mytool ("my_tool");
    EXPECT_EQ (MSG::INFO, mytool.msg().level());
  }

  TEST (AsgTool, message_outputLevel)
  {
    MessageTool mytool ("my_tool");
    ASSERT_SUCCESS (mytool.setProperty ("OutputLevel", MSG::ERROR));
    EXPECT_EQ (MSG::ERROR, mytool.msg().level());
  }

  TEST (AsgTool, message_outputLevel_int)
  {
    MessageTool mytool ("my_tool");
    ASSERT_SUCCESS (mytool.setProperty ("OutputLevel", 5));
    EXPECT_EQ (MSG::ERROR, mytool.msg().level());
  }

  TEST (AsgTool, message)
  {
    for (MSG::Level level : {MSG::INFO, MSG::ERROR, MSG::DEBUG})
    {
      SCOPED_TRACE(level);

      MessagePrinterMock message_printer;
      MessagePrinterOverlay overlay (&message_printer);
      EXPECT_CALL (message_printer, print (MSG::ERROR, "my_tool", MatchesRegex (".*error text")))
        .Times (1);
      EXPECT_CALL (message_printer, print (MSG::INFO, "my_tool", MatchesRegex (".*info text")))
        .Times (level<=MSG::INFO);
      EXPECT_CALL (message_printer, print (MSG::DEBUG, "my_tool", MatchesRegex (".*debug text")))
        .Times (level<=MSG::DEBUG);

      MessageTool mytool ("my_tool");
      mytool.msg().setLevel (level);
      EXPECT_EQ (level, mytool.msg().level());

      mytool.message_error ("error text");
      mytool.message_info ("info text");
      mytool.message_debug ("debug text");
    }
  }
}

ATLAS_GOOGLE_TEST_MAIN
