/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCLUCRODMAPRUN3_H
#define ZDCLUCRODMAPRUN3_H

#include <nlohmann/json.hpp>
#include <iostream>
#include "AsgMessaging/AsgMessaging.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


class ZdcLucrodMapRun3 : public asg::AsgMessaging
{

private:
  std::vector<nlohmann::json> m_lucrodInfo;
  nlohmann::json m_mainJson;
  bool m_debug;

public:
  ZdcLucrodMapRun3();

  static ZdcLucrodMapRun3* getInstance();
  static void deleteInstance();
  const nlohmann::json& getLucrod(int i) 
  {
    if (m_debug) std::cout << "ZdcLucrodMapRun3: getting LUCROD info for " << i << std::endl; 
    return m_lucrodInfo.at(i);
  }
  void setDebug(bool b){m_debug = b;}

};

#endif //ZDCLUCRODMAPRUN3_H
