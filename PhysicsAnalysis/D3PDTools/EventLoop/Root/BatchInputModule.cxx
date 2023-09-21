/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/BatchInputModule.h>

#include <EventLoop/BatchSample.h>
#include <EventLoop/BatchSegment.h>
#include <EventLoop/EventRange.h>
#include <EventLoop/IInputModuleActions.h>
#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode BatchInputModule ::
    processInputs (ModuleData& /*data*/, IInputModuleActions& actions)
    {
      using namespace msgEventLoop;

      Long64_t beginFile = segment->begin_file;
      Long64_t endFile   = segment->end_file;
      Long64_t lastFile  = segment->end_file;
      RCU_ASSERT (beginFile <= endFile);
      Long64_t beginEvent = segment->begin_event;
      Long64_t endEvent   = segment->end_event;
      if (endEvent > 0) endFile += 1;

      for (Long64_t file = beginFile; file != endFile; ++ file)
      {
        RCU_ASSERT (std::size_t(file) < sample->files.size());
        EventRange eventRange;
        eventRange.m_url = sample->files[file];
        eventRange.m_beginEvent = (file == beginFile ? beginEvent : 0);
        eventRange.m_endEvent = (file == lastFile ? endEvent : EventRange::eof);
        ANA_CHECK (actions.processEvents (eventRange));
      }

      return StatusCode::SUCCESS;
    }
  }
}
