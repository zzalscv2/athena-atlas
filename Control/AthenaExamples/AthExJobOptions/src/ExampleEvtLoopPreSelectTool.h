/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXJOBOPTIONS_EVTLOOPPRESELECTTOOL_H
#define ATHEXJOBOPTIONS_EVTLOOPPRESELECTTOOL_H 1

#include "AthenaKernel/IAthenaEvtLoopPreSelectTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "EventInfo/EventID.h"
#include <string>


/////////////////////////////////////////////////////////////////////////////

class ExampleEvtLoopPreSelectTool : public AthAlgTool, virtual public IAthenaEvtLoopPreSelectTool {
public:
   ExampleEvtLoopPreSelectTool( const std::string&, const std::string&, const IInterface* );

// to allow access to the IAthenaEvtLoopPreSelectTool interface
   StatusCode queryInterface( const InterfaceID& riid, void** ppvIf );

// setup/teardown functions
   StatusCode initialize();
   StatusCode finalize();

// the method that decides if an event should be passed to the EventSelector
   bool passEvent(const EventIDBase& pEvent);

public:
// to resolve possible conflicts with IProperty::interfaceID()
   static const InterfaceID& interfaceID() { return IAthenaEvtLoopPreSelectTool::interfaceID(); }

private:
  EventID::event_number_t m_prescale;
};

#endif // !ATHEXJOBOPTIONS_CONCRETETOOL_H
