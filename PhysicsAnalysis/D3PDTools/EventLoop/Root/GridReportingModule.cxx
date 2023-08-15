/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/GridReportingModule.h>

#include <EventLoop/MessageCheck.h>
#include <EventLoop/ModuleData.h>
#include <algorithm>
#include <fstream>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    ::StatusCode GridReportingModule ::
    onNewInputFile (ModuleData& data)
    {
      if (std::find (m_files.begin(), m_files.end(), data.m_inputFileUrl) == m_files.end())
        m_files.push_back (data.m_inputFileUrl);
      return ::StatusCode::SUCCESS;
    }



    ::StatusCode GridReportingModule ::
    onExecute (ModuleData& /*data*/)
    {
      ++m_eventsProcessed;
      return ::StatusCode::SUCCESS;
    }



    ::StatusCode GridReportingModule ::
    postFileClose (ModuleData& /*data*/)
    {
      using namespace msgEventLoop;

      // createJobSummary
      std::ofstream summaryfile("../AthSummary.txt");
      if (summaryfile.is_open()) {
        unsigned int nFiles = m_files.size();
        summaryfile << "Files read: " << nFiles << std::endl;
        for (auto& file : m_files)
          summaryfile << "  " << file << std::endl;
        summaryfile << "Events Read:    " << m_eventsProcessed << std::endl;
        summaryfile.close();
      }
      else {
        ANA_MSG_WARNING ("Failed to write summary file.");
      }
      return ::StatusCode::SUCCESS;
    }



    void GridReportingModule ::
    reportInputFailure (ModuleData& /*data*/)
    {
      using namespace msgEventLoop;

      ANA_MSG_FATAL ("encountered input error");
      exit (EC_BADINPUT);
    }
  }
}
