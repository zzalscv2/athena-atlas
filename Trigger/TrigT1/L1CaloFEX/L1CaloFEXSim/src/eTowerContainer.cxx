/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Code stolen shamelessly from Calorimeter/CaloEvent/src/CaloCellContainer.cxx and modified

#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eTower.h"
#include "AthenaKernel/errorcheck.h"

namespace LVL1{

eTowerContainer::eTowerContainer(SG::OwnershipPolicy ownPolicy) : 
  DataVector<LVL1::eTower>(ownPolicy)
{ 
  m_map_towerID_containerIndex.clear();
}

void eTowerContainer::push_back(float eta, float phi, float keybase, int posneg)
{
  DataVector<LVL1::eTower>::push_back(std::make_unique<eTower>(eta,phi,keybase,posneg));
}

void eTowerContainer::print() const {
  REPORT_MESSAGE_WITH_CONTEXT (MSG::WARNING, "eTowerContainer") << "eTowerContainer::print not implemented";
}


const LVL1::eTower * eTowerContainer::findTower(int towerID) const{
    const auto it = m_map_towerID_containerIndex.find(towerID);

    const int container_index = it->second;
    if (container_index < 0) {
        return nullptr;
    }
    return (*this)[container_index];
}

LVL1::eTower * eTowerContainer::findTower(int towerID){
    const auto it = m_map_towerID_containerIndex.find(towerID);

    const int container_index = it->second;
    if (container_index < 0) {
        return nullptr;
    }
    return (*this)[container_index];
}

void eTowerContainer::clearContainerMap()
{
  m_map_towerID_containerIndex.clear();
}

bool eTowerContainer::fillContainerMap(){
  clearContainerMap();
  size_t ntowers = size();
  for (size_t itower = 0; itower < ntowers; itower++) {
    const eTower * theTower = (*this)[itower];
    int towerID = theTower->constid();
    int container_index = itower;
    m_map_towerID_containerIndex.insert(std::pair<int,int>(towerID,container_index));
  }
  return true;
}

}
