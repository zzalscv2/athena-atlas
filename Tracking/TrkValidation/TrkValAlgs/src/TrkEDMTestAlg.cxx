/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//<<<<<< INCLUDES                                                       >>>>>>
#include "TrkValAlgs/TrkEDMTestAlg.h"

TrkEDMTestAlg::TrkEDMTestAlg(const std::string& name, ISvcLocator* pSvcLocator)
:
  AthAlgorithm(name, pSvcLocator),
  m_eventNum(0),
  m_numDetailedWarnings(0)
{
  declareProperty("DoDumpSummaryToFile",           m_dumpSummaryToFile=true);
  declareProperty("SummaryDumpFileName",           m_summaryDumpFileName="TrkEDMTestAlg.summary.log");
  declareProperty("DoDumpToFile",                  m_dumpToFile=true);
  declareProperty("DoDumpToMsgStream",             m_dumpToMsg=false);
  declareProperty("DoDetailedChecks",              m_doDetailedChecks=false);
}

//--------------------------------------------------------------------------
TrkEDMTestAlg::~TrkEDMTestAlg(void)
= default;

//-----------------------------------------------------------------------
StatusCode
TrkEDMTestAlg::initialize()
{
  // summary is special (normally want to write it)
  if (m_dumpSummaryToFile) {
    m_summaryFileOutput.open(m_summaryDumpFileName.c_str());
    ATH_MSG_VERBOSE("SUM dump="<<m_summaryDumpFileName);
  }

  return AthAlgorithm::initialize();
}

//-------------------------------------------------------------------------
StatusCode
TrkEDMTestAlg::execute()
{
  ++m_eventNum;
  return runTest();
}

//---------------------------------------------------------------------------

StatusCode
TrkEDMTestAlg::finalize()
{
  ATH_MSG_INFO("Number of events processed = "<<m_eventNum);
  if (m_numDetailedWarnings>0) ATH_MSG_ERROR(" Found "<<m_numDetailedWarnings<<" problems! Please check log for more details.");

  if (m_dumpSummaryToFile) m_summaryFileOutput  <<"Summary"<<std::endl<<"-------"<<std::endl;

  for (const auto & [SGKey, objCount] : m_numContsFound ) {
    const int numberOfObjects = m_numConstObjsFound[SGKey];
    ATH_MSG_INFO("Found :"<<objCount<<" \t at "<<SGKey<<", with "<<numberOfObjects<<" total contained objects.");
    if (m_dumpSummaryToFile){
      m_summaryFileOutput <<"Found :"<<objCount<<" \t at "<<SGKey<<", with "<<numberOfObjects<<" total contained objects."<<std::endl;
    } 
  }
  if (m_dumpSummaryToFile){
    if (m_doDetailedChecks) m_summaryFileOutput<<"Detailed tests found : "<<m_numDetailedWarnings<<" problems.";
    m_summaryFileOutput.close();
  }

  return StatusCode::SUCCESS;
}
