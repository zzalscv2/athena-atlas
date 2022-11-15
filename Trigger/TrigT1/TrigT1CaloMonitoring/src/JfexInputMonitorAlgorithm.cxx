/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JfexInputMonitorAlgorithm.h"

JfexInputMonitorAlgorithm::JfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JfexInputMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("JfexInputMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_jFexTowerContainer"<< m_jFexTowerContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_jFexTowerContainerKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode JfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("JfexInputMonitorAlgorithm::fillHistograms");

  // Access gFex gTower container
  SG::ReadHandle<xAOD::jFexTowerContainer> jFexTowerContainer{m_jFexTowerContainerKey, ctx};
  if(!jFexTowerContainer.isValid()){
    ATH_MSG_ERROR("No jFex Tower container found in storegate  "<< m_jFexTowerContainerKey);
    return StatusCode::SUCCESS;
  }  

  // monitored variables for histograms
  auto nJfexTowers = Monitored::Scalar<int>("NJfexTowers",0.0);
  auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
  auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
  auto Towerglobaleta = Monitored::Scalar<int>("TowerGlobalEta",0.0);
  auto Towerglobalphi = Monitored::Scalar<uint32_t>("TowerGlobalPhi",0.0);
  auto Towermodule = Monitored::Scalar<uint8_t>("TowerModule",0.0);
  auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0.0);
  auto Towerchannel = Monitored::Scalar<uint8_t>("TowerChannel",0.0);
  auto TowerdataID = Monitored::Scalar<uint8_t>("TowerDataID",0.0);
  auto TowersimulationID = Monitored::Scalar<uint32_t>("TowerSimulationID",0.0);
  auto Towercalosource = Monitored::Scalar<uint8_t>("TowerCalosource",0.0);

  auto Toweretcount_barrel = Monitored::Scalar<uint16_t>("TowerEtcount_barrel",0.0);
  auto Toweretcount_tile = Monitored::Scalar<uint16_t>("TowerEtcount_tile",0.0);
  auto Toweretcount_emec = Monitored::Scalar<uint16_t>("TowerEtcount_emec",0.0);
  auto Toweretcount_hec = Monitored::Scalar<uint16_t>("TowerEtcount_hec",0.0);
  auto Toweretcount_fcal1 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal1",0.0);
  auto Toweretcount_fcal2 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal2",0.0);
  auto Toweretcount_fcal3 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal3",0.0);

  auto Towersaturationflag = Monitored::Scalar<char>("TowerSaturationflag",0.0);

  ATH_MSG_DEBUG("number of jfex towers = "<< jFexTowerContainer->size());

  unsigned int njfexTowers=jFexTowerContainer->size();
  nJfexTowers=njfexTowers;
  fill(m_packageName,nJfexTowers);

  for(const xAOD::jFexTower* jfexTowerRoI : *jFexTowerContainer){

  Towereta=jfexTowerRoI->eta();
  Towerphi=jfexTowerRoI->phi();
  fill(m_packageName,Towereta,Towerphi);

  Towerglobaleta=jfexTowerRoI->globalEta();
  Towerglobalphi=jfexTowerRoI->globalPhi();
  fill(m_packageName,Towerglobaleta,Towerglobalphi);

  Towermodule=jfexTowerRoI->module();
  fill(m_packageName,Towermodule);

  Towerfpga=jfexTowerRoI->fpga();
  fill(m_packageName,Towerfpga);

  Towerchannel=jfexTowerRoI->channel();
  fill(m_packageName,Towerchannel);

  TowerdataID=jfexTowerRoI->jFEXdataID();
  fill(m_packageName,TowerdataID);

  TowersimulationID=jfexTowerRoI->jFEXtowerID();
  fill(m_packageName,TowersimulationID);

  Towercalosource=jfexTowerRoI->Calosource();
  fill(m_packageName,Towercalosource);

  std::vector<uint16_t> Toweret_count=jfexTowerRoI->et_count();
  Toweretcount_barrel=Toweret_count.at(0);
  Toweretcount_tile=Toweret_count.at(0);
  Toweretcount_emec=Toweret_count.at(0);
  Toweretcount_hec=Toweret_count.at(0);
  Toweretcount_fcal1=Toweret_count.at(0);
  Toweretcount_fcal2=Toweret_count.at(0);
  Toweretcount_fcal3=Toweret_count.at(0);

  if(Towercalosource==0){
  fill(m_packageName,Toweretcount_barrel);
  }
  if(Towercalosource==1){
  fill(m_packageName,Toweretcount_tile);
  }
  if(Towercalosource==2){
  fill(m_packageName,Toweretcount_emec);
  }
  if(Towercalosource==3){
  fill(m_packageName,Toweretcount_hec);
  }
  if(Towercalosource==4){
  fill(m_packageName,Toweretcount_fcal1);
  }
  if(Towercalosource==5){
  fill(m_packageName,Toweretcount_fcal2);
  }
  if(Towercalosource==6){
  fill(m_packageName,Toweretcount_fcal3);
  }

  std::vector<char> Tower_saturationflag=jfexTowerRoI->isjTowerSat();
  Towersaturationflag=Tower_saturationflag.at(0);
  fill(m_packageName,Towersaturationflag);

  }
  return StatusCode::SUCCESS;

}


