/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SystematicsHandles/SysFilterReporterCombiner.h>

#include <SystematicsHandles/SysFilterReporterParams.h>
#include <AsgMessaging/MessageCheck.h>
#include <cassert>

//
// method implementations
//

namespace CP
{
  SysFilterReporterCombiner ::
  SysFilterReporterCombiner (SysFilterReporterParams& val_params,
                             SysListHandle& systematicsList,
                             bool val_passedDefault)
    : AsgMessagingForward (&val_params)
    , m_params (val_params)
    , m_passedDefault (val_passedDefault)
  {
    assert (m_params.m_isInitialized);

    ANA_CHECK_THROW (m_params.m_eventDecisionOutputDecoration.preExecute(systematicsList));
  }



  SysFilterReporterCombiner ::
  ~SysFilterReporterCombiner () noexcept
  {
    ANA_MSG_DEBUG ("setting algorithm-filter-passed flag to " << m_passed);
    m_params.m_setFilterPassed (m_passed);
    m_params.m_total += 1;
    if (m_passed)
      m_params.m_passed += 1;
  }
}
