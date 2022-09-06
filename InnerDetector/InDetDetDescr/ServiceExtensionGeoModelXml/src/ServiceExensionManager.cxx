/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ServiceExtensionManager.h"

namespace ITk {

ServiceExtensionManager::ServiceExtensionManager() 
{
  setName("ITkServiceExtension");
}



ServiceExtensionManager::~ServiceExtensionManager()
{
}



unsigned int 
ServiceExtensionManager::getNumTreeTops() const
{
  return m_volume.size(); 
}

PVConstLink 
ServiceExtensionManager::getTreeTop(unsigned int i) const
{
  return m_volume[i];
}

void 
ServiceExtensionManager::addTreeTop(const PVConstLink& vol){
  m_volume.push_back(vol);
}

} // namespace ITk


