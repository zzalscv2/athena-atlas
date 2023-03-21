/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "FPTracker/getConfigFile.h"
#include "../src/openFile.tpl"
#include <stdexcept>

namespace FPTracker{

  std::shared_ptr< std::ifstream >  getConfigFile(const std::string& dir, const std::string& fn)
  {
    std::ifstream infile;
    std::shared_ptr<std::ifstream> pfile(new std::ifstream);
    FPTracker::openFile(dir, fn, pfile);
    return pfile;
  }
  
}

