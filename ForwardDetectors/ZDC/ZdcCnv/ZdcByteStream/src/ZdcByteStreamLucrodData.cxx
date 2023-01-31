/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcByteStream/ZdcByteStreamLucrodData.h"
#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "ZdcByteStream/ZdcLucrodData.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

ZdcByteStreamLucrodData::ZdcByteStreamLucrodData(const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm     (name                , pSvcLocator),
  m_robDataProvider("ROBDataProviderSvc", name) {
  
}

StatusCode ZdcByteStreamLucrodData::initialize() {
  
  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::initialize");

  StatusCode sc = m_robDataProvider.retrieve();
  
  if (sc.isFailure()) ATH_MSG_WARNING(" Could not retrieve ROBDataProviderSvc ");
  else                ATH_MSG_DEBUG  (" Retrieved service ROBDataProviderSvc ");

  ATH_CHECK( m_ZdcLucrodDataContainerKey.initialize() );
  
  return StatusCode::SUCCESS;
}

StatusCode ZdcByteStreamLucrodData::execute() {
  
  ATH_MSG_DEBUG(" ZdcByteStreamLucrodData::execute ");

  SG::WriteHandle<ZdcLucrodDataContainer> h_write(m_ZdcLucrodDataContainerKey);
  
  std::vector<const ROBFragment*> listOfRobf;
  std::vector<unsigned int> ROBIDs;
  
  ROBIDs.push_back(ROD_SOURCE_ID);
  ROBIDs.push_back(ROD_SOURCE_ID+1);
  ROBIDs.push_back(ROD_SOURCE_ID+2);
  ROBIDs.push_back(ROD_SOURCE_ID+3);
  ROBIDs.push_back(ROD_SOURCE_ID+4);
  ROBIDs.push_back(ROD_SOURCE_ID+5);

  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::getROBDATA");
  m_robDataProvider->getROBData(ROBIDs, listOfRobf);
  
  auto zdcLucrodDataContainer = std::make_unique<ZdcLucrodDataContainer>(); 
  
  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::filleContainer");
  StatusCode sc = fillContainer(listOfRobf,zdcLucrodDataContainer.get());
  
  if (sc.isFailure()) ATH_MSG_WARNING(" fillContainer failed ");
  else                ATH_MSG_DEBUG  (" fillContainer success ");

  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::record m_ZdcLucrodDataContainerKey");

  sc = h_write.record(std::move(zdcLucrodDataContainer));
  
  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::record testing...");

  if (sc.isFailure()) ATH_MSG_WARNING(" Could not record ZdcLucrodDataContainer in StoreGate ");
  else                ATH_MSG_DEBUG  (" ZdcLucrodDataContainer is recorded in StoreGate ");
  
  return StatusCode::SUCCESS;
}

StatusCode ZdcByteStreamLucrodData::fillContainer(std::vector<const ROBFragment*> listOfRobf, ZdcLucrodDataContainer* zdcLucrodDataContainer) {
  
  ATH_MSG_DEBUG(" ZdcByteStreamLucrodData::fillContainer ");
 
  int nFragments = listOfRobf.size();
    
  ATH_MSG_DEBUG(" Number of ROB fragments: " << nFragments);
  
  if (!nFragments) return StatusCode::SUCCESS;

  std::vector<const ROBFragment*>::const_iterator rob_it  = listOfRobf.begin();
  std::vector<const ROBFragment*>::const_iterator rob_end = listOfRobf.end();
  
  for (; rob_it != rob_end; ++rob_it) {
    
    uint32_t robid = (*rob_it)->rob_source_id();
    
    ATH_MSG_DEBUG(" ROB Fragment with ID: 0x" << std::hex << robid << std::dec); 

    uint32_t lucrodID = robid & 0x7;

    ZdcLucrodData* zld = new ZdcLucrodData(lucrodID);
    
    StatusCode sc = m_ZdcLucrodDecoder.decode(&**rob_it, zld);
    
    if (sc.isFailure()) ATH_MSG_WARNING(" Conversion from ByteStream to ZdcLucrodData failed ");
    else zdcLucrodDataContainer->push_back(zld);
  }

  for (auto zld : *zdcLucrodDataContainer) {zld->str();}

  return StatusCode::SUCCESS;
}

StatusCode ZdcByteStreamLucrodData::finalize() { 
  
  ATH_MSG_DEBUG(" ZdcByteStreamLucrodData::finalize ");
  
  return StatusCode::SUCCESS;
}
