/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "EfexMonitorAlgorithm.h"


EfexMonitorAlgorithm::EfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("Low Pt Cut "<< m_lowPtCut);
  ATH_MSG_DEBUG("High Pt Cut "<< m_hiPtCut);
  ATH_MSG_DEBUG("m_eFexContainer"<< m_eFexContainerKey); 
  ATH_MSG_DEBUG("m_eFexTauContainer"<< m_eFexTauContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexContainerKey.initialize() );
  ATH_CHECK( m_eFexTauContainerKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("EfexMonitorAlgorithm::fillHistograms");

  // Begin the EM processing
  // Access eFex EM container
  SG::ReadHandle<xAOD::eFexEMRoIContainer> eFexContainer{m_eFexContainerKey, ctx};
  if(!eFexContainer.isValid()){
    ATH_MSG_ERROR("No eFex EM container found in storegate  "<< m_eFexContainerKey); 
    return StatusCode::SUCCESS;
  }
  // Fill histogram for the total number of ToBs (without any cuts applied)
  auto nEmTOBs_total = Monitored::Scalar<int>("nEMTOBs_nocut",0.0);
  nEmTOBs_total = eFexContainer->size();
  fill(m_packageName, nEmTOBs_total);
  // Fill histograms for the low/high cuts
  const xAOD::eFexEMRoIContainer* emDataContPtr = eFexContainer.cptr();
  ATH_CHECK(fillEMHistograms("LowPtCut", emDataContPtr, m_lowPtCut));
  ATH_CHECK(fillEMHistograms("HiPtCut", emDataContPtr, m_hiPtCut));

  // Begin the Tau processing
  // Access eFex Tau container
  SG::ReadHandle<xAOD::eFexTauRoIContainer> eFexTauContainer{m_eFexTauContainerKey, ctx};
  if(!eFexTauContainer.isValid()){
    ATH_MSG_ERROR("No eFex Tau container found in storegate  "<< m_eFexTauContainerKey);
    return StatusCode::SUCCESS;
  }
  // Fill histogram for the total number of ToBs (without any cuts applied)
  auto nTauTOBs_total = Monitored::Scalar<int>("nTauTOBs_nocut",0.0); // Overall number of ToBs
  nTauTOBs_total = eFexTauContainer->size();
  fill(m_packageName, nTauTOBs_total);
  // Fill histograms for the low/high cuts
  const xAOD::eFexTauRoIContainer* tauDataContPtr = eFexTauContainer.cptr();
  ATH_CHECK(fillTauHistograms("LowPtCut", tauDataContPtr, m_lowPtCut));
  ATH_CHECK(fillTauHistograms("HiPtCut", tauDataContPtr, m_hiPtCut));

  return StatusCode::SUCCESS;
}

StatusCode EfexMonitorAlgorithm::fillEMHistograms(const std::string& cut_name, const xAOD::eFexEMRoIContainer *emcont, const float &cut_et) const {
  
  ATH_MSG_DEBUG("EfexMonitorAlgorithm::fillEMHistograms");
  std::string groupName = m_packageName+'_'+cut_name;

  // monitored variables for histograms
  auto nEmTOBs_passcut = Monitored::Scalar<int>("nEMTOBs",0.0); // Number of ToBs passing the cut
  auto TOBeT = Monitored::Scalar<float>("TOBTransverseEnergy",0.0);
  auto TOBeta = Monitored::Scalar<float>("TOBEta",0.0);
  auto TOBphi = Monitored::Scalar<float>("TOBPhi",0.0);
  auto TOBshelfNumber = Monitored::Scalar<float>("TOBshelfNumber",0.0);
  auto TOBeFEXNumberSh0 = Monitored::Scalar<float>("TOBeFEXNumberSh0",0.0);
  auto TOBeFEXNumberSh1 = Monitored::Scalar<float>("TOBeFEXNumberSh1",0.0);
  auto TOBfpga = Monitored::Scalar<float>("TOBfpga",0.0);
  auto TOBReta = Monitored::Scalar<float>("TOBReta",0.0);
  auto TOBRhad = Monitored::Scalar<float>("TOBRhad",0.0);
  auto TOBWstot = Monitored::Scalar<float>("TOBWstot",0.0);
  auto TOBReta_threshold = Monitored::Scalar<float>("TOBReta_threshold",0.0);
  auto TOBRhad_threshold = Monitored::Scalar<float>("TOBRhad_threshold",0.0);
  auto TOBWstot_threshold = Monitored::Scalar<float>("TOBWstot_threshold",0.0);

  for(const xAOD::eFexEMRoI* efexEmRoI : *emcont){
    if (efexEmRoI->et() > cut_et){
      nEmTOBs_passcut += 1;
      TOBeT = efexEmRoI->et();
      fill(groupName, TOBeT);
      TOBeta = efexEmRoI->eta();
      TOBphi = efexEmRoI->phi();
      fill(groupName, TOBeta, TOBphi);
      TOBshelfNumber=efexEmRoI->shelfNumber();
      fill(groupName, TOBshelfNumber);
      // Only fill the relevant shelf variable
      if (TOBshelfNumber == 0) {
        TOBeFEXNumberSh0 = int(efexEmRoI->eFexNumber());
        fill(groupName, TOBeFEXNumberSh0);
      } else {
        TOBeFEXNumberSh1 = int(efexEmRoI->eFexNumber());
        fill(groupName, TOBeFEXNumberSh1);
      }
      TOBfpga = efexEmRoI->fpga();
      fill(groupName, TOBfpga);
      TOBReta = efexEmRoI->Reta();
      fill(groupName, TOBReta);
      TOBRhad = efexEmRoI->Rhad();
      fill(groupName, TOBRhad);
      TOBWstot = efexEmRoI->Wstot();
      fill(groupName, TOBWstot);
      TOBReta_threshold = efexEmRoI->RetaThresholds();
      fill(groupName, TOBReta_threshold);
      TOBRhad_threshold = efexEmRoI->RhadThresholds();
      fill(groupName, TOBRhad_threshold);
      TOBWstot_threshold = efexEmRoI->WstotThresholds();
      fill(groupName, TOBWstot_threshold);
    }
  }
  fill(groupName, nEmTOBs_passcut);

  return StatusCode::SUCCESS;
}

StatusCode EfexMonitorAlgorithm::fillTauHistograms(const std::string& cut_name, const xAOD::eFexTauRoIContainer *taucont, const float &cut_et) const {

  ATH_MSG_DEBUG("EfexMonitorAlgorithm::fillTauHistograms");
  std::string groupName = m_packageName+'_'+cut_name;

  // monitored variables for histograms
  auto nTauTOBs_passcut = Monitored::Scalar<int>("nTauTOBs",0.0); // Number of ToBs passing the cut
  auto tauTOBeT = Monitored::Scalar<float>("tauTOBTransverseEnergy",0.0);
  auto tauTOBeta = Monitored::Scalar<float>("tauTOBEta",0.0);
  auto tauTOBphi = Monitored::Scalar<float>("tauTOBPhi",0.0);
  auto tauTOBshelfNumber = Monitored::Scalar<float>("tauTOBshelfNumber",0.0);
  auto tauTOBeFEXNumberSh0 = Monitored::Scalar<float>("tauTOBeFEXNumberSh0",0.0);
  auto tauTOBeFEXNumberSh1 = Monitored::Scalar<float>("tauTOBeFEXNumberSh1",0.0);
  auto tauTOBfpga = Monitored::Scalar<float>("tauTOBfpga",0.0);
  auto tauTOBRcore = Monitored::Scalar<float>("tauTOBRcore",0.0);
  auto tauTOBRhad = Monitored::Scalar<float>("tauTOBRhad",0.0);
  auto tauTOBRcore_threshold = Monitored::Scalar<float>("tauTOBRcore_threshold",0.0);
  auto tauTOBRhad_threshold = Monitored::Scalar<float>("tauTOBRhad_threshold",0.0);
  auto tauTOBthree_threshold = Monitored::Scalar<float>("tauTOBthree_threshold",0.0);

  for(const xAOD::eFexTauRoI* efexTauRoI : *taucont){
    if (efexTauRoI->et() > cut_et){
      nTauTOBs_passcut += 1;
      tauTOBeT = efexTauRoI->et();
      fill(groupName, tauTOBeT);
      tauTOBeta = efexTauRoI->eta();
      tauTOBphi = efexTauRoI->phi();
      fill(groupName, tauTOBeta, tauTOBphi);
      tauTOBshelfNumber=efexTauRoI->shelfNumber();
      fill(groupName, tauTOBshelfNumber);
      // Only fill the relevant shelf variable
      if (tauTOBshelfNumber == 0) {
        tauTOBeFEXNumberSh0 = int(efexTauRoI->eFexNumber());
        fill(groupName, tauTOBeFEXNumberSh0);
      } else {
        tauTOBeFEXNumberSh1 = int(efexTauRoI->eFexNumber());
        fill(groupName, tauTOBeFEXNumberSh1);
      }
      tauTOBfpga = efexTauRoI->fpga();
      fill(groupName, tauTOBfpga);
      tauTOBRcore = efexTauRoI->rCore();
      fill(groupName, tauTOBRcore);
      tauTOBRhad = efexTauRoI->rHad();
      fill(groupName, tauTOBRhad);
      tauTOBRcore_threshold=efexTauRoI->rCoreThresholds();
      fill(groupName, tauTOBRcore_threshold);
      tauTOBRhad_threshold=efexTauRoI->rHadThresholds();
      fill(groupName, tauTOBRhad_threshold);
      tauTOBthree_threshold=efexTauRoI->tauThreeThresholds();
      fill(groupName, tauTOBthree_threshold);
    }
  }
  fill(groupName, nTauTOBs_passcut);

  return StatusCode::SUCCESS;
}