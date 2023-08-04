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
#include <AsgExampleTools/UnitTestTool1.h>
#include <AsgExampleTools/UnitTestTool2.h>
#include <AsgExampleTools/UnitTestTool3.h>
#include <cmath>
#include <gtest/gtest.h>
#include <string>

//
// method implementations
//

using namespace asg::msgUserCode;

namespace asg
{
  namespace
  {
    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ()
    {
      static std::atomic<unsigned> index = 0;
      return "unique" + std::to_string(++index);
    }
  }



  TEST (AsgToolConfigTest, basic_linkdef)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool1/" + name);
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool1> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
  }



  TEST (AsgToolConfigTest, basic)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool1A/" + name);
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool1> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    EXPECT_TRUE (tool->isInitialized());
    EXPECT_EQ (-7, tool->getPropertyInt());
    EXPECT_EQ ("", tool->getPropertyString());
  }



  TEST (AsgToolConfigTest, basic_properties)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool1A/" + name);
    EXPECT_SUCCESS (config.setProperty ("propertyInt", 17));
    EXPECT_SUCCESS (config.setProperty ("propertyString", "alpha"));
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool1> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    EXPECT_TRUE (tool->isInitialized());
    EXPECT_EQ (17, tool->getPropertyInt());
    EXPECT_EQ ("alpha", tool->getPropertyString());
  }



  TEST (AsgToolConfigTest, basic_propertyFromString)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool1A/" + name);
    config.setPropertyFromString ("propertyInt", "17");
    config.setPropertyFromString ("propertyString", "'alpha'");
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool1> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    EXPECT_TRUE (tool->isInitialized());
    EXPECT_EQ (17, tool->getPropertyInt());
    EXPECT_EQ ("alpha", tool->getPropertyString());
  }



  TEST (AsgToolConfigTest, basic_subtoolEmpty)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool2/" + name);
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool2> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    EXPECT_FAILURE (tool->retrieveToolHandle("regPrivateHandle"));
    EXPECT_FALSE (tool->wasUserConfigured("anaPrivateHandle"));
  }



  TEST (AsgToolConfigTest, basic_subtoolConfigure)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool2/" + name);
    ASSERT_SUCCESS (config.createPrivateTool ("regPrivateHandle", "asg::UnitTestTool1A"));
    config.setPropertyFromString ("regPrivateHandle.propertyInt", "17");
    ASSERT_SUCCESS (config.createPrivateTool ("anaPrivateHandle", "asg::UnitTestTool1A"));
    config.setPropertyFromString ("anaPrivateHandle.propertyInt", "42");
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool2> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    EXPECT_SUCCESS (tool->retrieveToolHandle("regPrivateHandle"));
    EXPECT_TRUE (tool->getToolHandle ("regPrivateHandle")->isInitialized());
    EXPECT_EQ (17, tool->getToolHandle ("regPrivateHandle")->getPropertyInt());
    EXPECT_TRUE (tool->wasUserConfigured("anaPrivateHandle"));
    EXPECT_TRUE (tool->getToolHandle ("anaPrivateHandle")->isInitialized());
    EXPECT_EQ (42, tool->getToolHandle ("anaPrivateHandle")->getPropertyInt());
  }



  TEST (AsgToolConfigTest, basic_subtoolConfigureIndirect)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig config ("asg::UnitTestTool2/" + name);
    {
      AsgToolConfig subconfig ("asg::UnitTestTool1A");
      subconfig.setPropertyFromString ("propertyInt", "17");
      ASSERT_SUCCESS (config.addPrivateTool ("regPrivateHandle", subconfig));
    }
    {
      AsgToolConfig subconfig ("asg::UnitTestTool1A");
      subconfig.setPropertyFromString ("propertyInt", "42");
      ASSERT_SUCCESS (config.addPrivateTool ("anaPrivateHandle", subconfig));
    }
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool2> tool;
    ASSERT_SUCCESS (config.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    EXPECT_SUCCESS (tool->retrieveToolHandle("regPrivateHandle"));
    EXPECT_TRUE (tool->getToolHandle ("regPrivateHandle")->isInitialized());
    EXPECT_EQ (17, tool->getToolHandle ("regPrivateHandle")->getPropertyInt());
    EXPECT_TRUE (tool->wasUserConfigured("anaPrivateHandle"));
    EXPECT_TRUE (tool->getToolHandle ("anaPrivateHandle")->isInitialized());
    EXPECT_EQ (42, tool->getToolHandle ("anaPrivateHandle")->getPropertyInt());
  }



  TEST (AsgToolConfigTest, privateTool)
  {
    const std::string name1 = makeUniqueName();
    AsgToolConfig config1 ("asg::UnitTestTool1A/" + name1);
    std::shared_ptr<void> cleanup1;
    ToolHandle<IUnitTestTool1> tool1;
    ASSERT_SUCCESS (config1.makeTool (tool1, cleanup1));
    EXPECT_EQ ("ToolSvc." + name1, tool1->name());

    AsgToolConfig config2 ("asg::UnitTestTool1A/myPrivateTool");
    std::shared_ptr<void> cleanup2;
    ToolHandle<IUnitTestTool1> tool2 ("", &*tool1);
    ASSERT_SUCCESS (config2.makeTool (tool2, cleanup2));
    EXPECT_EQ ("ToolSvc." + name1 + ".myPrivateTool", tool2->name());
  }



  TEST (AsgToolConfigTest, emptyArray)
  {
    const std::string name = makeUniqueName();
    AsgToolConfig mainConfig ("asg::UnitTestTool2/" + name);
    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool2> tool;
    ASSERT_SUCCESS (mainConfig.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    ASSERT_EQ (0u, tool->getArray().size());
  }



  TEST (AsgToolConfigTest, fillArray)
  {
    const std::string name = makeUniqueName();

    AsgToolConfig mainConfig ("asg::UnitTestTool2/" + name);
    {
      auto subtool = mainConfig.createPrivateToolInArray ("regPrivateArray", "asg::UnitTestTool1");
      ANA_MSG_INFO ("subtool = " << subtool);
      ASSERT_SUCCESS (mainConfig.setProperty (subtool + ".propertyInt", 19));
    }
    {
      AsgToolConfig subconfig ("asg::UnitTestTool1");
      ANA_CHECK_THROW (subconfig.setProperty ("propertyInt", 22));
      auto subtool = mainConfig.addPrivateToolInArray ("regPrivateArray", subconfig);
      ANA_MSG_INFO ("subtool = " << subtool);
    }

    std::shared_ptr<void> cleanup;
    ToolHandle<IUnitTestTool2> tool;
    ASSERT_SUCCESS (mainConfig.makeTool (tool, cleanup));
    EXPECT_EQ ("ToolSvc." + name, tool->name());
    ASSERT_EQ (2u, tool->getArray().size());
    EXPECT_EQ (19, tool->getArray()[0]->getPropertyInt());
    EXPECT_EQ (22, tool->getArray()[1]->getPropertyInt());
  }



  TEST (AsgToolConfigTest, makePrivateTool)
  {
    const std::string name1 = makeUniqueName();
    AsgToolConfig config1 ("asg::UnitTestTool1A/" + name1);
    std::shared_ptr<void> cleanup1;
    ToolHandle<IUnitTestTool1> tool1;
    ASSERT_SUCCESS (config1.makeTool (tool1, cleanup1));
    EXPECT_EQ ("ToolSvc." + name1, tool1->name());

    AsgToolConfig config2 ("asg::UnitTestTool1A/myPrivateTool");
    ToolHandle<IUnitTestTool1> tool2 ("", &*tool1);
    ASSERT_SUCCESS (config2.makePrivateTool (tool2));
    EXPECT_EQ ("ToolSvc." + name1 + ".myPrivateTool", tool2->name());
  }
}

ATLAS_GOOGLE_TEST_MAIN
