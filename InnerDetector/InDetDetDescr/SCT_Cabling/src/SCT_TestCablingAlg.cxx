/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_TestCablingAlg.cxx
 * Implementation file for  SCT cabling service test algorithm
 * @author Shaun Roe
 * @date 20 October, 2008
 **/
 
//Package
#include "SCT_TestCablingAlg.h"

#include "SCT_CablingUtilities.h"
#include "SCT_CablingXmlTags.h"

//Athena
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/SCT_ID.h"

//Gaudi
#include "GaudiKernel/StatusCode.h"

//STL
#include <set>
#include <cstdint>
//for o/p to file
#include <iostream>
#include <fstream>
//to access environment
#include <cstdlib>
#include <algorithm>
#include <map>

using namespace std;
using namespace SCT_Cabling;
 
SCT_TestCablingAlg::SCT_TestCablingAlg (const std::string& name, ISvcLocator* pSvcLocator): AthAlgorithm(name, pSvcLocator),
  m_idHelper(0){
  //nop
}

SCT_TestCablingAlg::~SCT_TestCablingAlg(){
  //nop
}

StatusCode
SCT_TestCablingAlg::initialize(){
  ATH_CHECK(detStore()->retrieve(m_idHelper,"SCT_ID"));
  ATH_CHECK(m_cablingTool.retrieve());
  ATH_MSG_INFO("Test algorithm for SCT_Cabling");
  return StatusCode::SUCCESS;
}

std::string
SCT_TestCablingAlg::coordString(const Identifier & offlineId){
  using std::to_string;
  const std::string sep(", ");
  std::string result=std::string("[") + to_string(m_idHelper->barrel_ec(offlineId)) + sep;
  result+=to_string(m_idHelper->layer_disk(offlineId)) + sep;
  result+=to_string(m_idHelper->phi_module(offlineId)) + sep;
  result+=to_string(m_idHelper->eta_module(offlineId)) + sep;
  result+=to_string(m_idHelper->side(offlineId)) + "]";
  return result;
}

StatusCode
SCT_TestCablingAlg::execute(){
  const string testAreaPath=CoveritySafe::getenv("TestArea");
  string filename=testAreaPath+"/cabling.txt";
  msg(MSG::INFO)<<"Filename: "<<filename<<" will be written to your $TestArea."<<endmsg;
  ofstream opFile1(filename.c_str(),ios::out);
  msg(MSG::INFO)<<"Executing..."<<endmsg;
  msg(MSG::INFO)<<"hash, offline Id, online Id(hex), serial number"<<endmsg;
  const unsigned int nHashesInCabling(2*m_cablingTool->size());
  for (unsigned int i(0);i!=nHashesInCabling;++i){
    IdentifierHash hash(i);
    Identifier offlineId(m_idHelper->wafer_id(hash));
    SCT_OnlineId onlineId(m_cablingTool->getOnlineIdFromHash(hash));
    SCT_SerialNumber sn(m_cablingTool->getSerialNumberFromHash(hash));
    msg(MSG::INFO)<<i<<" "<<offlineId<<" "<<hex<<onlineId<<dec<<" "<<sn<<" "<<coordString(offlineId)<<endmsg;
    opFile1<<i<<" "<<offlineId<<" "<<hex<<onlineId<<dec<<" "<<sn<<" "<<coordString(offlineId)<<std::endl;
    if (m_cablingTool->getHashFromOnlineId(onlineId) != hash){
      msg(MSG::ERROR)<<"?? "<<m_cablingTool->getHashFromOnlineId(onlineId)<<endmsg;
    }
  }
  opFile1.close();
  msg(MSG::INFO)<<"Size: "<<m_cablingTool->size()<<endmsg;
  std::vector<unsigned int> rods;
  m_cablingTool->getAllRods(rods);
  msg(MSG::INFO)<<"Num. of rods= "<<rods.size()<<endmsg;
  msg(MSG::INFO)<<"First rod id "<<std::hex<<rods[0]<<std::dec<<endmsg;
  string sn("20220130000299");
  msg(MSG::INFO)<<"Hash from serial number "<<m_cablingTool->getHashFromSerialNumber(sn)<<endmsg;
  int tsn(130000299);
  msg(MSG::INFO)<<"Hash from truncated serial number "<<m_cablingTool->getHashFromSerialNumber(tsn)<<endmsg;
  unsigned long long snll(20220130000299LL);
  msg(MSG::INFO)<<"Hash from truncated serial number "<<m_cablingTool->getHashFromSerialNumber(snll)<<endmsg;
  msg(MSG::INFO)<<"Hash from onlineid "<<m_cablingTool->getHashFromOnlineId(0x3d230006)<<endmsg;
  msg(MSG::INFO)<<"Hash from invalid onlineid "<<m_cablingTool->getHashFromOnlineId(0x3d250006)<<endmsg;
  msg(MSG::INFO)<<"Hash from textfile onlineid "<<m_cablingTool->getHashFromOnlineId(0x3d220010)<<endmsg;
  msg(MSG::INFO)<<"Hash from db onlineid "<<m_cablingTool->getHashFromOnlineId(0x3d220105)<<endmsg;
  std::vector<IdentifierHash> hashVec;
  m_cablingTool->getHashesForRod(hashVec, 0x220005);
  msg(MSG::INFO)<<"number of hashes for rod 0x220005: "<<hashVec.size()<<endmsg;
  //new section December 2014
  
  //make a 'rodHash'
  std::sort(rods.begin(),rods.end());
  std::map<unsigned int, unsigned int> rodHashMap;
  for (unsigned int i(0);i!=rods.size();++i){
    rodHashMap[rods[i]]=i;
  }
  //open a file for writing
  const std::string fullFileName=testAreaPath + "/" + makeCablingFileName();
  msg(MSG::INFO)<<"Open file for write "<<fullFileName<<endmsg;
  ofstream opFile(fullFileName.c_str(),ios::out);
  opFile<<XmlHeader<<endl;
  opFile<<OpenRootTag<<endl;
  opFile<<OpenRodMappingTag<<endl;
  for (auto r:rods){
    opFile<<formatRodOutput(r)<<endl;
  }
  opFile<<CloseRodMappingTag<<endl;
  opFile<<OpenModuleMappingTag<<endl;
   for (unsigned int i(0);i!=nHashesInCabling;++i){
    IdentifierHash hash(i);
    Identifier offlineId(m_idHelper->wafer_id(hash));
    SCT_OnlineId onlineId(m_cablingTool->getOnlineIdFromHash(hash));
    SCT_SerialNumber sn(m_cablingTool->getSerialNumberFromHash(hash));
    //rod, fibre, bec, layerDisk, eta,  phi, side,  robId, sn
    const int bec(m_idHelper->barrel_ec(offlineId));
    const int layer(m_idHelper->layer_disk(offlineId));
    const int eta(m_idHelper->eta_module(offlineId));
    const int phi(m_idHelper->phi_module(offlineId));
    const int side(m_idHelper->side(offlineId));
    const unsigned int rodIndex(rodHashMap[onlineId.rod()]);
    opFile<<formatModuleOutput(rodIndex, onlineId.fibre(), bec,layer,eta,phi,side,onlineId.rod()  ,sn.str())<<endl;
    
  }
  opFile<<CloseModuleMappingTag<<endl;
  opFile<<CloseRootTag<<endl;
  opFile.close();
  return StatusCode::SUCCESS;
}


StatusCode
SCT_TestCablingAlg::finalize(){
  msg(MSG::INFO)<<"finalize()"<<endmsg;
  return StatusCode::SUCCESS;
}
