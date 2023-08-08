/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ModuleVetoTool.cxx
 * implementation file for service allowing one to declare modules as bad
 * @author shaun.roe@cern.ch
 **/

#include "SCT_ModuleVetoTool.h"
#include <nlohmann/json.hpp>

//STL includes
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//Athena includes
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SCT_DetectorElementStatus.h"

using json = nlohmann::json;

static const std::string databaseSignature{"database"};

// Constructor
SCT_ModuleVetoTool::SCT_ModuleVetoTool(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent)
{
}

//Initialize
StatusCode 
SCT_ModuleVetoTool::initialize() {
  if (m_maskLayers and m_layersToMask.size()==0 and m_disksToMask.size()==0) {
    ATH_MSG_INFO("Layer/Disk masking enabled, but no layer/disk specified!");
    m_maskLayers = false;
  }
  
  if ((not m_maskLayers) and (m_layersToMask.size() or m_disksToMask.size())) {
    ATH_MSG_INFO("Layer/Disk to mask specified, but masking is disabled!");
  } 

  if ((not m_maskLayers) and m_maskSide!=-1) {
    ATH_MSG_INFO("Layer/Disk side to mask specified, but masking is disabled!");
  } 
  
  if (m_maskLayers and m_disksToMask.size() and (std::find(m_disksToMask.begin(), m_disksToMask.end(),0)!=m_disksToMask.end())) {
    ATH_MSG_WARNING("0th Disk not defined (-N to N) - check your setup!");
  }   

  if (detStore()->retrieve(m_pHelper, "SCT_ID").isFailure()) {
    ATH_MSG_FATAL("SCT helper failed to retrieve");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("Successfully retrieved SCT_ID helper");
  }

  // If "database" is found in m_badElements, COOL database is used.
  m_useDatabase=(std::find(m_badElements.value().begin(), m_badElements.value().end(), databaseSignature) != m_badElements.value().end());
  ATH_MSG_INFO("m_useDatabase is " << m_useDatabase);

  if (not m_useDatabase) {
    if (fillData().isFailure()) {
      ATH_MSG_FATAL("Failed to fill data");
      return StatusCode::FAILURE;
    }
  }

  // Read Cond Handle
  ATH_CHECK(m_condKey.initialize(m_useDatabase));

  const std::string databaseUseString{m_useDatabase ? "" : "not "};
  ATH_MSG_INFO("Initialized veto service with data, "
               << (m_badElements.value().size() - static_cast<long unsigned int>(m_useDatabase))
               << " elements declared bad. Database will " << databaseUseString << "be used.");

  return StatusCode::SUCCESS;
}

//Finalize
StatusCode
SCT_ModuleVetoTool::finalize() {
  return StatusCode::SUCCESS;
}

bool 
SCT_ModuleVetoTool::canReportAbout(InDetConditions::Hierarchy h) const {
  return ((h==InDetConditions::DEFAULT) or (h==InDetConditions::SCT_SIDE));
}

bool 
SCT_ModuleVetoTool::isGood(const Identifier& elementId, const EventContext& ctx, InDetConditions::Hierarchy h) const {
  if (not canReportAbout(h)) return true;

  const Identifier waferId{m_pHelper->wafer_id(elementId)};

  // Bad wafer in properties
  if (m_localCondData.isBadWaferId(waferId)) return false;
  // If database is not used, all wafer IDs here should be good.
  if (not m_useDatabase) return true;

  const SCT_ModuleVetoCondData* condData{getCondData(ctx)};
  // If database cannot be retrieved, all wafer IDs are good.
  if (condData==nullptr) return true;

  // Return the result of database
  return (not condData->isBadWaferId(waferId));
}

bool 
SCT_ModuleVetoTool::isGood(const Identifier& elementId, InDetConditions::Hierarchy h) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return isGood(elementId, ctx, h);
}

bool 
SCT_ModuleVetoTool::isGood(const IdentifierHash& hashId, const EventContext& ctx) const {
  Identifier elementId{m_pHelper->wafer_id(hashId)};
  return isGood(elementId, ctx, InDetConditions::SCT_SIDE);
}

bool
SCT_ModuleVetoTool::isGood(const IdentifierHash& hashId) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return isGood(hashId, ctx);
}

void
SCT_ModuleVetoTool::getDetectorElementStatus(const EventContext& ctx, InDet::SiDetectorElementStatus &element_status, 
                                             SG::WriteCondHandle<InDet::SiDetectorElementStatus>* whandle) const {
   std::vector<bool> &status = element_status.getElementStatus();
   if (status.empty()) {
      status.resize(m_pHelper->wafer_hash_max(),true);
   }
   for (const Identifier &wafer_id:  m_localCondData.badWaferIds()) {
      status.at( m_pHelper->wafer_hash(wafer_id) ) = false;
   }
   if (m_useDatabase) {
      SG::ReadCondHandle<SCT_ModuleVetoCondData> condDataHandle{m_condKey, ctx};
      if (not condDataHandle.isValid()) {
         ATH_MSG_ERROR("Failed to get " << m_condKey.key());
         return;
      }
      const SCT_ModuleVetoCondData* condData{ condDataHandle.cptr() };
      if (whandle) {
        whandle->addDependency (condDataHandle);
      }
      if (condData) {
         for (const Identifier &wafer_id: condData->badWaferIds()) {
            status.at( m_pHelper->wafer_hash(wafer_id) ) = false;
         }
      }
   }
}


StatusCode 
SCT_ModuleVetoTool::fillData() {
  // Reset SCT_ModuleVetoCondData
  m_localCondData.clear();
 
 // @TODO: This part should be changed to use PathResolver before using in production.

 //Read bad module IDs from json file.
  if(!m_JsonLocation.empty())
  {
    std::ifstream json_file(m_JsonLocation);
    if (!json_file.is_open()) {
        ATH_MSG_FATAL("Failed to open the json file.");
        return StatusCode::FAILURE;
    }

    json data = json::parse(json_file);

    for(const auto& i:data)
    {
      std::string id_mod = i["Decimal_ID"];
      unsigned long long id_cstring = std::stoull(id_mod);     
      m_localCondData.setBadWaferId(Identifier(id_cstring));
      ATH_MSG_DEBUG("Masking Module ID: " << id_cstring << ".");
    }
     return StatusCode::SUCCESS;
  }


  // Fill data based on properties
  StatusCode sc{StatusCode::SUCCESS};
  if ((m_badElements.value().size() - static_cast<int>(m_useDatabase)) == 0 and (not m_maskLayers)) {
    ATH_MSG_INFO("No bad modules in job options.");
    return sc;
  } 


  bool success{true};
  std::vector<std::string>::const_iterator pId{m_badElements.value().begin()};
  std::vector<std::string>::const_iterator last{m_badElements.value().end()};
  for(; pId not_eq last;++pId) {
    unsigned long long idToWrite{static_cast<unsigned long long>(atoll(pId->c_str()))};
    if (*pId != databaseSignature) success &= m_localCondData.setBadWaferId(Identifier(idToWrite));
  }

  

  if (m_maskLayers) {
    ATH_MSG_INFO("Masking " << m_layersToMask.size() << " SCT Layers");
    ATH_MSG_INFO("Masking " << m_disksToMask.size() << " SCT Disks");
    for(unsigned int i{0}; i < m_pHelper->wafer_hash_max(); i++) {
      Identifier mID{m_pHelper->wafer_id(i)};
      int bec{m_pHelper->barrel_ec(mID)};
      int side{m_pHelper->side(mID)};
      int layer_disk{m_pHelper->layer_disk(mID)};

      if ((bec ==  0 and (m_maskSide==-1 or side==m_maskSide) and (std::find(m_layersToMask.begin(), m_layersToMask.end(),    layer_disk     ) != m_layersToMask.end())) or
          (bec ==  2 and (m_maskSide==-1 or side==m_maskSide) and (std::find(m_disksToMask.begin(),  m_disksToMask.end(),    (layer_disk + 1)) != m_disksToMask.end()))  or
          (bec == -2 and (m_maskSide==-1 or side==m_maskSide) and (std::find(m_disksToMask.begin(),  m_disksToMask.end(), -1*(layer_disk + 1)) != m_disksToMask.end()))) {
        ATH_MSG_DEBUG("Masking ID Hash " << i);
        m_localCondData.setBadWaferId(mID);
      }
    }
  }
  
  ATH_MSG_INFO(m_localCondData.size() << " bad wafers are defined via properties.");

  m_localCondData.setFilled();
  ATH_MSG_DEBUG("Successfully filled bad SCT identifiers list");
  return (success ? sc : StatusCode::FAILURE);
}

const SCT_ModuleVetoCondData*
SCT_ModuleVetoTool::getCondData(const EventContext& ctx) const {
  SG::ReadCondHandle<SCT_ModuleVetoCondData> condData{m_condKey, ctx};
  return condData.retrieve();
}
