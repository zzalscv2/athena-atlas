/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "EfexSimMonitorAlgorithm.h"
#include "eFEXTOBSimDataCompare.h"
#include <set>

EfexSimMonitorAlgorithm::EfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexSimMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexSimMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_eFexEmContainer"<< m_eFexEmContainerKey); 
  ATH_MSG_DEBUG("m_eFexEmSimContainer"<< m_eFexEmSimContainerKey); 
  ATH_MSG_DEBUG("m_eFexTauContainer"<< m_eFexTauContainerKey);
  ATH_MSG_DEBUG("m_eFexTauSimContainer"<< m_eFexTauSimContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexEmContainerKey.initialize() );
  ATH_CHECK( m_eFexEmSimContainerKey.initialize() );
  ATH_CHECK( m_eFexTauContainerKey.initialize() );
  ATH_CHECK( m_eFexTauSimContainerKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("EfexSimMonitorAlgorithm::fillHistograms");

  // Access eFex EM container
  SG::ReadHandle<xAOD::eFexEMRoIContainer> eFexEmContainer{m_eFexEmContainerKey, ctx};
  if(!eFexEmContainer.isValid()){
    ATH_MSG_ERROR("No eFex EM container found in storegate  "<< m_eFexEmContainerKey); 
    return StatusCode::SUCCESS;
  }
  unsigned int nEmTobs=eFexEmContainer->size();

  unsigned int nEmSimTobs=0;
  SG::ReadHandle<xAOD::eFexEMRoIContainer> eFexEmSimContainer{m_eFexEmSimContainerKey, ctx};
  if(!eFexEmSimContainer.isValid()){
    ATH_MSG_WARNING("No eFex EM simulated container found in storegate  "<< m_eFexEmSimContainerKey); 
  } else {
    nEmSimTobs=eFexEmSimContainer->size();
  }
  ATH_MSG_DEBUG("EM TOB: ndata: "<<nEmTobs<<" nsim: "<<nEmSimTobs);

  // Calculate where simulation agrees with data (EM)
  std::set<uint32_t> simEqDataEmWord0s;
  LVL1::compareTOBs(eFexEmContainer,eFexEmSimContainer,simEqDataEmWord0s);
  unsigned int nEmSimEqDataTobs=simEqDataEmWord0s.size();
  ATH_MSG_DEBUG("EM TOB sim equal data: "<<nEmSimEqDataTobs);

  // Fill EM data-simulation histograms
  const xAOD::eFexEMRoIContainer* emDataContPtr = eFexEmContainer.cptr();
  // same number of data and EM TOBs
  if (nEmTobs==nEmSimTobs) {     
    if (nEmSimEqDataTobs==nEmTobs) {     
      // simulation matches data fully
      ATH_CHECK(fillEmErrorHistos("simEqData",emDataContPtr,simEqDataEmWord0s));
      ATH_MSG_DEBUG("EM TOB expect simEqData: "<<nEmSimEqDataTobs);
    } else {
      ATH_CHECK(fillEmErrorHistos("simNeData",emDataContPtr,simEqDataEmWord0s));
      int nExpect=(nEmTobs-nEmSimEqDataTobs);
      ATH_MSG_DEBUG("EM TOB expect simNeData: "<<nExpect);
    }
  } else if (nEmTobs>nEmSimTobs) {
      ATH_CHECK(fillEmErrorHistos("dataNoSim",emDataContPtr,simEqDataEmWord0s));
      int nExpect = (nEmTobs-nEmSimTobs)+ (nEmSimTobs-nEmSimEqDataTobs);
      ATH_MSG_DEBUG("EM TOB expect dataNoSim: "<<nExpect);
  } else if (nEmTobs<nEmSimTobs) {
    const xAOD::eFexEMRoIContainer* emSimContPtr = eFexEmSimContainer.cptr();
    ATH_CHECK(fillEmErrorHistos("simNoData",emSimContPtr,simEqDataEmWord0s));
    int nExpect=(nEmSimTobs-nEmTobs)+(nEmSimTobs-nEmSimEqDataTobs);    
    ATH_MSG_DEBUG("EM TOB expect simNoData: "<<nExpect);
  }

  // Access eFex Tau container
  SG::ReadHandle<xAOD::eFexTauRoIContainer> eFexTauContainer{m_eFexTauContainerKey, ctx};
  if(!eFexTauContainer.isValid()){
    ATH_MSG_ERROR("No eFex Tau container found in storegate  "<< m_eFexTauContainerKey);
    return StatusCode::SUCCESS;
  }
  unsigned int nTauTobs=eFexTauContainer->size();

  unsigned int nTauSimTobs=0;
  SG::ReadHandle<xAOD::eFexTauRoIContainer> eFexTauSimContainer{m_eFexTauSimContainerKey, ctx};
  if(!eFexTauSimContainer.isValid()){
    ATH_MSG_WARNING("No eFex Tau simulated container found in storegate  "<< m_eFexTauSimContainerKey); 
  } else {
    nTauSimTobs=eFexTauSimContainer->size();
  }
  ATH_MSG_DEBUG("Tau TOB: data: "<<nTauTobs<<" sim: "<<nTauSimTobs);

  // Calculate where simulation agrees with data (Taus)
  std::set<uint32_t> simEqDataTauWord0s;
  LVL1::compareTOBs(eFexTauContainer,eFexTauSimContainer,simEqDataTauWord0s);
  unsigned int nTauSimEqDataTobs=simEqDataTauWord0s.size();
  ATH_MSG_DEBUG("Tau TOB sim equal data: "<<nTauSimEqDataTobs);

  // Fill tau data-simulation histograms
  const xAOD::eFexTauRoIContainer* tauDataContPtr = eFexTauContainer.cptr();
  // same number of data and tau TOBs
  if (nTauTobs==nTauSimTobs) {     
    if (nTauSimEqDataTobs==nTauTobs) {     
      // simulation matches data fully
      ATH_CHECK(fillTauErrorHistos("simEqData",tauDataContPtr,simEqDataTauWord0s));
      ATH_MSG_DEBUG("Tau TOB expect simEqData: "<<nTauSimEqDataTobs);
    } else {
      ATH_CHECK(fillTauErrorHistos("simNeData",tauDataContPtr,simEqDataTauWord0s));
      int nExpect=(nTauTobs-nTauSimEqDataTobs);
      ATH_MSG_DEBUG("Tau TOB expect simNeData: "<<nExpect);
    }
  } else if (nTauTobs>nTauSimTobs) {
      ATH_CHECK(fillTauErrorHistos("dataNoSim",tauDataContPtr,simEqDataTauWord0s));
      int nExpect = (nTauTobs-nTauSimTobs)+ (nTauSimTobs-nTauSimEqDataTobs);
      ATH_MSG_DEBUG("Tau TOB expect dataNoSim: "<<nExpect);
  } else if (nTauTobs<nTauSimTobs) {
    const xAOD::eFexTauRoIContainer* tauSimContPtr = eFexTauSimContainer.cptr();
    ATH_CHECK(fillTauErrorHistos("simNoData",tauSimContPtr,simEqDataTauWord0s));
    int nExpect=(nTauSimTobs-nTauTobs)+(nTauSimTobs-nTauSimEqDataTobs);    
    ATH_MSG_DEBUG("Tau TOB expect simNoData: "<<nExpect);
  }

  return StatusCode::SUCCESS;
}

StatusCode EfexSimMonitorAlgorithm::fillEmErrorHistos(const std::string& errName, const xAOD::eFexEMRoIContainer *emcont, const std::set<uint32_t> &simEqDataWord0s) const {

  std::string groupName = m_packageName+'_'+errName;

  ATH_MSG_DEBUG("filling histograms for EM "<< errName<<" "<<groupName);

  auto emTOBeta = Monitored::Scalar<float>("emTOBEta",0.0);
  auto emTOBphi = Monitored::Scalar<float>("emTOBPhi",0.0);
  auto emTOBeFEXNumber = Monitored::Scalar<float>("emTOBeFEXNumber",0.0);
  auto emTOBshelfNumber = Monitored::Scalar<float>("emTOBshelfNumber",0.0);


  for (const xAOD::eFexEMRoI* efexEmRoI : *emcont){
    // see if this tob was correctly simulated
    bool simEqData=false;
    uint32_t word0 = efexEmRoI->word0();
    for (auto & itr : simEqDataWord0s) {
      if(itr==word0) {
	simEqData=true;
      }
    } 

    // decide if histogram should be filled
    if ( errName == "simEqData" && !simEqData ) continue;
    if ( errName != "simEqData" && simEqData) continue;

    emTOBeta=efexEmRoI->eta();
    emTOBphi=efexEmRoI->phi();
    fill(groupName,emTOBeta,emTOBphi);
    
    uint8_t efexnumber=efexEmRoI->eFexNumber();
    emTOBeFEXNumber=int(efexnumber);
    uint8_t shelfnumber=efexEmRoI->shelfNumber();
    emTOBshelfNumber=shelfnumber;
    fill(groupName,emTOBshelfNumber,emTOBeFEXNumber);

  }

  return StatusCode::SUCCESS;
}

StatusCode EfexSimMonitorAlgorithm::fillTauErrorHistos(const std::string& errName, const xAOD::eFexTauRoIContainer *taucont, const std::set<uint32_t> &simEqDataWord0s) const {

  std::string groupName = m_packageName+'_'+errName;

  ATH_MSG_DEBUG("filling histograms for taus "<< errName<<" "<<groupName);

  auto tauTOBeta = Monitored::Scalar<float>("tauTOBEta",0.0);
  auto tauTOBphi = Monitored::Scalar<float>("tauTOBPhi",0.0);
  auto tauTOBeFEXNumber = Monitored::Scalar<float>("tauTOBeFEXNumber",0.0);
  auto tauTOBshelfNumber = Monitored::Scalar<float>("tauTOBshelfNumber",0.0);

  for (const xAOD::eFexTauRoI* efexTauRoI : *taucont){
    bool simEqData=false;
    uint32_t word0 = efexTauRoI->word0();
    for (auto & itr : simEqDataWord0s) {
      if(itr==word0) {
	simEqData=true;
      }
    } 

    // decide if histogram should be filled
    if ( errName == "simEqData" && !simEqData ) continue;
    if ( errName != "simEqData" && simEqData) continue;

    tauTOBeta=efexTauRoI->eta();
    tauTOBphi=efexTauRoI->phi();
    fill(groupName,tauTOBeta,tauTOBphi);

    uint8_t efexnumber=efexTauRoI->eFexNumber();
    tauTOBeFEXNumber=int(efexnumber);
    uint8_t shelfnumber=efexTauRoI->shelfNumber();
    tauTOBshelfNumber=shelfnumber;
    fill(groupName,tauTOBshelfNumber,tauTOBeFEXNumber);
  }

  return StatusCode::SUCCESS;
}
