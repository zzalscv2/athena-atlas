//
// Copyright (C) 2002-20222 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "../AsgExampleAlgorithm.h"
#include "../EventStoreTestAlg.h"

#include "AsgExampleTools/AsgHelloTool.h"
#include <AsgExampleTools/DataHandleTestTool.h>
#include "AsgExampleTools/EventStoreTestTool.h"
#include <AsgExampleTools/UnitTestService1.h>
#include <AsgExampleTools/UnitTestTool1.h>
#include <AsgExampleTools/UnitTestTool1A.h>
#include <AsgExampleTools/UnitTestTool2.h>
#include <AsgExampleTools/UnitTestTool3.h>

// Declare all algorithms.
DECLARE_COMPONENT( AsgExampleAlgorithm )
DECLARE_COMPONENT( asg::EventStoreTestAlg )

// Declare all tools.
DECLARE_COMPONENT( AsgHelloTool )
DECLARE_COMPONENT( asg::DataHandleTestTool )
DECLARE_COMPONENT( asg::EventStoreTestTool )
DECLARE_COMPONENT( asg::UnitTestService1 )
DECLARE_COMPONENT( asg::UnitTestTool1 )
DECLARE_COMPONENT( asg::UnitTestTool1A )
DECLARE_COMPONENT( asg::UnitTestTool2 )
DECLARE_COMPONENT( asg::UnitTestTool3 )

