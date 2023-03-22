/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_READCONFIGDATA_H
#define FPTRACKER_READCONFIGDATA_H
#include <memory>
#include <fstream>

namespace FPTracker{
  class ConfigData;
  bool readConfigData(std::shared_ptr< std::ifstream >  confDir, ConfigData&);
}

#endif
