/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/MessageCheck.h>

#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace EL
{
  ANA_MSG_SOURCE (msgEventLoop, "EventLoop")

  namespace Detail
  {
    void report_exception (std::exception_ptr eptr)
    {
      using namespace msgEventLoop;
      try
      {
        if (eptr) {
            std::rethrow_exception(eptr);
        }
      } catch (std::exception& e)
      {
        ANA_MSG_ERROR ("caught exception: " << e.what());
      } catch (std::string& str)
      {
        ANA_MSG_ERROR ("caught exception string: " << str);
      } catch (const char *str)
      {
        ANA_MSG_ERROR ("caught exception string: " << str);
      } catch (...)
      {
        ANA_MSG_ERROR ("caught unknown exception");
      }
    }
  }
}
