/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1RPClogic/RPCdata.h"


RPCdata::RPCdata(const std::string& name) : BaseObject(Data,name) {}

RPCdata::RPCdata(const char* name) : BaseObject(Data,name) {}

RPCdata::RPCdata(const RPCdata& data) : BaseObject(Data,data.name()) {}

RPCdata::~RPCdata() {}

