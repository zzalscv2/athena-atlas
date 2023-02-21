/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GfexMonitorAlgorithm.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloIdentifier/CaloGain.h"
#include "Identifier/Identifier.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include <istream>


GfexMonitorAlgorithm::GfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode GfexMonitorAlgorithm::initialize() {
  ATH_MSG_DEBUG("GfexMonitorAlgorithm::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  
  ATH_MSG_DEBUG("m_gFexLRJetContainerKey"<< m_gFexLRJetContainerKey);
  ATH_MSG_DEBUG("m_gFexSRJetContainerKey"<< m_gFexSRJetContainerKey);
  ATH_MSG_DEBUG("m_gFexRhoContainerKey"<< m_gFexRhoContainerKey);
  ATH_MSG_DEBUG("m_gScalarEJwojContainerKey"<< m_gScalarEJwojContainerKey);
  ATH_MSG_DEBUG("m_gMETComponentsJwojContainerKey"<< m_gMETComponentsJwojContainerKey);
  ATH_MSG_DEBUG("m_gMHTComponentsJwojContainerKey"<< m_gMHTComponentsJwojContainerKey);
  ATH_MSG_DEBUG("m_gMSTComponentsJwojContainerKey"<< m_gMSTComponentsJwojContainerKey);

  ATH_CHECK( m_gFexLRJetContainerKey.initialize() );  
  ATH_CHECK( m_gFexSRJetContainerKey.initialize() );  
  ATH_CHECK( m_gFexRhoContainerKey.initialize() );  

  ATH_CHECK( m_gScalarEJwojContainerKey.initialize() );  
  ATH_CHECK( m_gMETComponentsJwojContainerKey.initialize() );  
  ATH_CHECK( m_gMHTComponentsJwojContainerKey.initialize() );  
  ATH_CHECK( m_gMSTComponentsJwojContainerKey.initialize() );  

  ATH_CHECK( m_gMETComponentsNoiseCutContainerKey.initialize() );
  ATH_CHECK( m_gMETComponentsRmsContainerKey.initialize() );
  ATH_CHECK( m_gScalarENoiseCutContainerKey.initialize() );
  ATH_CHECK( m_gScalarERmsContainerKey.initialize() );

  return AthMonitorAlgorithm::initialize();
}

StatusCode GfexMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  ATH_MSG_DEBUG("GfexMonitorAlgorithm::fillHistograms");
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;

  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexLRJetContainer{m_gFexLRJetContainerKey, ctx};
  if(!gFexLRJetContainer.isValid()){
    ATH_MSG_ERROR("No gFex LR Jet container found in storegate  "<< m_gFexLRJetContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexSRJetContainer{m_gFexSRJetContainerKey, ctx};
  if(!gFexSRJetContainer.isValid()){
    ATH_MSG_ERROR("No gFex SR Jet container found in storegate  "<< m_gFexSRJetContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexRhoContainer{m_gFexRhoContainerKey, ctx};
  if(!gFexRhoContainer.isValid()){
    ATH_MSG_ERROR("No gFex rho container found in storegate  "<< m_gFexRhoContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gScalarEJwojContainer{m_gScalarEJwojContainerKey, ctx};
  if(!gScalarEJwojContainer.isValid()){
    ATH_MSG_ERROR("No scalar energy jets without jets container found in storegate  "<< m_gScalarEJwojContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gMETComponentsJwojContainer{m_gMETComponentsJwojContainerKey, ctx};
  if(!gMETComponentsJwojContainer.isValid()){
    ATH_MSG_ERROR("No MET components for jets without jets container found in storegate  "<< m_gMETComponentsJwojContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gMHTComponentsJwojContainer{m_gMHTComponentsJwojContainerKey, ctx};
  if(!gMHTComponentsJwojContainer.isValid()){
    ATH_MSG_ERROR("No MHT components for jets without jets container found in storegate  "<< m_gMHTComponentsJwojContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gMSTComponentsJwojContainer{m_gMSTComponentsJwojContainerKey, ctx};
  if(!gMSTComponentsJwojContainer.isValid()){
    ATH_MSG_ERROR("No MST components jets without jets container found in storegate  "<< m_gMSTComponentsJwojContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gMETComponentsNoiseCutContainer{m_gMETComponentsNoiseCutContainerKey, ctx};
  if(!gMETComponentsNoiseCutContainer.isValid()){
    ATH_MSG_ERROR("No MET components calculated with Noise Cut Algorithm in storegate  "<< m_gMETComponentsNoiseCutContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gMETComponentsRmsContainer{m_gMETComponentsRmsContainerKey, ctx};
  if(!gMETComponentsRmsContainer.isValid()){
    ATH_MSG_ERROR("No MET components calculated with Rms Algorithm in storegate  "<< m_gMETComponentsRmsContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gScalarENoiseCutContainer{m_gScalarENoiseCutContainerKey, ctx};
  if(!gScalarENoiseCutContainer.isValid()){
    ATH_MSG_ERROR("No scalar energy Noise Cut container found in storegate  "<< m_gScalarENoiseCutContainerKey);
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gScalarERmsContainer{m_gScalarERmsContainerKey, ctx};
  if(!gScalarENoiseCutContainer.isValid()){
    ATH_MSG_ERROR("No scalar energy Noise Cut container found in storegate  "<< m_gScalarENoiseCutContainerKey);
    return StatusCode::SUCCESS;
  }

  // variables for histograms
  auto gtype = Monitored::Scalar<int>("gFexType",0.0);
  auto gFexLRJeteT = Monitored::Scalar<float>("gFexLRJetTransverseEnergy",0.0);
  auto gFexLRJeteta = Monitored::Scalar<float>("gFexLRJetEta",0.0);
  auto gFexLRJetphi = Monitored::Scalar<float>("gFexLRJetPhi",0.0);
  auto gFexSRJeteT = Monitored::Scalar<float>("gFexSRJetTransverseEnergy",0.0);
  auto gFexSRJeteta = Monitored::Scalar<float>("gFexSRJetEta",0.0);
  auto gFexSRJetphi = Monitored::Scalar<float>("gFexSRJetPhi",0.0);
  auto gFexRhoeT = Monitored::Scalar<float>("gFexRhoTransverseEnergy",0.0);
  auto gFexMet = Monitored::Scalar<float>("gFexMET",0.0);
  auto gFexSumEt = Monitored::Scalar<float>("gFexSumET",0.0);
  auto METx = Monitored::Scalar<float>("gFexMETx",0.0);
  auto METy = Monitored::Scalar<float>("gFexMETy",0.0);
  auto MHTx = Monitored::Scalar<float>("gFexMHTx",0.0);
  auto MHTy = Monitored::Scalar<float>("gFexMHTy",0.0);
  auto MSTx = Monitored::Scalar<float>("gFexMSTx",0.0);
  auto MSTy = Monitored::Scalar<float>("gFexMSTy",0.0);
  auto METx_NoiseCut = Monitored::Scalar<float>("gFexMETx_NoiseCut",0.0);
  auto METy_NoiseCut = Monitored::Scalar<float>("gFexMETy_NoiseCut",0.0);
  auto METx_Rms = Monitored::Scalar<float>("gFexMETx_Rms",0.0);
  auto METy_Rms = Monitored::Scalar<float>("gFexMETy_Rms",0.0);
  auto gFexMet_NoiseCut = Monitored::Scalar<float>("gFexMET_NoiseCut",0.0);
  auto gFexSumEt_NoiseCut = Monitored::Scalar<float>("gFexSumET_NoiseCut",0.0);
  auto gFexMet_Rms = Monitored::Scalar<float>("gFexMET_Rms",0.0);
  auto gFexSumEt_Rms = Monitored::Scalar<float>("gFexSumET_Rms",0.0);

  /* Variables with pt cuts */
  // SR jets
  auto gFexSRJetPtCutPt0 = Monitored::Scalar<float>("gFexSRJetPtCutPt0",0.0);
  auto gFexSRJetPtCutPt10 = Monitored::Scalar<float>("gFexSRJetPtCutPt10",0.0);
  auto gFexSRJetPtCutPt50 = Monitored::Scalar<float>("gFexSRJetPtCutPt50",0.0);
  auto gFexSRJetPtCutPt100 = Monitored::Scalar<float>("gFexSRJetPtCutPt100",0.0);

  auto gFexSRJetEtaCutPt0 = Monitored::Scalar<float>("gFexSRJetEtaCutPt0",0.0);
  auto gFexSRJetEtaCutPt10 = Monitored::Scalar<float>("gFexSRJetEtaCutPt10",0.0);
  auto gFexSRJetEtaCutPt50 = Monitored::Scalar<float>("gFexSRJetEtaCutPt50",0.0);
  auto gFexSRJetEtaCutPt100 = Monitored::Scalar<float>("gFexSRJetEtaCutPt100",0.0);

  auto gFexSRJetPhiCutPt0 = Monitored::Scalar<float>("gFexSRJetPhiCutPt0",0.0);
  auto gFexSRJetPhiCutPt10 = Monitored::Scalar<float>("gFexSRJetPhiCutPt10",0.0);
  auto gFexSRJetPhiCutPt50 = Monitored::Scalar<float>("gFexSRJetPhiCutPt50",0.0);
  auto gFexSRJetPhiCutPt100 = Monitored::Scalar<float>("gFexSRJetPhiCutPt100",0.0);

  // LR jets
  auto gFexLRJetPtCutPt0 = Monitored::Scalar<float>("gFexLRJetPtCutPt0",0.0);
  auto gFexLRJetPtCutPt10 = Monitored::Scalar<float>("gFexLRJetPtCutPt10",0.0);
  auto gFexLRJetPtCutPt50 = Monitored::Scalar<float>("gFexLRJetPtCutPt50",0.0);
  auto gFexLRJetPtCutPt100 = Monitored::Scalar<float>("gFexLRJetPtCutPt100",0.0);

  auto gFexLRJetEtaCutPt0 = Monitored::Scalar<float>("gFexLRJetEtaCutPt0",0.0);
  auto gFexLRJetEtaCutPt10 = Monitored::Scalar<float>("gFexLRJetEtaCutPt10",0.0);
  auto gFexLRJetEtaCutPt50 = Monitored::Scalar<float>("gFexLRJetEtaCutPt50",0.0);
  auto gFexLRJetEtaCutPt100 = Monitored::Scalar<float>("gFexLRJetEtaCutPt100",0.0);

  auto gFexLRJetPhiCutPt0 = Monitored::Scalar<float>("gFexLRJetPhiCutPt0",0.0);
  auto gFexLRJetPhiCutPt10 = Monitored::Scalar<float>("gFexLRJetPhiCutPt10",0.0);
  auto gFexLRJetPhiCutPt50 = Monitored::Scalar<float>("gFexLRJetPhiCutPt50",0.0);
  auto gFexLRJetPhiCutPt100 = Monitored::Scalar<float>("gFexLRJetPhiCutPt100",0.0);

  /* Variables with FPGA region cuts */
  // SR jets
  auto gFexSRJetEtaCutFPGAa = Monitored::Scalar<float>("gFexSRJetEtaCutFPGAa",0.0);
  auto gFexSRJetEtaCutFPGAb = Monitored::Scalar<float>("gFexSRJetEtaCutFPGAb",0.0);
  auto gFexSRJetEtaCutFPGAc = Monitored::Scalar<float>("gFexSRJetEtaCutFPGAc",0.0);

  auto gFexSRJetPhiCutFPGAa = Monitored::Scalar<float>("gFexSRJetPhiCutFPGAa",0.0);
  auto gFexSRJetPhiCutFPGAb = Monitored::Scalar<float>("gFexSRJetPhiCutFPGAb",0.0);
  auto gFexSRJetPhiCutFPGAc = Monitored::Scalar<float>("gFexSRJetPhiCutFPGAc",0.0);

  auto gFexSRJetEtCutFPGAa = Monitored::Scalar<float>("gFexSRJetEtCutFPGAa",0.0);
  auto gFexSRJetEtCutFPGAb = Monitored::Scalar<float>("gFexSRJetEtCutFPGAb",0.0);
  auto gFexSRJetEtCutFPGAc = Monitored::Scalar<float>("gFexSRJetEtCutFPGAc",0.0);
  // Same as above, but requiring non-zero objects by enforcing pt>0
  auto gFexSRJetEtaCutFPGAaCutPt0 = Monitored::Scalar<float>("gFexSRJetEtaCutFPGAaCutPt0",0.0);
  auto gFexSRJetEtaCutFPGAbCutPt0 = Monitored::Scalar<float>("gFexSRJetEtaCutFPGAbCutPt0",0.0);
  auto gFexSRJetEtaCutFPGAcCutPt0 = Monitored::Scalar<float>("gFexSRJetEtaCutFPGAcCutPt0",0.0);

  auto gFexSRJetPhiCutFPGAaCutPt0 = Monitored::Scalar<float>("gFexSRJetPhiCutFPGAaCutPt0",0.0);
  auto gFexSRJetPhiCutFPGAbCutPt0 = Monitored::Scalar<float>("gFexSRJetPhiCutFPGAbCutPt0",0.0);
  auto gFexSRJetPhiCutFPGAcCutPt0 = Monitored::Scalar<float>("gFexSRJetPhiCutFPGAcCutPt0",0.0);

  auto gFexSRJetEtCutFPGAaCutPt0 = Monitored::Scalar<float>("gFexSRJetEtCutFPGAaCutPt0",0.0);
  auto gFexSRJetEtCutFPGAbCutPt0 = Monitored::Scalar<float>("gFexSRJetEtCutFPGAbCutPt0",0.0);
  auto gFexSRJetEtCutFPGAcCutPt0 = Monitored::Scalar<float>("gFexSRJetEtCutFPGAcCutPt0",0.0);

  // LR jets
  auto gFexLRJetEtaCutFPGAa = Monitored::Scalar<float>("gFexLRJetEtaCutFPGAa",0.0);
  auto gFexLRJetEtaCutFPGAb = Monitored::Scalar<float>("gFexLRJetEtaCutFPGAb",0.0);
  auto gFexLRJetEtaCutFPGAc = Monitored::Scalar<float>("gFexLRJetEtaCutFPGAc",0.0);

  auto gFexLRJetPhiCutFPGAa = Monitored::Scalar<float>("gFexLRJetPhiCutFPGAa",0.0);
  auto gFexLRJetPhiCutFPGAb = Monitored::Scalar<float>("gFexLRJetPhiCutFPGAb",0.0);
  auto gFexLRJetPhiCutFPGAc = Monitored::Scalar<float>("gFexLRJetPhiCutFPGAc",0.0);

  auto gFexLRJetEtCutFPGAa = Monitored::Scalar<float>("gFexLRJetEtCutFPGAa",0.0);
  auto gFexLRJetEtCutFPGAb = Monitored::Scalar<float>("gFexLRJetEtCutFPGAb",0.0);
  auto gFexLRJetEtCutFPGAc = Monitored::Scalar<float>("gFexLRJetEtCutFPGAc",0.0);
  // Same as above, but requiring non-zero objects by enforcing pt>0
  auto gFexLRJetEtaCutFPGAaCutPt0 = Monitored::Scalar<float>("gFexLRJetEtaCutFPGAaCutPt0",0.0);
  auto gFexLRJetEtaCutFPGAbCutPt0 = Monitored::Scalar<float>("gFexLRJetEtaCutFPGAbCutPt0",0.0);
  auto gFexLRJetEtaCutFPGAcCutPt0 = Monitored::Scalar<float>("gFexLRJetEtaCutFPGAcCutPt0",0.0);

  auto gFexLRJetPhiCutFPGAaCutPt0 = Monitored::Scalar<float>("gFexLRJetPhiCutFPGAaCutPt0",0.0);
  auto gFexLRJetPhiCutFPGAbCutPt0 = Monitored::Scalar<float>("gFexLRJetPhiCutFPGAbCutPt0",0.0);
  auto gFexLRJetPhiCutFPGAcCutPt0 = Monitored::Scalar<float>("gFexLRJetPhiCutFPGAcCutPt0",0.0);

  auto gFexLRJetEtCutFPGAaCutPt0 = Monitored::Scalar<float>("gFexLRJetEtCutFPGAaCutPt0",0.0);
  auto gFexLRJetEtCutFPGAbCutPt0 = Monitored::Scalar<float>("gFexLRJetEtCutFPGAbCutPt0",0.0);
  auto gFexLRJetEtCutFPGAcCutPt0 = Monitored::Scalar<float>("gFexLRJetEtCutFPGAcCutPt0",0.0);

  for(const xAOD::gFexJetRoI* gFexLRJetRoI : *gFexLRJetContainer){
    gFexLRJeteT=gFexLRJetRoI->gFexTobEt();
    gFexLRJeteta=gFexLRJetRoI->eta();
    gFexLRJetphi=gFexLRJetRoI->phi();
    gtype=gFexLRJetRoI->gFexType();
    fill(m_packageName,gFexLRJeteta,gFexLRJetphi,gFexLRJeteT,gtype);

    // pT cuts
    if(gFexLRJeteT > 0){
      gFexLRJetPtCutPt0 = gFexLRJetRoI->gFexTobEt();
      gFexLRJetEtaCutPt0 = gFexLRJetRoI->eta();
      gFexLRJetPhiCutPt0 = gFexLRJetRoI->phi();
      fill(m_packageName,gFexLRJetPtCutPt0,gFexLRJetEtaCutPt0,gFexLRJetPhiCutPt0);
    }
    if(gFexLRJeteT > 10){
      gFexLRJetPtCutPt10 = gFexLRJetRoI->gFexTobEt();
      gFexLRJetEtaCutPt10 = gFexLRJetRoI->eta();
      gFexLRJetPhiCutPt10 = gFexLRJetRoI->phi();
      fill(m_packageName,gFexLRJetPtCutPt10,gFexLRJetEtaCutPt10,gFexLRJetPhiCutPt10);
    }
    if(gFexLRJeteT > 50){
      gFexLRJetPtCutPt50 = gFexLRJetRoI->gFexTobEt();
      gFexLRJetEtaCutPt50 = gFexLRJetRoI->eta();
      gFexLRJetPhiCutPt50 = gFexLRJetRoI->phi();
      fill(m_packageName,gFexLRJetPtCutPt50,gFexLRJetEtaCutPt50,gFexLRJetPhiCutPt50);
    }
    if(gFexLRJeteT > 100){
      gFexLRJetPtCutPt100 = gFexLRJetRoI->gFexTobEt();
      gFexLRJetEtaCutPt100 = gFexLRJetRoI->eta();
      gFexLRJetPhiCutPt100 = gFexLRJetRoI->phi();
      fill(m_packageName,gFexLRJetPtCutPt100,gFexLRJetEtaCutPt100,gFexLRJetPhiCutPt100);
    }

    // FPGA region cuts
    if(gFexLRJeteta > 0 && gFexLRJeteta < 2.5){ // pFPGA A
      gFexLRJetEtaCutFPGAa = gFexLRJetRoI->eta();
      gFexLRJetPhiCutFPGAa = gFexLRJetRoI->phi();
      gFexLRJetEtCutFPGAa = gFexLRJetRoI->gFexTobEt();
      fill(m_packageName,gFexLRJetEtaCutFPGAa,gFexLRJetPhiCutFPGAa,gFexLRJetEtCutFPGAa);
      if(gFexLRJeteT > 0){
        fill(m_packageName,gFexLRJetEtaCutFPGAaCutPt0);
      }
    }
    if(gFexLRJeteta < 0 && gFexLRJeteta > -2.5){ // pFPGA B
      gFexLRJetEtaCutFPGAb = gFexLRJetRoI->eta();
      gFexLRJetPhiCutFPGAb = gFexLRJetRoI->phi();
      gFexLRJetEtCutFPGAb = gFexLRJetRoI->gFexTobEt();
      fill(m_packageName,gFexLRJetEtaCutFPGAb,gFexLRJetPhiCutFPGAb,gFexLRJetEtCutFPGAb);
      if(gFexLRJeteT > 0){
        fill(m_packageName,gFexLRJetEtaCutFPGAbCutPt0);
      }
    }
    if(std::abs(gFexLRJeteta) > 2.5 && std::abs(gFexLRJeteta) < 4.9){ // pFPGA C
      gFexLRJetEtaCutFPGAc = gFexLRJetRoI->eta();
      gFexLRJetPhiCutFPGAc = gFexLRJetRoI->phi();
      gFexLRJetEtCutFPGAc = gFexLRJetRoI->gFexTobEt();
      fill(m_packageName,gFexLRJetEtaCutFPGAc,gFexLRJetPhiCutFPGAc,gFexLRJetEtCutFPGAc);
      if(gFexLRJeteT > 0){
        fill(m_packageName,gFexLRJetEtaCutFPGAcCutPt0);
      }
    }

  }

  for(const xAOD::gFexJetRoI* gFexSRJetRoI : *gFexSRJetContainer){
    gFexSRJeteT=gFexSRJetRoI->gFexTobEt();
    gFexSRJeteta=gFexSRJetRoI->eta();
    gFexSRJetphi=gFexSRJetRoI->phi();
    gtype=gFexSRJetRoI->gFexType();
    fill(m_packageName,gFexSRJeteta,gFexSRJetphi,gFexSRJeteT,gtype);

    // pT cuts
    if(gFexSRJeteT > 0){
      gFexSRJetPtCutPt0 = gFexSRJetRoI->gFexTobEt();
      gFexSRJetEtaCutPt0 = gFexSRJetRoI->eta();
      gFexSRJetPhiCutPt0 = gFexSRJetRoI->phi();
      fill(m_packageName,gFexSRJetPtCutPt0,gFexSRJetEtaCutPt0,gFexSRJetPhiCutPt0);
    }
    if(gFexSRJeteT > 10){
      gFexSRJetPtCutPt10 = gFexSRJetRoI->gFexTobEt();
      gFexSRJetEtaCutPt10 = gFexSRJetRoI->eta();
      gFexSRJetPhiCutPt10 = gFexSRJetRoI->phi();
      fill(m_packageName,gFexSRJetPtCutPt10,gFexSRJetEtaCutPt10,gFexSRJetPhiCutPt10);
    }
    if(gFexSRJeteT > 50){
      gFexSRJetPtCutPt50 = gFexSRJetRoI->gFexTobEt();
      gFexSRJetEtaCutPt50 = gFexSRJetRoI->eta();
      gFexSRJetPhiCutPt50 = gFexSRJetRoI->phi();
      fill(m_packageName,gFexSRJetPtCutPt50,gFexSRJetEtaCutPt50,gFexSRJetPhiCutPt50);
    }
    if(gFexSRJeteT > 100){
      gFexSRJetPtCutPt100 = gFexSRJetRoI->gFexTobEt();
      gFexSRJetEtaCutPt100 = gFexSRJetRoI->eta();
      gFexSRJetPhiCutPt100 = gFexSRJetRoI->phi();
      fill(m_packageName,gFexSRJetPtCutPt100,gFexSRJetEtaCutPt100,gFexSRJetPhiCutPt100);
    }

    // FPGA region cuts
    if(gFexSRJeteta > 0 && gFexSRJeteta < 2.5){ // pFPGA A
      gFexSRJetEtaCutFPGAa = gFexSRJetRoI->eta();
      gFexSRJetPhiCutFPGAa = gFexSRJetRoI->phi();
      gFexSRJetEtCutFPGAa = gFexSRJetRoI->gFexTobEt();
      fill(m_packageName,gFexSRJetEtaCutFPGAa,gFexSRJetPhiCutFPGAa,gFexSRJetEtCutFPGAa);
      if(gFexSRJeteT > 0){
        gFexSRJetEtaCutFPGAaCutPt0 = gFexSRJetRoI->eta();
        gFexSRJetPhiCutFPGAaCutPt0 = gFexSRJetRoI->phi();
        gFexSRJetEtCutFPGAaCutPt0 = gFexSRJetRoI->gFexTobEt();
        fill(m_packageName,gFexSRJetEtaCutFPGAaCutPt0,gFexSRJetPhiCutFPGAaCutPt0,gFexSRJetEtCutFPGAaCutPt0);
      }
    }
    if(gFexSRJeteta < 0 && gFexSRJeteta > -2.5){ // pFPGA B
      gFexSRJetEtaCutFPGAb = gFexSRJetRoI->eta();
      gFexSRJetPhiCutFPGAb = gFexSRJetRoI->phi();
      gFexSRJetEtCutFPGAb = gFexSRJetRoI->gFexTobEt();
      fill(m_packageName,gFexSRJetEtaCutFPGAb,gFexSRJetPhiCutFPGAb,gFexSRJetEtCutFPGAb);
      if(gFexSRJeteT > 0){
        gFexSRJetEtaCutFPGAbCutPt0 = gFexSRJetRoI->eta();
        gFexSRJetPhiCutFPGAbCutPt0 = gFexSRJetRoI->phi();
        gFexSRJetEtCutFPGAbCutPt0 = gFexSRJetRoI->gFexTobEt();
        fill(m_packageName,gFexSRJetEtaCutFPGAbCutPt0,gFexSRJetPhiCutFPGAbCutPt0,gFexSRJetEtCutFPGAbCutPt0);
      }
    }
    if(std::abs(gFexSRJeteta) > 2.5 && std::abs(gFexSRJeteta) < 4.9){ // pFPGA C
      gFexSRJetEtaCutFPGAc = gFexSRJetRoI->eta();
      gFexSRJetPhiCutFPGAc = gFexSRJetRoI->phi();
      gFexSRJetEtCutFPGAc = gFexSRJetRoI->gFexTobEt();
      fill(m_packageName,gFexSRJetEtaCutFPGAc,gFexSRJetPhiCutFPGAc,gFexSRJetEtCutFPGAc);
      if(gFexSRJeteT > 0){
        fill(m_packageName,gFexSRJetEtaCutFPGAcCutPt0);
        gFexSRJetEtaCutFPGAcCutPt0 = gFexSRJetRoI->eta();
        gFexSRJetPhiCutFPGAcCutPt0 = gFexSRJetRoI->phi();
        gFexSRJetEtCutFPGAcCutPt0 = gFexSRJetRoI->gFexTobEt();
        fill(m_packageName,gFexSRJetEtaCutFPGAcCutPt0,gFexSRJetPhiCutFPGAcCutPt0,gFexSRJetEtCutFPGAcCutPt0);
      }
    }
  }

  for(const xAOD::gFexJetRoI* gFexRhoRoI : *gFexRhoContainer){
    gFexRhoeT=gFexRhoRoI->gFexTobEt();
    gtype=gFexRhoRoI->gFexType();
    fill(m_packageName,gFexRhoeT, gtype);
  }

  for(const xAOD::gFexGlobalRoI* gScalarEJwoj : *gScalarEJwojContainer){
    gFexSumEt=gScalarEJwoj->METquantityTwo();
    gFexMet=gScalarEJwoj->METquantityOne();
    fill(m_packageName,gFexMet,gFexSumEt);
  }

  for (const xAOD::gFexGlobalRoI* gMETComponentsJwoj : *gMETComponentsJwojContainer) {
    METx=gMETComponentsJwoj->METquantityOne();
    METy=gMETComponentsJwoj->METquantityTwo();
    fill(m_packageName,METx);
    fill(m_packageName,METy);
  }

  for (const xAOD::gFexGlobalRoI* gMHTComponentsJwoj : *gMHTComponentsJwojContainer) {
    MHTx=gMHTComponentsJwoj->METquantityOne();
    MHTy=gMHTComponentsJwoj->METquantityTwo();
    fill(m_packageName,MHTx,MHTy);
  }

  for (const xAOD::gFexGlobalRoI* gMSTComponentsJwoj : *gMSTComponentsJwojContainer) {
    MSTx=gMSTComponentsJwoj->METquantityOne();
    MSTy=gMSTComponentsJwoj->METquantityTwo();
    fill(m_packageName,MSTx,MSTy);
  }

  for (const xAOD::gFexGlobalRoI* gMETComponentsNoiseCut : *gMETComponentsNoiseCutContainer) {
    METx_NoiseCut=gMETComponentsNoiseCut->METquantityOne();
    METy_NoiseCut=gMETComponentsNoiseCut->METquantityTwo();
    fill(m_packageName,METx_NoiseCut,METy_NoiseCut);
  }

  for (const xAOD::gFexGlobalRoI* gMETComponentsRms : *gMETComponentsRmsContainer) {
    METx_Rms=gMETComponentsRms->METquantityOne();
    METy_Rms=gMETComponentsRms->METquantityTwo();
    fill(m_packageName,METx_Rms,METy_Rms);
  }

  for (const xAOD::gFexGlobalRoI* gScalarENoiseCut : *gScalarENoiseCutContainer) {
    gFexMet_NoiseCut=gScalarENoiseCut->METquantityOne();
    gFexSumEt_NoiseCut=gScalarENoiseCut->METquantityTwo();
    fill(m_packageName,gFexMet_NoiseCut,gFexSumEt_NoiseCut);
  }

  for (const xAOD::gFexGlobalRoI* gScalarERms : *gScalarERmsContainer) {
    gFexMet_Rms=gScalarERms->METquantityOne();
    gFexSumEt_Rms=gScalarERms->METquantityTwo();
    fill(m_packageName,gFexMet_Rms,gFexSumEt_Rms);
  }

  fill(m_packageName,variables);
  variables.clear();
  return StatusCode::SUCCESS;
}

