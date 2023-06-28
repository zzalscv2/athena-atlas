/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// This source file implements all of the functions related to Photons
// in the SUSYObjDef_xAOD class

// Local include(s):
#include "SUSYTools/SUSYObjDef_xAOD.h"

#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "EgammaAnalysisInterfaces/IEGammaAmbiguityTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgDeadHVCellRemovalTool.h"

#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "IsolationSelection/IIsolationSelectionTool.h"

// Helper for object quality
#include "EgammaAnalysisHelpers/PhotonHelpers.h"

#ifndef XAOD_STANDALONE // For now metadata is Athena-only
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

namespace ST {

StatusCode SUSYObjDef_xAOD::GetPhotons(xAOD::PhotonContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& photonkey, const xAOD::PhotonContainer* containerToBeCopied)
{
  if (!m_tool_init) {
    ATH_MSG_ERROR("SUSYTools was not initialized!!");
    return StatusCode::FAILURE;
  }

  if (m_isPHYSLITE && photonkey.find("AnalysisPhotons")==std::string::npos){
    ATH_MSG_ERROR("You are running on PHYSLITE derivation. Please change the Photons container to 'AnalysisPhotons'");
    return StatusCode::FAILURE;
  }

  const xAOD::PhotonContainer* photons = nullptr;
  if (copy==nullptr) { // empty container provided
    if (containerToBeCopied != nullptr) {
      photons = containerToBeCopied;
    }
    else {
      ATH_CHECK( evtStore()->retrieve(photons, photonkey) );
    }
    std::pair<xAOD::PhotonContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*photons);
    copy = shallowcopy.first;
    copyaux = shallowcopy.second;
    bool setLinks = xAOD::setOriginalObjectLink(*photons, *copy);
    if (!setLinks) {
      ATH_MSG_WARNING("Failed to set original object links on " << photonkey);
    }
  } else { // use the user-supplied collection instead 
    ATH_MSG_DEBUG("Not retrieving photon collecton, using existing one provided by user");
    photons=copy;
  }  

  for (const auto& photon : *copy) {
    ATH_CHECK( this->FillPhoton(*photon, m_photonBaselinePt, m_photonBaselineEta) );
    this->IsSignalPhoton(*photon, m_photonPt, m_photonEta);
  }

  if (recordSG) {
    ATH_CHECK( evtStore()->record(copy, "STCalib" + photonkey + m_currentSyst.name()) );
    ATH_CHECK( evtStore()->record(copyaux, "STCalib" + photonkey + m_currentSyst.name() + "Aux.") );
  }
  return StatusCode::SUCCESS;
}


StatusCode SUSYObjDef_xAOD::FillPhoton(xAOD::Photon& input, float ptcut, float etacut) {

  ATH_MSG_VERBOSE( "Starting FillPhoton on ph with pre-calibration pt=" << input.pt() );

  if ( !input.caloCluster() ) {
     ATH_MSG_WARNING( "FillPhoton: no caloCluster found: " << input.caloCluster() );
     return StatusCode::SUCCESS;
  }

 //Check DeadHVCellRemoval
  bool pass_deadHVTool = m_deadHVTool->accept(&input);

  if (m_debug) {
    ATH_MSG_INFO( "PHOTON eta: " << input.eta() );
    ATH_MSG_INFO( "PHOTON phi: " << input.phi() );
    ATH_MSG_INFO( "PHOTON cl eta: " << input.caloCluster()->eta() );
    ATH_MSG_INFO( "PHOTON cl phi: " << input.caloCluster()->phi() );
    ATH_MSG_INFO( "PHOTON cl e: " << input.caloCluster()->e() );
    ATH_MSG_INFO( "PHOTON OQ: " << input.OQ() );
    ATH_MSG_INFO( "PHOTON author: " << input.author() );
    ATH_MSG_INFO( "PHOTON deadHVTools: " << pass_deadHVTool );
  }

  dec_baseline(input) = false;
  dec_selected(input) = 0;
  dec_isol(input) = false;
  dec_isEM(input) = 0;

  if (!pass_deadHVTool) return StatusCode::SUCCESS; 

  // Author cuts needed according to https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Photon_authors
  if ( !(input.author() & (xAOD::EgammaParameters::AuthorPhoton + xAOD::EgammaParameters::AuthorAmbiguous)) )
    return StatusCode::SUCCESS;

  if (m_egammaCalibTool->applyCorrection(input)  != CP::CorrectionCode::Ok)
    ATH_MSG_ERROR("FillPhoton: EgammaCalibTool applyCorrection failed");

  if (m_isoCorrTool->applyCorrection(input)  != CP::CorrectionCode::Ok)
    ATH_MSG_ERROR("FillPhoton: IsolationCorrectionTool applyCorrection failed");

  ATH_MSG_VERBOSE( "FillPhoton: post-calibration pt=" << input.pt() );

  if (input.pt() < ptcut || std::abs(input.caloCluster()->etaBE(2)) >= etacut) return StatusCode::SUCCESS;

  if (m_photonBaselineCrackVeto){
    if  ( std::abs( input.caloCluster()->etaBE(2) ) >1.37 &&  std::abs( input.caloCluster()->etaBE(2) ) <1.52) {
      return StatusCode::SUCCESS; 
    }
  }

  //Object quality cut as described at https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Object_quality_cut
  if (!input.isGoodOQ(xAOD::EgammaParameters::BADCLUSPHOTON))
    return StatusCode::SUCCESS;

  //Photon quality as in https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Photon_cleaning
  bool passPhCleaning = false;
  if (acc_passPhCleaning.isAvailable(input) && acc_passPhCleaningNoTime.isAvailable(input)) {
    if ( (!m_photonAllowLate && acc_passPhCleaning(input)) || (m_photonAllowLate && acc_passPhCleaningNoTime(input)) ) passPhCleaning = true;
  } else {
    ATH_MSG_VERBOSE ("DFCommonPhotonsCleaning is not found in DAOD..");
    if ( (!m_photonAllowLate && PhotonHelpers::passOQquality(input)) || 
         ( m_photonAllowLate && PhotonHelpers::passOQqualityDelayed(input)) ) passPhCleaning = true;
  }
  if (!passPhCleaning) return StatusCode::SUCCESS;


  bool passBaseID = false;
  if (m_acc_photonIdBaseline.isAvailable(input)) {
    passBaseID = bool(m_acc_photonIdBaseline(input));
    //disable m_photonSelIsEM->accept(&input);
    //disable dec_isEM(input) = bool(m_photonSelIsEM->IsemValue());
  } else {
    ATH_MSG_VERBOSE ("DFCommonPhotonsIsEMxxx variables are not found. Calculating the ID from Photon ID tool..");
    if (!isAtlfast() && !isData()) {
      if ( m_electronPhotonShowerShapeFudgeTool->applyCorrection(input) != CP::CorrectionCode::Ok)
        ATH_MSG_ERROR("FillPhoton - fudge tool: applyCorrection failed");
    }
    passBaseID = bool(m_photonSelIsEMBaseline->accept(&input));
    //disable m_photonSelIsEM->accept(&input);
    //disable dec_isEM(input) = bool(m_photonSelIsEM->IsemValue());
  }
  if (!passBaseID) return StatusCode::SUCCESS;

  //--- Do baseline isolation check
  if ( !( m_photonBaselineIso_WP.empty() ) &&  !( m_isoBaselineTool->accept(input) ) ) return StatusCode::SUCCESS;

  dec_baseline(input) = true;
  dec_selected(input) = 2;
  if (!m_photonIso_WP.empty()) dec_isol(input) = bool(m_isoTool->accept(input));

  ATH_MSG_VERBOSE("FillPhoton: passed baseline selection.");
  return StatusCode::SUCCESS;
}


bool SUSYObjDef_xAOD::IsSignalPhoton(const xAOD::Photon& input, float ptcut, float etacut) const
{
  dec_signal(input) = false;

  if ( !acc_baseline(input) ) return false;

  if ( !m_egammaAmbiguityTool->accept(input) ) return false;

  if ( input.pt() < ptcut ) return false;
  if ( etacut==DUMMYDEF ){
    if(std::abs(input.caloCluster()->etaBE(2)) > m_photonEta ) return false;
  }
  else if ( std::abs(input.caloCluster()->etaBE(2)) > etacut ) return false;

  if (m_photonCrackVeto){
    if  ( std::abs( input.caloCluster()->etaBE(2) ) >1.37 &&  std::abs( input.caloCluster()->etaBE(2) ) <1.52) {
      return false; 
    }
  }

  if (acc_isol(input) || !m_doPhIsoSignal) {
    ATH_MSG_VERBOSE( "IsSignalPhoton: passed isolation");
  } else return false;

  bool passID = false;
  if (m_acc_photonId.isAvailable(input)) {
    passID = m_acc_photonId(input);
  } else {
    ATH_MSG_VERBOSE ("DFCommonPhotonsIsEMxxx variables are not found. Calculating the ID from Photon ID tool..");
    passID = bool(m_photonSelIsEM->accept(&input));
  }
  if ( !passID ) return false;
  
  dec_signal(input) = true;

  return true;
}


double SUSYObjDef_xAOD::GetSignalPhotonSF(const xAOD::Photon& ph, const bool effSF, const bool isoSF, const bool triggerSF) const
{
  double sf(1.);

  if (effSF) {

    double sf_eff = 1.;

    CP::CorrectionCode res = m_photonEfficiencySFTool->getEfficiencyScaleFactor( ph, sf_eff );
    if (res == CP::CorrectionCode::OutOfValidityRange) ATH_MSG_WARNING(" GetSignalPhotonSF: EfficiencyScaleFactor out of validity range");

    sf *= sf_eff;
  }

  if (isoSF) {

    double sf_iso = 1.;

    CP::CorrectionCode res = m_photonIsolationSFTool->getEfficiencyScaleFactor( ph, sf_iso );
    if (res == CP::CorrectionCode::OutOfValidityRange) ATH_MSG_WARNING(" GetSignalPhotonSF: IsolationScaleFactor out of validity range");

    sf *= sf_iso;
  }

  if (triggerSF) {

    double sf_trigger = 1.;

    CP::CorrectionCode res = m_photonTriggerSFTool->getEfficiencyScaleFactor( ph, sf_trigger );
    if (res == CP::CorrectionCode::OutOfValidityRange) ATH_MSG_WARNING(" GetSignalPhotonSF: TriggerScaleFactor out of validity range");

    sf *= sf_trigger;
  }

  ATH_MSG_VERBOSE( "ScaleFactor " << sf );

  dec_effscalefact(ph) = sf;
  return sf;
}


double SUSYObjDef_xAOD::GetSignalPhotonSFsys(const xAOD::Photon& ph, const CP::SystematicSet& systConfig, const bool effSF, const bool isoSF, const bool triggerSF)
{
  double sf(1.);

  //Set the new systematic variation
  StatusCode ret = m_photonEfficiencySFTool->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgPhotonEfficiencyCorrectionTool (reco) for systematic var. " << systConfig.name() );
  }

  ret = m_photonIsolationSFTool->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgPhotonEfficiencyCorrectionTool (iso) for systematic var. " << systConfig.name() );
  }

  ret = m_photonTriggerSFTool->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgPhotonEfficiencyCorrectionTool (trigger) for systematic var. " << systConfig.name() );
  }

  if (effSF) {

    double sf_eff = 1.;

    CP::CorrectionCode res = m_photonEfficiencySFTool->getEfficiencyScaleFactor( ph, sf_eff );
    if (res == CP::CorrectionCode::OutOfValidityRange) ATH_MSG_WARNING(" GetSignalPhotonSF: getEfficiencyScaleFactor out of validity range");

    sf *= sf_eff;
  }

  if (isoSF) {

    double sf_iso = 1.;

    CP::CorrectionCode res = m_photonIsolationSFTool->getEfficiencyScaleFactor( ph, sf_iso );
    if (res == CP::CorrectionCode::OutOfValidityRange) ATH_MSG_WARNING(" GetSignalPhotonSF: getEfficiencyScaleFactor out of validity range");

    sf *= sf_iso;
  }

  if (triggerSF) {

    double sf_trigger = 1.;

    CP::CorrectionCode res = m_photonTriggerSFTool->getEfficiencyScaleFactor( ph, sf_trigger );
    if (res == CP::CorrectionCode::OutOfValidityRange) ATH_MSG_WARNING(" GetSignalPhotonSF: getEfficiencyScaleFactor out of validity range");

    sf *= sf_trigger;
  }

  //Roll back to current sys
  ret = m_photonEfficiencySFTool->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgPhotonEfficiencyCorrectionTool back to default.");
  }

  ret = m_photonIsolationSFTool->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgPhotonEfficiencyCorrectionTool (iso) for systematic var. " << systConfig.name() );
  }

  ret = m_photonTriggerSFTool->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgPhotonEfficiencyCorrectionTool (trigger) for systematic var. " << systConfig.name() );
  }

  ATH_MSG_VERBOSE( "ScaleFactor " << sf );

  dec_effscalefact(ph) = sf;
  return sf;
}


double SUSYObjDef_xAOD::GetTotalPhotonSF(const xAOD::PhotonContainer& photons, const bool effSF, const bool isoSF, const bool triggerSF) const
{
  double sf(1.);

  for (const xAOD::Photon* photon : photons) {
    if (acc_signal(*photon) && acc_passOR(*photon)) { sf *= this->GetSignalPhotonSF(*photon, effSF, isoSF, triggerSF); }
  }

  return sf;
}


double SUSYObjDef_xAOD::GetTotalPhotonSFsys(const xAOD::PhotonContainer& photons, const CP::SystematicSet& systConfig, const bool effSF, const bool isoSF, const bool triggerSF)
{
  double sf(1.);

  for (const xAOD::Photon* photon : photons) {
    if (acc_signal(*photon) && acc_passOR(*photon)) { sf *= this->GetSignalPhotonSFsys(*photon, systConfig, effSF, isoSF, triggerSF); }
  }

  return sf;
}

}
