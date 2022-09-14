/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisTools/AANTEventContext.h"
#include "CxxUtils/checker_macros.h"

//________________________________________________________________________________
AANTEventContext::AANTEventContext(const IEvtSelector* selector)
  : m_evtSelector(selector)
{
}

//________________________________________________________________________________
AANTEventContext& AANTEventContext::operator=(const AANTEventContext& ctxt)
{
  if (this != &ctxt)
    m_evtSelector = ctxt.m_evtSelector;
  return *this;
}

//________________________________________________________________________________
AANTEventContext::AANTEventContext(const AANTEventContext& ctxt)
  : IEvtSelector::Context(), m_evtSelector(ctxt.m_evtSelector)
{
}

//________________________________________________________________________________
AANTEventContext::~AANTEventContext()
{
}

//________________________________________________________________________________
void* AANTEventContext::identifier() const
{
  IEvtSelector* id ATLAS_THREAD_SAFE = const_cast<IEvtSelector*>(m_evtSelector);
  return id;
}
