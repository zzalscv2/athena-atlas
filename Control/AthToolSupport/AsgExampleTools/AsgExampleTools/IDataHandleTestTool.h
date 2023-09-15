/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_TOOLS__I_DATA_HANDLE_TEST_TOOL_H
#define ASG_TOOLS__I_DATA_HANDLE_TEST_TOOL_H

#include <AsgTools/IAsgTool.h>
#include "CxxUtils/checker_macros.h"

namespace asg
{
  /// \brief the interface for \ref DataHandleTestTool

  class IDataHandleTestTool : virtual public IAsgTool
  {
    // Declare the interface that this class provides
    ASG_TOOL_INTERFACE( CP::IDataHandleTestTool )

    /// \brief run the test
  public:
    virtual void runTest ATLAS_NOT_THREAD_SAFE () = 0;
  };
}

#endif
