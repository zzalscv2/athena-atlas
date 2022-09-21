/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "ElectronEfficiencyCorrection/ElRecomFileHelpers.h"

#include <fstream>
#include <map>

namespace {
// Reads the provided file with the mappings
// and construct the std::map
std::map<std::string, std::string>
read(const std::string& strFile)
{
  std::ifstream is(strFile.c_str());
  if (!is.is_open()) {
    return {};
  }
  std::map<std::string, std::string> result;
  while (!is.eof()) {
    std::string strLine;
    getline(is, strLine);
    int nPos = strLine.find('=');
    if ((signed int)std::string::npos == nPos)
      continue; // no '=', invalid line;
    std::string strKey = strLine.substr(0, nPos);
    std::string strVal = strLine.substr(nPos + 1, strLine.length() - nPos + 1);
    result.insert({ strKey, strVal });
  }
  return result;
};
}

// Convert reco, ID, iso and trigger key values into a
// single key according to the map file key format
std::string
ElRecomFileHelpers::convertToOneKey(const std::string& recokey,
                                    const std::string& idkey,
                                    const std::string& isokey,
                                    const std::string& trigkey)
{

  std::string key;
  // Reconstruction Key
  if (!recokey.empty()) {
    key = recokey;
  }
  // Identification Key
  if (!idkey.empty() &&
      (recokey.empty() && isokey.empty() && trigkey.empty())) {
    key = idkey;
  }
  // Isolation Key
  if ((!idkey.empty() && !isokey.empty()) && recokey.empty() &&
      trigkey.empty()) {
    key = std::string(idkey + "_" + isokey);
  }
  // Trigger Key
  if (!trigkey.empty() && !idkey.empty()) {
    // Trigger SF file with isolation
    if (!isokey.empty()) {
      key = std::string(trigkey + "_" + idkey + "_" + isokey);
    } else {
      // Trigger SF file without isolation
      key = std::string(trigkey + "_" + idkey);
    }
  }
  return key;
}

// Retrieves the value from the provided map file
// that is  associated with the provided key
std::string
ElRecomFileHelpers::getValueByKey(const std::string& file,
                                  const std::string& key)
{
  std::map<std::string, std::string> fileTomap = read(file);
  if (fileTomap.empty()) {
    return {};
  }

  auto i = fileTomap.find(key);
  if (i != fileTomap.end()) {
    return i->second;
  }
  return {};
}

