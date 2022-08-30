// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-20222 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGEXAMPLETOOLS_IEVENTSTORETESTTOOL_H
#define ASGEXAMPLETOOLS_IEVENTSTORETESTTOOL_H

// Athena include(s).
#include "AsgTools/IAsgTool.h"

namespace asg {

   /// Interface for (a) tool(s) testing the event store
   class IEventStoreTestTool : virtual public IAsgTool {

   public:
      // Declare the interface that this class provides
      ASG_TOOL_INTERFACE( asg::IEventStoreTestTool )

      /// Function performing (a) test(s) with the event store.
      virtual StatusCode performTest() const = 0;

   }; // class IEventStoreTestTool

} // namespace asg

#endif // ASGEXAMPLETOOLS_IEVENTSTORETESTTOOL_H
