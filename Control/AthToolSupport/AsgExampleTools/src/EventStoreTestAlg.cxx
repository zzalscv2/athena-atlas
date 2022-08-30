//
// Copyright (C) 2002-20222 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "EventStoreTestAlg.h"

namespace asg {

   StatusCode EventStoreTestAlg::initialize() {

      // Retrieve the tool(s).
      ATH_CHECK( m_tool.retrieve() );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode EventStoreTestAlg::execute( const EventContext& ) const {

      // Execute the tool(s).
      ATH_CHECK( m_tool->performTest() );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

} // namespace asg
