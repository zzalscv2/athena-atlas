/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "EfexInputMonitorAlgorithm.h"

EfexInputMonitorAlgorithm::EfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexInputMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexInputMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_eFexTowerContainer"<< m_eFexTowerContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexTowerContainerKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("EfexInputMonitorAlgorithm::fillHistograms");

  //std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;

  // Access eFex eTower container
  SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowerContainer{m_eFexTowerContainerKey, ctx};
  if(!eFexTowerContainer.isValid()){
    ATH_MSG_ERROR("No eFex Tower container found in storegate  "<< m_eFexTowerContainerKey);
    return StatusCode::SUCCESS;
  }

  // monitored variables for histograms
  auto nEfexTowers = Monitored::Scalar<int>("NEfexTowers",0.0);
  auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
  auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
  auto Towermodule = Monitored::Scalar<uint8_t>("TowerModule",0.0);
  auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0.0);
  auto Toweretcount1 = Monitored::Scalar<uint16_t>("TowerEtcount1",0.0);
  auto Toweretcount2 = Monitored::Scalar<uint16_t>("TowerEtcount2",0.0);
  auto Toweretcount3 = Monitored::Scalar<uint16_t>("TowerEtcount3",0.0);
  auto Toweretcount4 = Monitored::Scalar<uint16_t>("TowerEtcount4",0.0);
  auto Toweretcount5 = Monitored::Scalar<uint16_t>("TowerEtcount5",0.0);
  auto Toweretcount6 = Monitored::Scalar<uint16_t>("TowerEtcount6",0.0);
  auto Toweretcount7 = Monitored::Scalar<uint16_t>("TowerEtcount7",0.0);
  auto Toweretcount8 = Monitored::Scalar<uint16_t>("TowerEtcount8",0.0);
  auto Toweretcount9 = Monitored::Scalar<uint16_t>("TowerEtcount9",0.0);
  auto Toweretcount10 = Monitored::Scalar<uint16_t>("TowerEtcount10",0.0);
  auto Toweretcount11 = Monitored::Scalar<uint16_t>("TowerEtcount11",0.0);
  auto Toweremstatus = Monitored::Scalar<uint32_t>("TowerEmstatus",0.0);
  auto Towerhadstatus = Monitored::Scalar<uint32_t>("TowerHadstatus",0.0);

  ATH_MSG_DEBUG("number of efex towers = "<< eFexTowerContainer->size());


  unsigned int nefexTowers=eFexTowerContainer->size();
  nEfexTowers=nefexTowers;
  fill(m_packageName,nEfexTowers);

  for(const xAOD::eFexTower* efexTowerRoI : *eFexTowerContainer){


    Towereta=efexTowerRoI->eta();

    //ATH_MSG_DEBUG("Tower eta = "<< Towereta);

    Towerphi=efexTowerRoI->phi();
    fill(m_packageName,Towereta,Towerphi);

    Towermodule=efexTowerRoI->module();
    fill(m_packageName,Towermodule);

    Towerfpga=efexTowerRoI->fpga();
    fill(m_packageName,Towerfpga);

    Toweremstatus=efexTowerRoI->em_status();
    fill(m_packageName,Toweremstatus);

    Towerhadstatus=efexTowerRoI->had_status();
    fill(m_packageName,Towerhadstatus);

    std::vector<uint16_t> Toweret_count=efexTowerRoI->et_count();
    Toweretcount1=Toweret_count.at(0);
    Toweretcount2=Toweret_count.at(1);
    Toweretcount3=Toweret_count.at(2);
    Toweretcount4=Toweret_count.at(3);
    Toweretcount5=Toweret_count.at(4);
    Toweretcount6=Toweret_count.at(5);
    Toweretcount7=Toweret_count.at(6);
    Toweretcount8=Toweret_count.at(7);
    Toweretcount9=Toweret_count.at(8);
    Toweretcount10=Toweret_count.at(9);
    Toweretcount11=Toweret_count.at(10);

    if(Toweremstatus == 0){
      fill(m_packageName,Toweretcount1);
      fill(m_packageName,Toweretcount2);
      fill(m_packageName,Toweretcount3);
      fill(m_packageName,Toweretcount4);
      fill(m_packageName,Toweretcount5);
      fill(m_packageName,Toweretcount6);
      fill(m_packageName,Toweretcount7);
      fill(m_packageName,Toweretcount8);
      fill(m_packageName,Toweretcount9);
      fill(m_packageName,Toweretcount10);
    }else{
      //ATH_MSG_DEBUG("EM status "<<Toweremstatus);
      std::string groupName = m_packageName+"_EmError";
      fill(groupName,Toweretcount1);
      fill(groupName,Toweretcount2);
      fill(groupName,Toweretcount3);
      fill(groupName,Toweretcount4);
      fill(groupName,Toweretcount5);
      fill(groupName,Toweretcount6);
      fill(groupName,Toweretcount7);
      fill(groupName,Toweretcount8);
      fill(groupName,Toweretcount9);
      fill(groupName,Toweretcount10);
    }

    if(Towerhadstatus == 0){
      fill(m_packageName,Toweretcount11);
    }else{
      std::string groupName1 = m_packageName+"_HadError";
      fill(groupName1,Toweretcount11);
    }

  }

  return StatusCode::SUCCESS;
}


