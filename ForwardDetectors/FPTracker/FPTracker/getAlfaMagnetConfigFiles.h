/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_GETALFAMAGNETCONFIGFILES_H
#define FPTRACKER_GETALFAMAGNETCONFIGFILES_H

#include "FPTracker/FPTrackerConstants.h"
#include <memory>
#include <string>
#include <map>


namespace FPTracker{
  std::shared_ptr< std::ifstream >  getAlfaMagnetConfigFiles(const std::string& dir, const Side& side);
}

#endif
