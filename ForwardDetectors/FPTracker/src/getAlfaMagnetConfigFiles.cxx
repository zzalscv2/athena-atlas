/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "FPTracker/getAlfaMagnetConfigFiles.h"
#include "FPTracker/getConfigFile.h"
#include "../src/openFile.tpl"
#include <sstream>
#include <stdexcept>

namespace FPTracker{


  std::shared_ptr<std::ifstream> getAlfaMagnetConfigFiles(const std::string& dir, const Side& side)
  {
    std::string fn = (side == beam1) ? "alfaTwiss1.txt":"alfaTwiss2.txt";
    return  getConfigFile( dir, fn );
  }
  
}

