/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelDCSStateData.h"

void PixelDCSStateData::setModuleStatus(const int chanNum, const int value) {
  m_moduleStatus[chanNum] = value;
}

int PixelDCSStateData::getModuleStatus(const int chanNum) const {
  auto itr = m_moduleStatus.find(chanNum);
  if (itr!=m_moduleStatus.end()) { return itr->second; }
  return PixelDCSStateData::DCSModuleState::READY;
}

