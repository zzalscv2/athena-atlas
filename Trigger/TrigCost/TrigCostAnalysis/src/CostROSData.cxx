/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CostROSData.h"


void CostROSData::initialize(const std::map<std::string, std::vector<uint32_t>>& rosToRobMap) {

    unsigned rosCounter = 0;

    for (const auto& rosRequest : rosToRobMap) {
      for (uint32_t robId : rosRequest.second) {
        m_robToRos[robId] = rosRequest.first;
      }
      m_rosIdToBin[rosRequest.first] = rosCounter;
      ++rosCounter;
    }

    m_rosToRob = rosToRobMap;

    m_msgStream.reset(new MsgStream(nullptr, "CostROSData"));
}


int CostROSData::getBinForROS(const std::string& rosName) const {
    try {
        return m_rosIdToBin.at(rosName);
    } catch (const std::out_of_range& e) {
        ATH_MSG_DEBUG("Bin for ROS " << rosName << " not found");
        return -1;
    }
}


std::string CostROSData::getROSForROB(uint32_t robId) const {
    try {
        return m_robToRos.at(robId);
    } catch (const std::out_of_range& e) {
        ATH_MSG_DEBUG("ROS for ROB " << getROBName(robId) << " not found");
        return "";
    }    
}

std::vector<uint32_t> CostROSData::getROBForROS(const std::string& rosName) const {
    try {
        return m_rosToRob.at(rosName);
    } catch (const std::out_of_range& e) {
        ATH_MSG_DEBUG("ROBs for ROS " << rosName << " not found");
        return std::vector<uint32_t>();
    }  
}

std::string CostROSData::getROBName(uint32_t robId) const {
    std::stringstream robName;
    robName << "0x" << std::hex << robId;
    return robName.str();
}

MsgStream& CostROSData::msg() const {
  return *m_msgStream;
}

MsgStream& CostROSData::msg(const MSG::Level lvl) const {
  return *m_msgStream << lvl;
}

bool CostROSData::msgLvl(const MSG::Level lvl) const {
  return lvl >= m_msgStream->level();
}