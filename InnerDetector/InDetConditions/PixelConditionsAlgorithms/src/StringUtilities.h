/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PixelConditionsAlgorithms_StringUtilities
#define PixelConditionsAlgorithms_StringUtilities

#include <vector>
#include <utility> //for std::pair
#include <string>

namespace PixelConditionsAlgorithms{

  //parses the data_array string from the pixel dead map conditions db
  std::vector<std::pair<int, int>>//module hash, module status
  parseDeadMapString(const std::string & dataArrayString);
}
#endif
