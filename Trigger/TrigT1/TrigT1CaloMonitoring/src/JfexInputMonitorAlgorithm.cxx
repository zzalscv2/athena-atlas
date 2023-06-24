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
  ATH_MSG_DEBUG("m_jFexDataTowerKey: "<< m_jFexDataTowerKey);
  ATH_MSG_DEBUG("m_jFexEmulatedTowerKey: "<< m_jFexEmulatedTowerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_jFexDataTowerKey.initialize() );
  ATH_CHECK( m_jFexEmulatedTowerKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode JfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("JfexInputMonitorAlgorithm::fillHistograms");

    // Access jFex tower container
    SG::ReadHandle<xAOD::jFexTowerContainer> jFexTowerContainer{m_jFexDataTowerKey, ctx};
    if(!jFexTowerContainer.isValid()) {
        ATH_MSG_ERROR("No jFex Tower container valid in storegate with key: "<< m_jFexDataTowerKey);
        return StatusCode::FAILURE;
    }
    
    SG::ReadHandle<xAOD::jFexTowerContainer> jFexEmulatedTowerContainer{m_jFexEmulatedTowerKey, ctx};
    if(!jFexEmulatedTowerContainer.isValid()) {
        ATH_MSG_ERROR("No jFex Tower container valid in storegate with key: "<< m_jFexEmulatedTowerKey);
        return StatusCode::FAILURE;
    }
    
    //Run the monitoring only when the input data is filled (it is pre-scaled), otherwise skip
    if(jFexTowerContainer->empty()){
        ATH_MSG_DEBUG("number of jfex towers = "<< jFexTowerContainer->size());
        return StatusCode::SUCCESS;
    }
    
    
    // monitored variables for histogramscd 
    auto nJfexTowers = Monitored::Scalar<int>("NJfexTowers",0.0);
    auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
    auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
    auto DataEt = Monitored::Scalar<float>("DataEt",0.0);
    auto SCellSum = Monitored::Scalar<float>("SCellSum",0.0);
    auto EmuSum = Monitored::Scalar<float>("EmuSum",0.0);
    auto region = Monitored::Scalar<int>("region",0.0);
    auto type = Monitored::Scalar<int>("type",0.0);
    auto frac_SCellSum = Monitored::Scalar<float>("frac_SCellSum",0.0);
    auto ToweretaInvalid = Monitored::Scalar<float>("TowerEtaInvalid",-99.0);
    auto TowerPhiInvalid = Monitored::Scalar<float>("TowerPhiInvalid",-99.0);
    auto Towerglobaleta = Monitored::Scalar<int>("TowerGlobalEta",0);
    auto Towerglobalphi = Monitored::Scalar<uint32_t>("TowerGlobalPhi",0);
    auto Towermodule = Monitored::Scalar<uint8_t>("TowerModule",0);
    auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0);
    auto Towerchannel = Monitored::Scalar<uint8_t>("TowerChannel",0);
    auto TowerdataID = Monitored::Scalar<uint8_t>("TowerDataID",0);
    auto TowersimulationID = Monitored::Scalar<uint32_t>("TowerSimulationID",0.0);
    auto Towercalosource = Monitored::Scalar<uint8_t>("TowerCalosource",0);

    auto Toweretcount_barrel = Monitored::Scalar<uint16_t>("TowerEtcount_barrel",0);
    auto Toweretcount_tile = Monitored::Scalar<uint16_t>("TowerEtcount_tile",0);
    auto Toweretcount_emec = Monitored::Scalar<uint16_t>("TowerEtcount_emec",0);
    auto Toweretcount_hec = Monitored::Scalar<uint16_t>("TowerEtcount_hec",0);
    auto Toweretcount_fcal1 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal1",0);
    auto Toweretcount_fcal2 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal2",0);
    auto Toweretcount_fcal3 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal3",0);

    auto Towersaturationflag = Monitored::Scalar<char>("TowerSaturationflag",0);

    unsigned int njfexTowers=jFexTowerContainer->size();
    nJfexTowers=njfexTowers;
    fill(m_packageName,nJfexTowers);
    
    std::unordered_map< int, const xAOD::jFexTower* >dataTowers_map;

    for(const xAOD::jFexTower* dataTower : *jFexTowerContainer) {
        
        //Adding 1e-5 for plotting style 
        Towereta=dataTower->eta()+1e-5;
        Towerphi=dataTower->phi();
        
        if(dataTower->isCore()){
            const int TTID = dataTower->jFEXtowerID();
            const int source = dataTower->Calosource();
            int code = codedVal(TTID, source);
            
            
            auto it_TTower2SCells = dataTowers_map.find( code );
            if(it_TTower2SCells == dataTowers_map.end()){
                dataTowers_map[ code ] = dataTower;
            }
            else{
                ATH_MSG_WARNING("DataTower: "<<TTID<< " with Calosource: "<<source<< "is repeated. It shouldn't! ");
            }
            
            
            DataEt = dataTower->jtowerEtMeV();
            SCellSum = dataTower->SCellEtMeV();
            EmuSum = dataTower->SCellEtMeV();
            
            fill(m_packageName+"_decorated_all",DataEt,SCellSum);
            
            //Looking at decorated variables, Comming soon
            if( (dataTower->et_count()).at(0) != dataTower->emulated_jtowerEt() ){
                
                if( (dataTower->et_count()).at(0) != m_InvalidCode ){
                    frac_SCellSum = dataTower->jtowerEtMeV() != 0 ? (dataTower->SCellEtMeV() - dataTower->jtowerEtMeV())/dataTower->jtowerEtMeV() : 0;
                    fill(m_packageName+"_decorated",Towereta,Towerphi,DataEt,SCellSum,frac_SCellSum);
                }
                
                region = source;
                type = (dataTower->et_count()).at(0) == m_InvalidCode ? 0 : 1;
                
                fill(m_packageName,region,type);
                    
            }
            
            
            if(source == 1){
               DataEt /= 1e3; 
               EmuSum = dataTower->TileEtMeV()/1e3;
            }
            fill(m_packageName+"_details_"+std::to_string(source),DataEt,EmuSum);
            
            DataEt = (dataTower->et_count()).at(0);
            EmuSum = dataTower->emulated_jtowerEt();
            fill(m_packageName+"_detailsEncoded_"+std::to_string(source),DataEt,EmuSum);
            
            
        }
        
        fill(m_packageName,Towereta,Towerphi);

        Towerglobaleta=dataTower->globalEta();
        Towerglobalphi=dataTower->globalPhi();
        fill(m_packageName,Towerglobaleta,Towerglobalphi);

        Towermodule=dataTower->module();
        fill(m_packageName,Towermodule);

        Towerfpga=dataTower->fpga();
        fill(m_packageName,Towerfpga);

        Towerchannel=dataTower->channel();
        fill(m_packageName,Towerchannel);

        TowerdataID=dataTower->jFEXdataID();
        fill(m_packageName,TowerdataID);

        TowersimulationID=dataTower->jFEXtowerID();
        fill(m_packageName,TowersimulationID);

        Towercalosource=dataTower->Calosource();
        fill(m_packageName,Towercalosource);

        std::vector<uint16_t> Toweret_count=dataTower->et_count();
        Toweretcount_barrel=Toweret_count.at(0);
        Toweretcount_tile=Toweret_count.at(0);
        Toweretcount_emec=Toweret_count.at(0);
        Toweretcount_hec=Toweret_count.at(0);
        Toweretcount_fcal1=Toweret_count.at(0);
        Toweretcount_fcal2=Toweret_count.at(0);
        Toweretcount_fcal3=Toweret_count.at(0);
        
        if(Toweret_count.at(0) == m_InvalidCode){
            ToweretaInvalid=dataTower->eta();
            TowerPhiInvalid=dataTower->phi();
            fill(m_packageName,ToweretaInvalid,TowerPhiInvalid);
        }

        if(Towercalosource==0) {
            fill(m_packageName,Toweretcount_barrel);
        }
        if(Towercalosource==1) {
            fill(m_packageName,Toweretcount_tile);
        }
        if(Towercalosource==2) {
            fill(m_packageName,Toweretcount_emec);
        }
        if(Towercalosource==3) {
            fill(m_packageName,Toweretcount_hec);
        }
        if(Towercalosource==4) {
            fill(m_packageName,Toweretcount_fcal1);
        }
        if(Towercalosource==5) {
            fill(m_packageName,Toweretcount_fcal2);
        }
        if(Towercalosource==6) {
            fill(m_packageName,Toweretcount_fcal3);
        }

        std::vector<char> Tower_saturationflag=dataTower->isjTowerSat();
        Towersaturationflag=Tower_saturationflag.at(0);
        fill(m_packageName,Towersaturationflag);
        
        

    }
    
    for(const xAOD::jFexTower* emulTower : *jFexEmulatedTowerContainer) {
        
        

        if(emulTower->isCore()){
            
            auto it_TTower2SCells = dataTowers_map.find( codedVal(emulTower->jFEXtowerID(), emulTower->Calosource() ) );
            
            if(it_TTower2SCells == dataTowers_map.end() ){
                ATH_MSG_WARNING("DataTower: "<<emulTower->jFEXtowerID()<< " with Calosource: "<<emulTower->Calosource()<< " does not exists. It should! Investigate ");
                continue;
            }
            
            const xAOD::jFexTower* dataTower = it_TTower2SCells->second;       
            
            if(emulTower->et_count().at(0) != dataTower->et_count().at(0) ){
                //Adding 1e-5 for plotting style 
                Towereta=emulTower->eta()+1e-5;
                Towerphi=emulTower->phi();            

                fill(m_packageName+"_emulated",Towereta,Towerphi);                
            }    
 
        }
        
    }
    

    return StatusCode::SUCCESS;

}

int JfexInputMonitorAlgorithm::codedVal(const int ID, const int source) const{
    return (ID<<4) + source;
}


