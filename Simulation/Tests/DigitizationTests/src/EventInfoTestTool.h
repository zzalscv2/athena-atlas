/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DIGITIZATIONTESTS_EVENTINFOTESTTOOL
#define DIGITIZATIONTESTS_EVENTINFOTESTTOOL

#include "DigiTestToolBase.h"
#include "xAODEventInfo/EventInfo.h"

class EventInfoTestTool : public DigiTestToolBase {
 public:
  EventInfoTestTool( const std::string& type,
                    const std::string& name,
                    const IInterface* parent );

  StatusCode initialize();

  StatusCode processEvent();

  StatusCode finalize();

 private:
  SG::ReadHandleKey<xAOD::EventInfo> m_rhkEventInfo{
      this, "EventInfoName", "EventInfo", "EventInfo name"};
};

#endif
