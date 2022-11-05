/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCLUCRODMAPRUN3_H
#define ZDCLUCRODMAPRUN3_H

#include <nlohmann/json.hpp>
#include <iostream>
#include "AsgMessaging/AsgMessaging.h"

class ZdcLucrodMapRun3 : public asg::AsgMessaging
{

private:
  std::vector<nlohmann::json> m_lucrodInfo;
  nlohmann::json m_mainJson;

public:
  ZdcLucrodMapRun3();

  static const ZdcLucrodMapRun3* getInstance();
  const nlohmann::json& getLucrod(int i) const
  {
    ATH_MSG_DEBUG("getting LUCROD info for " << i);
    return m_lucrodInfo.at(i);
  }

};

#endif //ZDCLUCRODMAPRUN3_H
