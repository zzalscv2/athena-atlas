/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//          Copyright Nils Krumnack 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (krumnack@iastate.edu) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <EventLoop/LSFDriver.h>

#include <AsgTools/StatusCode.h>
#include <EventLoop/Job.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/ThrowMsg.h>
#include <TSystem.h>
#include <sstream>

//
// method implementations
//

ClassImp(EL::LSFDriver)

namespace EL
{
  void LSFDriver ::
  testInvariant () const
  {
    RCU_INVARIANT (this != 0);
  }



  LSFDriver ::
  LSFDriver ()
  {
    RCU_NEW_INVARIANT (this);
  }



  ::StatusCode LSFDriver ::
  doManagerStep (Detail::ManagerData& data) const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgEventLoop;
    ANA_CHECK (BatchDriver::doManagerStep (data));
    switch (data.step)
    {
    case Detail::ManagerStep::submitJob:
    case Detail::ManagerStep::doResubmit:
      {
        // safely ignoring: resubmit

        std::ostringstream cmd;
        cmd << "cd " << data.submitDir << "/submit";
        for (std::size_t iter : data.batchJobIndices)
        {
          cmd << " && bsub " << data.options.castString (Job::optSubmitFlags);
          if (data.options.castBool (Job::optResetShell, true))
            cmd << " -L /bin/bash";
          cmd << " " << data.submitDir << "/submit/run " << iter;
        }
        if (gSystem->Exec (cmd.str().c_str()) != 0)
          RCU_THROW_MSG (("failed to execute: " + cmd.str()).c_str());
        data.submitted = true;
      }
      break;

    default:
      break;
    }
    return ::StatusCode::SUCCESS;
  }
}
