/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "StringUtilities.h"
#include <sstream>
//getline is in <string>, already included

namespace PixelConditionsAlgorithms{

   //parses the data_array string from the pixel dead map conditions db
  std::vector<std::pair<int, int>>//module hash, module status
  parseDeadMapString(const std::string & dataArrayString){
    std::vector<std::pair<int, int>> result;
    if (dataArrayString.empty()) return result;
    //
    std::stringstream ss(dataArrayString);
    std::vector<std::string> component;
    std::string buffer;
    while (std::getline(ss,buffer,',')) { 
      component.push_back(buffer); 
    }
    for (const auto & elem : component) {
      std::stringstream checkModule(elem);
      std::vector<std::string> moduleString;
      while (std::getline(checkModule,buffer,':')) { 
        moduleString.push_back(buffer); 
      }
      if (moduleString.size()==2) {
        std::stringstream checkModuleHash(moduleString[0]);
        std::vector<std::string> moduleStringHash;
        while (std::getline(checkModuleHash,buffer,'"')) { 
          moduleStringHash.push_back(buffer); 
        }
        const int moduleHash   = std::stoi(moduleStringHash[1]);
        const int moduleStatus = std::stoi(moduleString[1]);
        result.push_back({moduleHash, moduleStatus}); 
      }
    }
    //
    return result;
  }
}