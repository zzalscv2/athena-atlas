/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "CostData.h"
#include "AthenaBaseComps/AthCheckMacros.h"


CostData::CostData() :
  m_costCollection(nullptr),
  m_algTotalTime(0.),
  m_liveTime(1.),
  m_lb(0),
  m_slot(0),
  m_liveTimeIsPerEvent(false),
  m_typeMapPtr(nullptr) {
}


StatusCode CostData::set(const xAOD::TrigCompositeContainer* costCollection, const xAOD::TrigCompositeContainer* rosCollection, uint32_t onlineSlot) {
  m_costCollection = costCollection;
  m_rosCollection = rosCollection;

  setOnlineSlot( onlineSlot );
  ATH_CHECK(cache());

  // Create mapping from algorithm to associated ROS requests
  m_algToRos.clear();
  size_t rosIdx = 0;
  for (const xAOD::TrigComposite* tc : *rosCollection) {
    m_algToRos[tc->getDetail<size_t>("alg_idx")].push_back(rosIdx);
    ++rosIdx;
  }

  return StatusCode::SUCCESS;
}


StatusCode CostData::cache() {
  for (const xAOD::TrigComposite* tc : costCollection()) {
    if (tc->getDetail<uint32_t>("slot") != onlineSlot()) {
      continue; // When monitoring the master slot, ignores algs running in different slots 
    }
    m_algTotalTime += (tc->getDetail<uint64_t>("stop") - tc->getDetail<uint64_t>("start"));
  }
  return StatusCode::SUCCESS;
}

void CostData::setRosToRobMap(const std::map<std::string, std::vector<uint32_t>>& rosToRobMap) {
  m_rosToRob = &rosToRobMap;
}

void CostData::setLb(uint32_t lb) {
  m_lb = lb;
}

void CostData::setOnlineSlot(uint32_t slot) {
  m_slot = slot;
}

void CostData::setLivetime(float time, bool liveTimeIsPerEvent) {
  m_liveTime = time;
  m_liveTimeIsPerEvent = liveTimeIsPerEvent;
}


bool CostData::liveTimeIsPerEvent() const {
  return m_liveTimeIsPerEvent;
}


const xAOD::TrigCompositeContainer& CostData::costCollection() const {
  if (!m_costCollection) {
    throw std::runtime_error("nullptr in CostData::costCollection(). Make sure CostData::set() is called.");
  }
  return *m_costCollection;
}


const xAOD::TrigCompositeContainer& CostData::rosCollection() const {
  if (!m_rosCollection) {
    throw std::runtime_error("nullptr in CostData::rosCollection(). Make sure CostData::set() is called.");
  }
  return *m_rosCollection;
}

const std::map<std::string, std::vector<uint32_t>>& CostData::rosToRobMap() const {
  return *m_rosToRob;
}

float CostData::algTotalTimeMilliSec() const {
  return m_algTotalTime * 1e-3; // microseconds to milliseconds
}

uint32_t CostData::lb() const {
  return m_lb;
}

uint32_t CostData::onlineSlot() const {
  return m_slot;
}

bool CostData::isMasterSlot() const {
  return (onlineSlot() == 0);
}


float CostData::liveTime() const {
  return m_liveTime;
}


const std::string& CostData::algNameToClassType(size_t algNameHash) const {
  const static std::string unknown = "UNKNOWN_CLASS";
  if (m_typeMapPtr == nullptr) {
    return unknown;
  }
  const auto it = m_typeMapPtr->find(algNameHash);
  if (it == m_typeMapPtr->end()) {
    return unknown;
  }
  return it->second;
}


void CostData::setTypeMap( const std::unordered_map<uint32_t, std::string>& typeMap ) {
  m_typeMapPtr = &typeMap;
}