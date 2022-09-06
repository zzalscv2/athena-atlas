// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGEXAMPLETOOLS_EVENTSTORETESTALG_H
#define ASGEXAMPLETOOLS_EVENTSTORETESTALG_H

// Local include(s).
#include "AsgExampleTools/IEventStoreTestTool.h"

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

namespace asg {

   /// Algorithm used to exercise @c asg::EventStoreTestTool in Athena
   class EventStoreTestAlg : public AthReentrantAlgorithm {

   public:
      // Inherit all of the base class's constructors
      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      /// @name Function(s) inherited from @c AthAlgorithm
      /// @{

      /// Function initialising the algorithm
      virtual StatusCode initialize() override;
      /// Function executing the algorithm
      virtual StatusCode execute( const EventContext& ctx ) const override;

      /// @}

   private:
      /// @name Algorithm properties
      /// @{

      /// The tool performing the test
      ToolHandle< IEventStoreTestTool > m_tool{ this, "Tool",
         "asg::EventStoreTestTool", "Tool performing the test" };

      /// @}

   }; // class EventStoreTestAlg

} // namespace asg

#endif // ASGEXAMPLETOOLS_EVENTSTORETESTALG_H
