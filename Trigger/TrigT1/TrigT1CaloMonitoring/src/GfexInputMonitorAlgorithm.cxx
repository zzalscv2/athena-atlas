/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GfexInputMonitorAlgorithm.h"

GfexInputMonitorAlgorithm::GfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode GfexInputMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("GfexInputMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_gFexTowerContainer"<< m_gFexTowerContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_gFexTowerContainerKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode GfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("GfexInputMonitorAlgorithm::fillHistograms");

  // Access gFex gTower container
  SG::ReadHandle<xAOD::gFexTowerContainer> gFexTowerContainer{m_gFexTowerContainerKey, ctx};
  if(!gFexTowerContainer.isValid()){
    ATH_MSG_ERROR("No gFex Tower container found in storegate  "<< m_gFexTowerContainerKey);
    return StatusCode::SUCCESS;
  }

  // monitored variables for histograms
  auto nGfexTowers = Monitored::Scalar<int>("NGfexTowers",0.0);
  auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
  auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
  auto Toweretaindex = Monitored::Scalar<uint8_t>("TowerEtaindex",0.0);
  auto Towerphiindex = Monitored::Scalar<uint8_t>("TowerPhiindex",0.0);
  auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0.0);
  auto Toweret = Monitored::Scalar<uint16_t>("TowerEt",0.0);
  auto Towersaturationflag = Monitored::Scalar<char>("TowerSaturationflag",0.0);

  ATH_MSG_DEBUG("number of gfex towers = "<< gFexTowerContainer->size());

  unsigned int ngfexTowers=gFexTowerContainer->size();
  nGfexTowers=ngfexTowers;
  fill(m_packageName,nGfexTowers);

  for(const xAOD::gFexTower* gfexTowerRoI : *gFexTowerContainer){

  Towereta=gfexTowerRoI->eta();
  Towerphi=gfexTowerRoI->phi();
  fill(m_packageName,Towereta,Towerphi);

  Toweretaindex=gfexTowerRoI->iEta();
  Towerphiindex=gfexTowerRoI->iPhi();
  fill(m_packageName,Toweretaindex,Towerphiindex);

  Towerfpga=gfexTowerRoI->fpga();
  fill(m_packageName,Towerfpga);

  Toweret=gfexTowerRoI->towerEt();
  fill(m_packageName,Toweret);

  Towersaturationflag=gfexTowerRoI->isSaturated();
  fill(m_packageName,Towersaturationflag);

  }

  return StatusCode::SUCCESS;
}


