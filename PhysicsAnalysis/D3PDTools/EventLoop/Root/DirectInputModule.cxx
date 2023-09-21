/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/DirectInputModule.h>

#include <EventLoop/IInputModuleActions.h>
#include <EventLoop/EventRange.h>
#include <EventLoop/MessageCheck.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode DirectInputModule ::
    processInputs (ModuleData& /*data*/, IInputModuleActions& actions)
    {
      using namespace msgEventLoop;
      Long64_t toSkip = this->skipEvents.has_value() ? this->skipEvents.value() : 0u;
      std::optional<Long64_t> toProcess;
      if (this->maxEvents.has_value())
        toProcess = this->maxEvents.value();
      for (const std::string& fileName : fileList)
      {
        // open the input file to inspect it
        ANA_CHECK (actions.openInputFile (fileName));

        EventRange eventRange;
        eventRange.m_url = fileName;
        eventRange.m_endEvent = actions.inputFileNumEntries();

        // this has to be `>` not `>=` to ensure that we don't accidentally skip
        // empty files.
        if (toSkip > eventRange.m_endEvent)
        {
          toSkip -= eventRange.m_endEvent;
          continue;
        }

        eventRange.m_beginEvent = toSkip;
        toSkip = 0;

        if (toProcess.has_value())
        {
          if (eventRange.m_endEvent >= eventRange.m_beginEvent + toProcess.value())
          {
            eventRange.m_endEvent = eventRange.m_beginEvent + toProcess.value();
            toProcess = 0u;
          } else
          {
            toProcess.value() -= eventRange.m_endEvent - eventRange.m_beginEvent;
            continue;
          }
        }
        ANA_CHECK (actions.processEvents (eventRange));
        if (toProcess.has_value() && toProcess.value() == 0u)
          break;
      }
      return StatusCode::SUCCESS;
    }
  }
}
