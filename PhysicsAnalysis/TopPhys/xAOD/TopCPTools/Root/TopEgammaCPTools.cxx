/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TopCPTools/TopEgammaCPTools.h"

#include <map>
#include <string>

// Top includes
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

// PathResolver include(s):
#include "PathResolver/PathResolver.h"

// Egamma include(s):
#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "ElectronEfficiencyCorrection/ElectronChargeEfficiencyCorrectionTool.h"
#include "ElectronPhotonSelectorTools/AsgPhotonIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h"
#include "Root/EGSelectorConfigurationMapping.h"

#include "AsgTools/AnaToolHandle.h"

namespace top {
  EgammaCPTools::EgammaCPTools(const std::string& name) :
    asg::AsgTool(name),
    m_egammaCalibrationModel("es2022_R22_PRE"),
    m_electronEffTriggerFile("SetMe"),
    m_electronEffTriggerLooseFile("SetMe"),
    m_electronEffSFTriggerFile("SetMe"),
    m_electronEffSFTriggerLooseFile("SetMe"),
    m_electronEffSFRecoFile("SetMe"),
    m_electronEffSFIDFile("SetMe"),
    m_electronEffSFIDLooseFile("SetMe"),
    m_electronEffSFIsoFile("SetMe"),
    m_electronEffSFIsoLooseFile("SetMe"),
    m_electronEffSFChargeIDFile("SetMe"),
    m_electronEffSFChargeIDLooseFile("SetMe"),
    m_electronEffSFChargeMisIDFile("SetMe"),
    m_electronEffSFChargeMisIDLooseFile("SetMe") {
//    m_fwdElectronEffSFIDFile("SetMe"),
//    m_fwdElectronEffSFIDLooseFile("SetMe") {
    declareProperty("config", m_config);

    declareProperty("EgammaCalibrationAndSmearingTool", m_egammaCalibrationAndSmearingTool);
    declareProperty("ElectronEffTrigger", m_electronEffTrigger);
    declareProperty("ElectronEffTriggerLoose", m_electronEffTriggerLoose);
    declareProperty("ElectronEffSFTrigger", m_electronEffSFTrigger);
    declareProperty("ElectronEffSFTriggerLoose", m_electronEffSFTriggerLoose);
    declareProperty("ElectronEffReco", m_electronEffSFReco);
    declareProperty("ElectronEffID", m_electronEffSFID);
    declareProperty("ElectronEffIDLoose", m_electronEffSFIDLoose);
    declareProperty("ElectronEffIso", m_electronEffSFIso);
    declareProperty("ElectronEffIsoLoose", m_electronEffSFIsoLoose);
    declareProperty("ElectronEffChargeID", m_electronEffSFChargeID);
    declareProperty("ElectronEffChargeIDLoose", m_electronEffSFChargeIDLoose);

    declareProperty("ElectronEffTriggerCorrModel", m_electronEffTriggerCorrModel);
    declareProperty("ElectronEffTriggerLooseCorrModel", m_electronEffTriggerLooseCorrModel);
    declareProperty("ElectronEffSFTriggerCorrModel", m_electronEffSFTriggerCorrModel);
    declareProperty("ElectronEffSFTriggerLooseCorrModel", m_electronEffSFTriggerLooseCorrModel);
    declareProperty("ElectronEffRecoCorrModel", m_electronEffSFRecoCorrModel);
    declareProperty("ElectronEffIDCorrModel", m_electronEffSFIDCorrModel);
    declareProperty("ElectronEffIDLooseCorrModel", m_electronEffSFIDLooseCorrModel);
    declareProperty("ElectronEffIsoCorrModel", m_electronEffSFIsoCorrModel);
    declareProperty("ElectronEffIsoLooseCorrModel", m_electronEffSFIsoLooseCorrModel);

    declareProperty("PhotonIsEMSelectorLoose", m_photonLooseIsEMSelector);
    declareProperty("PhotonIsEMSelectorMedium", m_photonMediumIsEMSelector);
    declareProperty("PhotonIsEMSelectorTight", m_photonTightIsEMSelector);

//    declareProperty("FwdElectronSelector", m_fwdElectronSelector);
//    declareProperty("FwdElectronSelectorLoose", m_fwdElectronSelectorLoose);
//    declareProperty("FwdElectronEffIDLoose", m_fwdElectronEffSFIDLoose);
  }

  StatusCode EgammaCPTools::initialize() {
    ATH_MSG_INFO("top::EgammaCPTools initialize...");

    if (m_config->isTruthDxAOD()) {
      ATH_MSG_INFO("top::EgammaCPTools: no need to initialise anything on truth DxAOD");
      return StatusCode::SUCCESS;
    }

    if (m_config->usePhotons() || m_config->useElectrons() || m_config->useFwdElectrons()) {
      if (m_config->makeAllCPTools()) {// skiping calibrations on mini-xAODs
        if(m_config->egammaCalibration() != m_egammaCalibrationModel){
          m_config->setPrintEgammaCalibModelWarning(true);
          m_egammaCalibrationModel = m_config->egammaCalibration();
        }
        top::check(setupCalibration(), "Failed to setup Egamma calibration tools");
      }
      if (m_config->useFwdElectrons() && m_config->makeAllCPTools()) {
        top::check(setupSelectors(), "Failed to setup Fwd electrons selectors tools");
      }
      if (m_config->isMC()) {// scale-factors are only for MC
        top::check(setupScaleFactors(), "Failed to setup Egamma scale-factor tools");
      }
    } else {
      ATH_MSG_INFO(
        "top::EgammaCPTools: no need to initialise anything since using neither electrons nor fwd electrons nor photons");
    }

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaCPTools::setupSelectors() {
    ATH_MSG_INFO("top::EgammaCPTools setupSelectors..");
    if (m_config->useFwdElectrons()) {
//      m_fwdElectronSelector = new AsgForwardElectronLikelihoodTool("CP::FwdElectronSelector");
//      top::check(m_fwdElectronSelector->setProperty("ConfigFile",
//                                                    EgammaSelectors::ForwardLHPointToConfFile.at(
//                                                      m_config->fwdElectronID() + "LHForwardElectron")),
//                 "Failed to set config for AsgElectronFwdLikelihoodTool");
//      top::check(m_fwdElectronSelector->initialize(), "Couldn't initialise Forward Electron LH ID Tool");
//
//      m_fwdElectronSelectorLoose = new AsgForwardElectronLikelihoodTool("CP::FwdElectronSelectorLoose");
//      top::check(m_fwdElectronSelectorLoose->setProperty("ConfigFile",
//                                                         EgammaSelectors::ForwardLHPointToConfFile.at(
//                                                           m_config->fwdElectronIDLoose() + "LHForwardElectron")),
//                 "Failed to set config for AsgElectronFwdLikelihoodTool");
//      top::check(m_fwdElectronSelectorLoose->initialize(), "Couldn't initialise Forward Electron LH ID Loose Tool");

    }
    return StatusCode::SUCCESS;
  }

  StatusCode EgammaCPTools::setupCalibration() {
    // Setup electron and photon calibration tools
    // List of tools include:
    // - EgammaCalibrationAndSmearingTool
    // - Electron Charge ID Selector tool
    // - Photon shower shape fudge tool
    // - Photon efficiency correction tool
    using IEgammaCalibTool = CP::IEgammaCalibrationAndSmearingTool;
    const std::string egamma_calib_name = "EgammaCalibrationAndSmearingTool";
    if (asg::ToolStore::contains<IEgammaCalibTool>(egamma_calib_name)) {
      m_egammaCalibrationAndSmearingTool = asg::ToolStore::get<IEgammaCalibTool>(egamma_calib_name);
    } else {
      IEgammaCalibTool* egammaCalibrationAndSmearingTool = new CP::EgammaCalibrationAndSmearingTool(egamma_calib_name);
      top::check(asg::setProperty(egammaCalibrationAndSmearingTool,
                                  "ESModel", m_egammaCalibrationModel),
                 "Failed to set ESModel for " + egamma_calib_name);
      top::check(asg::setProperty(egammaCalibrationAndSmearingTool,
                                  "decorrelationModel",
                                  m_config->egammaSystematicModel()),
                 "Failed to set decorrelationModel for " + egamma_calib_name);

      if (m_config->forceRandomRunNumber() > 0) {
        top::check(asg::setProperty(egammaCalibrationAndSmearingTool, "randomRunNumber", m_config->forceRandomRunNumber()), "Cannot set randomRunNumber for the egamma tools");
      }

      top::check(asg::setProperty(egammaCalibrationAndSmearingTool,
				  "useFastSim", m_config->isAFII() ? 1 : 0),
		 "Failed to set useFastSim to true for" + egamma_calib_name);
      top::check(egammaCalibrationAndSmearingTool->initialize(),
                 "Failed to initialize " + egamma_calib_name);
      m_egammaCalibrationAndSmearingTool = egammaCalibrationAndSmearingTool;
    }

    // The terribly named ElectronPhotonShowerShapeFudgeTool...
    // We apply this only to photons to correct the shower shape
    // This should only be applied on MC
    using IFudgeTool = IElectronPhotonShowerShapeFudgeTool;
    const std::string fudgeName = "PhotonFudgeTool";
    m_photonFudgeTool = asg::AnaToolHandle<IElectronPhotonShowerShapeFudgeTool> ("ElectronPhotonVariableCorrectionTool/" + fudgeName);
    if (!asg::ToolStore::contains<IFudgeTool>(fudgeName)) {
      std::string configFilePath = "EGammaVariableCorrection/TUNE23/ElPhVariableNominalCorrection.conf";
      ANA_CHECK(m_photonFudgeTool.setProperty("ConfigFile",configFilePath));
      top::check(m_photonFudgeTool.initialize(), "Failed to initialize PhotonFudgeTool");
    }

    // The photon efficiency SF tool
    bool af2 = m_config->isAFII();
    int data_type = 0; // Data
    if (m_config->isMC()) {
      if (af2) {
        ATH_MSG_WARNING("PhotonEfficiencies - Currently there are no recommendations for FastSimulation photons");
        ATH_MSG_WARNING("PhotonEfficiencies - Therefore we are advised to treat FastSim as FullSim for configuration");
        //data_type = 3; // AF2
        data_type = 1; // AF2 masquerading as full sim
      } else {
        data_type = 1; // Full sim
      }
    }

    using IPhotonEffTool = IAsgPhotonEfficiencyCorrectionTool;
    const std::string photonSFName = "AsgPhotonEfficiencyCorrectionTool"; // to retrieve ID Eff scale factors
    if (asg::ToolStore::contains<IPhotonEffTool>(photonSFName)) {
      m_photonEffSF = asg::ToolStore::get<IPhotonEffTool>(photonSFName);
    } else {
      if (m_config->isMC()) {  // Seem to only be able to setup the tool for MC
        IPhotonEffTool* photonEffSF = new AsgPhotonEfficiencyCorrectionTool(photonSFName);
        top::check(asg::setProperty(photonEffSF, "ForceDataType", data_type),
                   "Failed to set ForceDataType for " + photonSFName);
        top::check(photonEffSF->initialize(),
                   "Failed to initialize " + photonSFName);
        m_photonEffSF = photonEffSF;
      }
    }

    // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/IsolationSF2016Moriond#Radiative_Z_low_ET
    // To retrieve Isolation Eff scale factors
    // N.B. Naming for isolation working points for AsgPhotonEfficiencyCorrectionTool isolation scale factors
    // are different than those for the IsolationCorrectionTool (preceded by FixedCut)
    std::set<std::string> photon_isolations = {
      "Tight",
      "Loose",
      "TightCaloOnly"
    };
    for (const std::string& isoWP : photon_isolations) {
      std::string photonIsoSFName = "AsgPhotonEfficiencyCorrectionTool_IsoSF" + isoWP;
      if (!asg::ToolStore::contains<IPhotonEffTool>(photonIsoSFName)) {
        if (m_config->isMC()) {
          IPhotonEffTool* photonIsoSFTool = new AsgPhotonEfficiencyCorrectionTool(photonIsoSFName);
          top::check(asg::setProperty(photonIsoSFTool, "ForceDataType", data_type),
                     "Failed to set ForceDataType for " + photonIsoSFName);
          top::check(asg::setProperty(photonIsoSFTool, "IsoKey", isoWP),
                     "Failed to set IsoKey for " + photonIsoSFName);
          top::check(photonIsoSFTool->initialize(),
                     "Failed to initialize " + photonIsoSFName);
          m_photonIsoSFTools.push_back(photonIsoSFTool);
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode EgammaCPTools::setupScaleFactors() {
    // Don't need for data, return SUCCESS straight away
    if (!m_config->isMC()) return StatusCode::SUCCESS;

    ///-- Scale factors --///
    std::string electron_data_dir = "ElectronEfficiencyCorrection/";

    // Define the data type variable - 0 : Data, 1 : MC FullSim, 3 : MC AFII
    int dataType(0);
    if (m_config->isMC()) {
      dataType = (m_config->isAFII()) ? 3 : 1;
    }

    ///-- Reco SFs doesn't depend on WP --///
    std::string electronID = m_config->electronID();
    if (electronID.find("LH") != std::string::npos) electronID.replace(electronID.find("LH"), 2, "LLH"); // that way people do not have to change their cuts file
    std::string electronIDLoose = m_config->electronIDLoose();
    if (electronIDLoose.find("LH") != std::string::npos) electronIDLoose.replace(electronIDLoose.find("LH"), 2, "LLH"); // that way people do not have to change their cuts file
    std::string electronIsolation = mapWorkingPoints(m_config->electronIsolationSF()); // temporary fix: we can hopefully remove the map soon!
    std::string electronIsolationLoose = mapWorkingPoints(m_config->electronIsolationSFLoose());

    if(electronIsolation == "PLImprovedTight" || electronIsolation == "PLImprovedVeryTight"){
      if(!(electronID == "TightLLH" || electronID == "MediumLLH")){
        ATH_MSG_ERROR("Combination of electron PLIV WP and ID WP not available. Try MediumLH or TightLH.");
        return StatusCode::FAILURE;
      }
    }
    if(electronIsolationLoose == "PLImprovedTight" || electronIsolationLoose == "PLImprovedVeryTight"){
      if(!(electronIDLoose == "TightLLH" || electronIDLoose == "MediumLLH")){
        ATH_MSG_ERROR("Combination of loose electron PLIV WP and ID WP not available. Try MediumLH or TightLH.");
        return StatusCode::FAILURE;
      }
    }
    if(electronIsolation == "PLImprovedTight" || electronIsolation == "PLImprovedVeryTight" || electronIsolationLoose == "PLImprovedTight" || electronIsolationLoose == "PLImprovedVeryTight"){
      if(dataType == 3){
        ATH_MSG_ERROR("electron PLIV WPs are only available for FullSim.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_WARNING("Trigger SFs for PLIV isolation unavailable. Proceeding without online Isolation.");
    }
    
    // Retrieve full path to maps for different types of tool
    m_electronEffSFRecoFile = electronSFMapFilePath("reco");
    // - Tight
    m_electronEffSFIDFile = electronSFMapFilePath("ID");
    std::vector<std::string> inExpID;
    if(m_config->electronIDSFFilePath() !="Default"){
      m_config->setPrintEIDFileWarning(true);
      inExpID.push_back(electronSFFilePath("EXPID", electronID, electronIsolation));
    }

    std::vector<std::string> inExpIDLoose;
    if(m_config->electronIDSFFileLoosePath() !="Default"){
      m_config->setPrintEIDFileWarning(true);
      inExpIDLoose.push_back(electronSFFilePath("EXPID_Loose", electronID, electronIsolation));
    }

    m_electronEffSFTriggerFile = electronSFMapFilePath("trigger");
    m_electronEffTriggerFile = electronSFMapFilePath("trigger");
    std::vector<std::string> inPLViso;
    if (electronIsolation == "PLVTight" || electronIsolation == "PLVLoose") {
      m_electronEffSFIsoFile = electronSFFilePath("PLV", electronID, electronIsolation);
      inPLViso.push_back(m_electronEffSFIsoFile);
    }
    else {
      m_electronEffSFIsoFile = electronSFMapFilePath("isolation");
    }
    // - Loose
    m_electronEffSFIDLooseFile = electronSFMapFilePath("ID");
    m_electronEffSFTriggerLooseFile = electronSFMapFilePath("trigger");
    m_electronEffTriggerLooseFile = electronSFMapFilePath("trigger");
    std::vector<std::string> inPLVisoLoose;
    if (electronIsolationLoose == "PLVTight" || electronIsolationLoose == "PLVLoose") {
      m_electronEffSFIsoLooseFile = electronSFFilePath("PLV", electronID, electronIsolationLoose);
      inPLVisoLoose.push_back(m_electronEffSFIsoLooseFile);
    }
    else {
      m_electronEffSFIsoLooseFile = electronSFMapFilePath("isolation");
    }

    // Define the trigger string for scale factors
    std::string trigger_string = "";

    if (m_config->elTrigEffConfig() != " " ) {
      trigger_string = m_config->elTrigEffConfig();
    }
    else{
      ATH_MSG_ERROR("EgammaCPTools::setupScaleFactors, ElectronTriggerEfficiencyConfig not set, unable to retrieve SFs");
      return StatusCode::FAILURE;
    }

    // Define the tool prefix name
    const std::string elSFPrefix = "AsgElectronEfficiencyCorrectionTool_";

    ATH_MSG_INFO("Setting up Electrons SF tool for TOTAL correlation model");
    // Configure the tools with the maps - Name, map, reco_key, ID_key, iso_key, trigger_key, data_type, for the TOTAL
    // correlation model
    // Reco SFs
    m_electronEffSFReco = setupElectronSFToolWithMap(elSFPrefix + "Reco", m_electronEffSFRecoFile, "Reconstruction", "",
                                                     "", "", dataType, "TOTAL", "", "");
    // ID SFs
    if(m_config->electronIDSFFilePath() =="Default") m_electronEffSFID = setupElectronSFToolWithMap(elSFPrefix + "ID", m_electronEffSFIDFile, "", electronID, "", "",
                                                   dataType, "TOTAL", "", "");
    else m_electronEffSFID = setupElectronSFTool(elSFPrefix + "ID", inExpID, dataType);
    if (m_config->electronIDSFFileLoosePath() =="Default") m_electronEffSFIDLoose = setupElectronSFToolWithMap(elSFPrefix + "IDLoose", m_electronEffSFIDLooseFile, "",
                                                        electronIDLoose, "", "", dataType, "TOTAL", "", "");
    else m_electronEffSFIDLoose = setupElectronSFTool(elSFPrefix + "IDLoose", inExpIDLoose, dataType);
    // Trigger SFs
    m_electronEffSFTrigger = setupElectronSFToolWithMap(elSFPrefix + "TriggerSF", m_electronEffSFTriggerFile, "",
                                                        electronID,
							(!(electronIsolation == "PLImprovedTight" || electronIsolation == "PLImprovedVeryTight")) ? electronIsolation : "",
							trigger_string, dataType,
                                                        "TOTAL", "", "");
    m_electronEffSFTriggerLoose = setupElectronSFToolWithMap(elSFPrefix + "TriggerSFLoose",
                                                             m_electronEffSFTriggerLooseFile, "", electronIDLoose,
                                                             (!(electronIsolationLoose == "PLImprovedTight" || electronIsolationLoose == "PLImprovedVeryTight")) ? electronIsolationLoose : "",
							     trigger_string, dataType, "TOTAL",
                                                             "", "");
    // Trigger Efficiencies
    m_electronEffTrigger = setupElectronSFToolWithMap(elSFPrefix + "Trigger", m_electronEffTriggerFile, "", electronID,
                                                      (!(electronIsolation == "PLImprovedTight" || electronIsolation == "PLImprovedVeryTight")) ? electronIsolation : "",
						      "Eff_" + trigger_string, dataType, "TOTAL", "",
                                                      "");
    m_electronEffTriggerLoose = setupElectronSFToolWithMap(elSFPrefix + "TriggerLoose", m_electronEffTriggerLooseFile,
                                                           "", electronIDLoose,
							   (!(electronIsolationLoose == "PLImprovedTight" || electronIsolationLoose == "PLImprovedVeryTight")) ? electronIsolationLoose : "",
                                                           "Eff_" + trigger_string, dataType, "TOTAL", "", "");
    // Isolation SFs
    if (electronIsolation == "PLVTight" || electronIsolation == "PLVLoose") {
      m_electronEffSFIso = setupElectronSFTool(elSFPrefix + "Iso", inPLViso, dataType);
    }
    else if(electronIsolation == "None" || electronIsolation == "") m_electronEffSFIso = nullptr;
    else {
      m_electronEffSFIso = setupElectronSFToolWithMap(elSFPrefix + "Iso", m_electronEffSFIsoFile, "", electronID,
						      electronIsolation, "", dataType, "TOTAL", "", "");
    }
    if (electronIsolationLoose == "PLVTight" || electronIsolationLoose == "PLVLoose") {
      m_electronEffSFIsoLoose = setupElectronSFTool(elSFPrefix + "IsoLoose", inPLVisoLoose, dataType);
    }
    else if(electronIsolationLoose == "None" || electronIsolationLoose == "") m_electronEffSFIsoLoose = nullptr;
    else {
      m_electronEffSFIsoLoose = setupElectronSFToolWithMap(elSFPrefix + "IsoLoose", m_electronEffSFIsoLooseFile, "",
							   electronIDLoose, electronIsolationLoose, "", dataType, "TOTAL", "",
							   "");
    }

    ATH_MSG_INFO(
      "Requested Electrons SF tool for " << m_config->electronEfficiencySystematicModel() << " correlation model");
     
    if (m_config->electronEfficiencySystematicModel() != "TOTAL") {
      ATH_MSG_INFO(
        "Setting up Electrons SF tool for " << m_config->electronEfficiencySystematicModel() << " correlation model");

      const std::string elSFPrefixCorrModel = elSFPrefix + "CorrModel_";
      // Reco SFs
      m_electronEffSFRecoCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "Reco", m_electronEffSFRecoFile,
                                                                "Reconstruction", "", "", "", dataType,
                                                                m_config->electronEfficiencySystematicModelNToys(),
                                                                m_config->electronEfficiencySystematicModelToySeed(),
                                                                m_config->electronEfficiencySystematicModel(),
                                                                m_config->electronEfficiencySystematicModelEtaBinning(),
                                                                m_config->electronEfficiencySystematicModelEtBinning());
      // ID SFs
      if(m_config->electronIDSFFilePath() =="Default") m_electronEffSFIDCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "ID", m_electronEffSFIDFile, "",
                                                              electronID, "", "", dataType,
                                                              m_config->electronEfficiencySystematicModelNToys(),
                                                              m_config->electronEfficiencySystematicModelToySeed(),
                                                              m_config->electronEfficiencySystematicModel(),
                                                              m_config->electronEfficiencySystematicModelEtaBinning(),
                                                              m_config->electronEfficiencySystematicModelEtBinning());
      else m_electronEffSFIDCorrModel = setupElectronSFTool(elSFPrefixCorrModel + "ID", inExpID, dataType,
                    m_config->electronEfficiencySystematicModel(),
                    m_config->electronEfficiencySystematicModelEtaBinning(),
                    m_config->electronEfficiencySystematicModelEtBinning());


      if(m_config->electronIDSFFileLoosePath() =="Default") m_electronEffSFIDLooseCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "IDLoose",
                                                                   m_electronEffSFIDLooseFile, "", electronIDLoose, "",
                                                                   "", dataType,
                    m_config->electronEfficiencySystematicModelNToys(),
                    m_config->electronEfficiencySystematicModelToySeed(),
                                                                   m_config->electronEfficiencySystematicModel(),
                                                                   m_config->electronEfficiencySystematicModelEtaBinning(),
                                                                   m_config->electronEfficiencySystematicModelEtBinning());

      else m_electronEffSFIDLooseCorrModel = setupElectronSFTool(elSFPrefixCorrModel + "IDLoose", inExpIDLoose, dataType,
                    m_config->electronEfficiencySystematicModel(),
                    m_config->electronEfficiencySystematicModelEtaBinning(),
                    m_config->electronEfficiencySystematicModelEtBinning());

      // Trigger SFs
      m_electronEffSFTriggerCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "TriggerSF",
                                                                   m_electronEffSFTriggerFile, "", electronID,
                                                                   (!(electronIsolation == "PLImprovedTight" || electronIsolation == "PLImprovedVeryTight")) ? electronIsolation : "",
								   trigger_string, dataType,
                    m_config->electronEfficiencySystematicModelNToys(),
                    m_config->electronEfficiencySystematicModelToySeed(),
                                                                   m_config->electronEfficiencySystematicModel(),
                                                                   m_config->electronEfficiencySystematicModelEtaBinning(),
                                                                   m_config->electronEfficiencySystematicModelEtBinning());
      m_electronEffSFTriggerLooseCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "TriggerSFLoose",
                                                                        m_electronEffSFTriggerLooseFile, "",
                                                                        electronIDLoose,
									(!(electronIsolationLoose == "PLImprovedTight" || electronIsolationLoose == "PLImprovedVeryTight")) ? electronIsolationLoose : "",
                                                                        trigger_string, dataType,
                    m_config->electronEfficiencySystematicModelNToys(),
                    m_config->electronEfficiencySystematicModelToySeed(),
                                                                        m_config->electronEfficiencySystematicModel(),
                                                                        m_config->electronEfficiencySystematicModelEtaBinning(),
                                                                        m_config->electronEfficiencySystematicModelEtBinning());
      // Trigger Efficiencies
      m_electronEffTriggerCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "Trigger",
                                                                 m_electronEffTriggerFile, "", electronID,
                                                                 (!(electronIsolation == "PLImprovedTight" || electronIsolation == "PLImprovedVeryTight")) ? electronIsolation : "",
								 "Eff_" + trigger_string, dataType,
                    m_config->electronEfficiencySystematicModelNToys(),
                    m_config->electronEfficiencySystematicModelToySeed(),
                                                                 m_config->electronEfficiencySystematicModel(),
                                                                 m_config->electronEfficiencySystematicModelEtaBinning(),
                                                                 m_config->electronEfficiencySystematicModelEtBinning());
      m_electronEffTriggerLooseCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "TriggerLoose",
                                                                      m_electronEffTriggerLooseFile, "",
                                                                      electronIDLoose,
								      (!(electronIsolationLoose == "PLImprovedTight" || electronIsolationLoose == "PLImprovedVeryTight")) ? electronIsolationLoose : "",
                                                                      "Eff_" + trigger_string, dataType,
                    m_config->electronEfficiencySystematicModelNToys(),
                    m_config->electronEfficiencySystematicModelToySeed(),
                                                                      m_config->electronEfficiencySystematicModel(),
                                                                      m_config->electronEfficiencySystematicModelEtaBinning(),
                                                                      m_config->electronEfficiencySystematicModelEtBinning());
      // Isolation SFs
      if (electronIsolation == "PLVTight" || electronIsolation == "PLVLoose") {
        m_electronEffSFIsoCorrModel = setupElectronSFTool(elSFPrefixCorrModel + "Iso", inPLViso, dataType,
			  m_config->electronEfficiencySystematicModel(),
			  m_config->electronEfficiencySystematicModelEtaBinning(),
			  m_config->electronEfficiencySystematicModelEtBinning());
      }
      else if(electronIsolation == "None" || electronIsolation == "") m_electronEffSFIsoCorrModel = nullptr;
      else {
        m_electronEffSFIsoCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "Iso", m_electronEffSFIsoFile, "",
								 electronID, electronIsolation, "", dataType,
                 m_config->electronEfficiencySystematicModelNToys(),
                 m_config->electronEfficiencySystematicModelToySeed(),
								 m_config->electronEfficiencySystematicModel(),
								 m_config->electronEfficiencySystematicModelEtaBinning(),
								 m_config->electronEfficiencySystematicModelEtBinning());
      }
      if (electronIsolationLoose == "PLVTight" || electronIsolationLoose == "PLVLoose") {
        m_electronEffSFIsoLooseCorrModel = setupElectronSFTool(elSFPrefixCorrModel + "IsoLoose", inPLVisoLoose, dataType,
							       m_config->electronEfficiencySystematicModel(),
							       m_config->electronEfficiencySystematicModelEtaBinning(),
							       m_config->electronEfficiencySystematicModelEtBinning());

      }
      else if(electronIsolationLoose == "None" || electronIsolationLoose == "") m_electronEffSFIsoLooseCorrModel = nullptr;
      else {
        m_electronEffSFIsoLooseCorrModel = setupElectronSFToolWithMap(elSFPrefixCorrModel + "IsoLoose",
								      m_electronEffSFIsoLooseFile, "", electronIDLoose,
								      electronIsolationLoose, "", dataType,
                      m_config->electronEfficiencySystematicModelNToys(),
                      m_config->electronEfficiencySystematicModelToySeed(),
								      m_config->electronEfficiencySystematicModel(),
								      m_config->electronEfficiencySystematicModelEtaBinning(),
								      m_config->electronEfficiencySystematicModelEtBinning());
      }
    }

    if (m_config->useFwdElectrons()) {
      ATH_MSG_INFO("Setting up forward Electrons SF tool");

//      m_fwdElectronEffSFIDFile = electronSFMapFilePath("FWDID");
//      m_fwdElectronEffSFIDLooseFile = electronSFMapFilePath("FWDID");
//      m_fwdElectronEffSFID = setupElectronSFToolWithMap("AsgFwdElectronEfficiencyCorrectionTool_ID",
//                                                        m_fwdElectronEffSFIDFile, "",
//                                                        "Fwd" + m_config->fwdElectronID(), "", "", dataType, "TOTAL", "",
//                                                        "");
//      m_fwdElectronEffSFIDLoose = setupElectronSFToolWithMap("AsgFwdElectronEfficiencyCorrectionTool_IDLoose",
//                                                             m_fwdElectronEffSFIDLooseFile, "",
//                                                             "Fwd" + m_config->fwdElectronIDLoose(), "", "", dataType, "TOTAL", "",
//                                                             "");

      ATH_MSG_INFO("Finished setting up forward Electrons SF tool");
    }


    // Charge ID cannot use maps at the moment so we default to the old method
    if (m_config->useElectronChargeIDSelection()
      && electronIsolation != "PLVTight" && electronIsolation != "PLVLoose"
      && electronIsolationLoose != "PLVTight" && electronIsolationLoose != "PLVLoose"
      && electronIsolation != "PLImprovedTight" && electronIsolation != "PLImprovedVeryTight"
      && electronIsolationLoose != "PLImprovedTight" && electronIsolationLoose != "PLImprovedVeryTight") { // We need to update the implementation according to new recommendations

      ATH_MSG_INFO("Setting up Electrons ChargeID SF tool");
      // Charge ID file (no maps)
      m_electronEffSFChargeIDFile = electronSFFilePath("ChargeID", electronID, electronIsolation);
      if (m_config->applyTightSFsInLooseTree()) // prevent crash on-supported loose electron WPs with ECIDS
        m_electronEffSFChargeIDLooseFile = electronSFFilePath("ChargeID", electronID, electronIsolation);
      else m_electronEffSFChargeIDLooseFile = electronSFFilePath("ChargeID", electronIDLoose, electronIsolationLoose);
      // The tools want the files in vectors: remove this with function
      std::vector<std::string> inChargeID {
        m_electronEffSFChargeIDFile
      };
      std::vector<std::string> inChargeIDLoose {
        m_electronEffSFChargeIDLooseFile
      };
      // Charge Id efficiency scale factor
      m_electronEffSFChargeID = setupElectronSFTool(elSFPrefix + "ChargeID", inChargeID, dataType);
      m_electronEffSFChargeIDLoose = setupElectronSFTool(elSFPrefix + "ChargeIDLoose", inChargeIDLoose, dataType);
    }
    //if(electronIsolation != "PLVTight" && electronIsolation != "PLVLoose" &&
    //   electronIsolation != "PLImprovedTight" && electronIsolation != "PLImprovedVeryTight"){
    //  CP::ElectronChargeEfficiencyCorrectionTool* ChargeMisIDCorrections = new CP::ElectronChargeEfficiencyCorrectionTool("ElectronChargeEfficiencyCorrection");
    //  m_electronEffSFChargeMisIDFile = electronSFFilePath("ChargeMisID", electronID, electronIsolation);
    //  top::check(ChargeMisIDCorrections->setProperty("CorrectionFileName", m_electronEffSFChargeMisIDFile), "Failed to setProperty");
    //  top::check(ChargeMisIDCorrections->initialize(), "Failed to setProperty");
    //}
    //if(electronIsolationLoose != "PLVTight" && electronIsolationLoose != "PLVLoose" &&
    //   electronIsolationLoose != "PLImprovedTight" && electronIsolationLoose != "PLImprovedVeryTight"){
    //  CP::ElectronChargeEfficiencyCorrectionTool* ChargeMisIDCorrectionsLoose = new CP::ElectronChargeEfficiencyCorrectionTool("ElectronChargeEfficiencyCorrectionLoose");
    //  m_electronEffSFChargeMisIDLooseFile = electronSFFilePath("ChargeMisID", electronIDLoose, electronIsolationLoose);
    //  top::check(ChargeMisIDCorrectionsLoose->setProperty("CorrectionFileName", m_electronEffSFChargeMisIDLooseFile), "Failed to setProperty");
    //  top::check(ChargeMisIDCorrectionsLoose->initialize(), "Failed to setProperty");
    //}
    return StatusCode::SUCCESS;
  }

  IAsgElectronEfficiencyCorrectionTool*
  EgammaCPTools::setupElectronSFTool(const std::string& name, const std::vector<std::string>& file_list,
                                     const int& data_type,
				     const std::string& correlation_model,
				     const std::string& correlationModelEtaBinning,
				     const std::string& correlationModelEtBinning) {
    IAsgElectronEfficiencyCorrectionTool* tool = nullptr;

    if (asg::ToolStore::contains<IAsgElectronEfficiencyCorrectionTool>(name)) {
      tool = asg::ToolStore::get<IAsgElectronEfficiencyCorrectionTool>(name);
    } else {
      if (!file_list.empty()) {  // If the file list is empty do nothing
        tool = new AsgElectronEfficiencyCorrectionTool(name);
        top::check(asg::setProperty(tool, "CorrectionFileNameList", file_list),
                   "Failed to set CorrectionFileNameList to " + name);
        top::check(asg::setProperty(tool, "ForceDataType", data_type),
                   "Failed to set ForceDataType to " + name);
        top::check(asg::setProperty(tool, "CorrelationModel", correlation_model),
                   "Failed to set CorrelationModel to " + name);
        if (correlationModelEtaBinning != "" && correlationModelEtaBinning != "default") this->setCorrelationModelBinning(tool, "UncorrEtaBinsUser", correlationModelEtaBinning);
        if (correlationModelEtBinning != "" && correlationModelEtBinning != "default") this->setCorrelationModelBinning(tool, "UncorrEtBinsUser", correlationModelEtBinning);
        
        top::check(asg::setProperty(tool, "OutputLevel", MSG::INFO), "Failed to set OutputLevel to " + name);
        top::check(tool->initialize(), "Failed to initialize " + name);
      }
    }
    return tool;
  }

  void EgammaCPTools::setCorrelationModelBinning(IAsgElectronEfficiencyCorrectionTool* tool,
                                                 const std::string& binningName, const std::string& binning) {
    std::vector<std::string> tokens;
    top::tokenize(binning, tokens, ":");
    if (tokens.size() < 1) {
      ATH_MSG_ERROR(
        "EgammaCPTools::setupElectronSFToolWithMap, correlation model " << binningName <<
          " binning must be in the form XXX:YYY:WWW:ZZZ...");
    }
    std::vector<float> bins;
    for (unsigned int i = 0; i < tokens.size(); i++) {
      std::string token = tokens[i];
      float value = 0.;
      try{
        value = std::stof(token);
      }
      catch (...) {
        throw std::invalid_argument {
                "EgammaCPTools::setupElectronSFToolWithMap, correlation model " + binningName +
                " binning must be in the for XXX:YYY:WWW:ZZZ, couldn't convert correctly to float"
        };
      }
      bins.push_back(value);
    }
    ATH_MSG_INFO(" ---> electron SF tools will use " << binningName << " bins:");
    for (unsigned int i = 0; i < bins.size(); i++) ATH_MSG_INFO("       " << bins[i]);
    top::check(asg::setProperty(tool, binningName,
                                bins), "Failed to set correlation model " + binningName + " binning to " + binning);
  }

  void EgammaCPTools::setCorrelationModelToys(IAsgElectronEfficiencyCorrectionTool* tool,
                                                 const std::string& ToysName, const int& number) {
    ATH_MSG_INFO(" ---> electron SF tools will use " << ToysName << " :"<< number);
    top::check(asg::setProperty(tool, ToysName,
                                number), "Failed to set correlation model " + ToysName );
  }


IAsgElectronEfficiencyCorrectionTool*
  EgammaCPTools::setupElectronSFToolWithMap(const std::string& name, const std::string& map_path,
                                            const std::string& reco_key, const std::string& ID_key,
                                            const std::string& ISO_key, const std::string& trigger_key,
                                            const int& data_type,
                                            const int& correlationModelNToys,
                                            const int& correlationModelToySeed,
                                            const std::string& correlation_model,
                                            const std::string& correlationModelEtaBinning,
                                            const std::string& correlationModelEtBinning) {

    std::string iso_key = ISO_key;
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/LatestRecommendationsElectronIDRun2#PLV_scale_factors_for_central_el
    // If isolation WP is PLVTight or PLVLoose, switch to no isolation to trick this function.
    if (iso_key == "PLVTight" || iso_key == "PLVLoose") iso_key = "";

    std::string infoStr = "Configuring : name=" + name + " map=" + map_path + " reco_key=" + reco_key + " ID_key=" +
                          ID_key + " iso_key=" + iso_key + " trigger_key=" + trigger_key + "data_type=" +
                          std::to_string(data_type) +
                          " correlation_model=" + correlation_model + " etaBinning=" + correlationModelEtaBinning +
                          " etBinning=" +
                          correlationModelEtBinning;
    ATH_MSG_INFO(infoStr);
    IAsgElectronEfficiencyCorrectionTool* tool = nullptr;
    if (asg::ToolStore::contains<IAsgElectronEfficiencyCorrectionTool>(name)) {
      tool = asg::ToolStore::get<IAsgElectronEfficiencyCorrectionTool>(name);
    } else {
      tool = new AsgElectronEfficiencyCorrectionTool(name);
      if (map_path != "UseEgammaRecommended") {
	// Give the full map path if not using the default one
	top::check(asg::setProperty(tool, "MapFilePath", map_path), "Failed to set MapFilePath to " + name);
      }
      // Set the data type for all tools
      top::check(asg::setProperty(tool, "ForceDataType", data_type), "Failed to set ForceDataType to " + name);
      // Set the correlation model for all tools
      top::check(asg::setProperty(tool, "CorrelationModel",
                                  correlation_model), "Failed to set CorrelationModel to " + name);

      if (correlationModelEtaBinning != "" && correlationModelEtaBinning != "default") this->setCorrelationModelBinning(
          tool, "UncorrEtaBinsUser", correlationModelEtaBinning);
      if (correlationModelEtBinning != "" && correlationModelEtBinning != "default") this->setCorrelationModelBinning(
          tool, "UncorrEtBinsUser", correlationModelEtBinning);

      //MC Toy model variables
      if (correlationModelNToys != 0 ) this->setCorrelationModelToys(
          tool, "NumberOfToys", correlationModelNToys);

      if (correlationModelToySeed != 0) this->setCorrelationModelToys(
          tool, "MCToySeed", correlationModelToySeed);


      // Set the keys which configure the tool options (empty string means we do not include this key)
      if (reco_key != "" && reco_key != "None") {
        ATH_MSG_INFO(" Adding RecoKey    : " + reco_key);
        top::check(asg::setProperty(tool, "RecoKey", reco_key), "Failed to set RecoKey to " + name);
      }
      if (ID_key != "" && ID_key != "None") {
        std::string id_key = mapWorkingPoints(ID_key);
        ATH_MSG_INFO(" Adding IDKey      : " + id_key);
        top::check(asg::setProperty(tool, "IdKey", id_key), "Failed to set IdKey to " + name);
      }
      if (iso_key != "" && iso_key != "None") {
        ATH_MSG_INFO(" Adding IsoKey     : " + iso_key);
        top::check(asg::setProperty(tool, "IsoKey", iso_key), "Failed to set IsoKey to " + name);
      }
      if (trigger_key != "" && trigger_key != "None") {
        ATH_MSG_INFO(" Adding TriggerKey : " + trigger_key);
        top::check(asg::setProperty(tool, "TriggerKey", trigger_key), "Failed to set TriggerKey to " + name);
      }
      // Initialise this tool

      top::check(tool->initialize(), "Failed to initialize " + name);
    }
    return tool;
}


  IAsgElectronEfficiencyCorrectionTool*
  EgammaCPTools::setupElectronSFToolWithMap(const std::string& name, const std::string& map_path,
                                            const std::string& reco_key, const std::string& ID_key,
                                            const std::string& ISO_key, const std::string& trigger_key,
                                            const int& data_type, 
                                            const std::string& correlation_model,
                                            const std::string& correlationModelEtaBinning,
                                            const std::string& correlationModelEtBinning) {
    
    std::string iso_key = ISO_key;
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/LatestRecommendationsElectronIDRun2#PLV_scale_factors_for_central_el
    // If isolation WP is PLVTight or PLVLoose, switch to no isolation to trick this function.
    if (iso_key == "PLVTight" || iso_key == "PLVLoose") iso_key = "";

    // fixes needed for run 2
    if (!m_config->isRun3() && (trigger_key != "" && trigger_key != "None")) {
      if (iso_key == "Tight_VarRad") iso_key = "FCTight";
    }

    if( (iso_key == "PLImprovedTight" || iso_key == "PLImprovedVeryTight") && m_config->useElectronChargeIDSelection() ){
      ATH_MSG_INFO( "ECIDS tool and PLImproved* isolation detected, switching to combined isolation SFs.");
      iso_key += "ECIDS";
    }

    std::string infoStr = "Configuring : name=" + name + " map=" + map_path + " reco_key=" + reco_key + " ID_key=" +
                          ID_key + " iso_key=" + iso_key + " trigger_key=" + trigger_key + "data_type=" +
                          std::to_string(data_type) +
                          " correlation_model=" + correlation_model + " etaBinning=" + correlationModelEtaBinning +
                          " etBinning=" +
                          correlationModelEtBinning;
    ATH_MSG_INFO(infoStr);
    IAsgElectronEfficiencyCorrectionTool* tool = nullptr;
    if (asg::ToolStore::contains<IAsgElectronEfficiencyCorrectionTool>(name)) {
      tool = asg::ToolStore::get<IAsgElectronEfficiencyCorrectionTool>(name);
    } else {
      tool = new AsgElectronEfficiencyCorrectionTool(name);
      if (!m_config->isRun3()) {
        top::check(asg::setProperty(tool, "MapFilePath", "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"), "Failed to set MapFilePath to " + name);
      } else {
        if (map_path != "UseEgammaRecommended") {
          // Give the full map path if not using the default one
          top::check(asg::setProperty(tool, "MapFilePath", map_path), "Failed to set MapFilePath to " + name);
        }
      }
      // Set the data type for all tools
      top::check(asg::setProperty(tool, "ForceDataType", data_type), "Failed to set ForceDataType to " + name);
      // Set the correlation model for all tools
      top::check(asg::setProperty(tool, "CorrelationModel",
                                  correlation_model), "Failed to set CorrelationModel to " + name);

      if (correlationModelEtaBinning != "" && correlationModelEtaBinning != "default") this->setCorrelationModelBinning(
          tool, "UncorrEtaBinsUser", correlationModelEtaBinning);
      if (correlationModelEtBinning != "" && correlationModelEtBinning != "default") this->setCorrelationModelBinning(
          tool, "UncorrEtBinsUser", correlationModelEtBinning);

      // Set the keys which configure the tool options (empty string means we do not include this key)
      if (reco_key != "" && reco_key != "None") {
        ATH_MSG_INFO(" Adding RecoKey    : " + reco_key);
        top::check(asg::setProperty(tool, "RecoKey", reco_key), "Failed to set RecoKey to " + name);
      }
      if (ID_key != "" && ID_key != "None") {
        std::string id_key = mapWorkingPoints(ID_key);
        ATH_MSG_INFO(" Adding IDKey      : " + id_key);
        top::check(asg::setProperty(tool, "IdKey", id_key), "Failed to set IdKey to " + name);
      }
      if (iso_key != "" && iso_key != "None") {
        ATH_MSG_INFO(" Adding IsoKey     : " + iso_key);
        top::check(asg::setProperty(tool, "IsoKey", iso_key), "Failed to set IsoKey to " + name);
      }
      if (trigger_key != "" && trigger_key != "None") {
        ATH_MSG_INFO(" Adding TriggerKey : " + trigger_key);
        top::check(asg::setProperty(tool, "TriggerKey", trigger_key), "Failed to set TriggerKey to " + name);
      }
      top::check(asg::setProperty(tool, "OutputLevel", MSG::INFO), "Failed to set OutputLevel to " + name);
      top::check(tool->initialize(), "Failed to initialize " + name);
    }
    return tool;
  }

  std::string EgammaCPTools::electronSFFilePath(const std::string& type, const std::string& ID,
                                                const std::string& ISO) {
    bool isPliv = (ISO == "PLImprovedTight" || ISO == "PLImprovedVeryTight");
    const std::string el_calib_path = "ElectronEfficiencyCorrection/2015_2025/rel22.2/2022_Summer_Prerecom_v1/";

    std::string file_path;

    if (type == "reco") {
      ATH_MSG_ERROR("Moved to using egamma maps for configuring scale factor tools - electronSFMapFilePath");
    } else if (type == "ID") {
      ATH_MSG_ERROR("Moved to using egamma maps for configuring scale factor tools - electronSFMapFilePath");
    } else if (type == "triggerSF") {
      ATH_MSG_ERROR("Moved to using egamma maps for configuring scale factor tools - electronSFMapFilePath");
    } else if (type == "triggerEff") {
      ATH_MSG_ERROR("Moved to using egamma maps for configuring scale factor tools - electronSFMapFilePath");
    } else if (type == "ChargeID") {
      if (ID != "MediumLLH" && ID != "TightLLH") ATH_MSG_ERROR(
          "The requested ID WP (" + ID +
        ") is not supported for electron ChargeID SFs! Try TightLH or MediumLH instead.");
      if (ISO != "FCTight" && ISO != "Gradient" && ISO != "PLImprovedTight" && ISO != "PLImprovedVeryTight") ATH_MSG_ERROR("The requested ISO WP (" + ISO + ") is not supported for electron ChargeID SFs! Try FCTight, Gradient, or PLImproved(Very)Tight instead.");	
      file_path += (isPliv) ? "isolation/efficiencySF.Isolation." : "additional/efficiencySF.ChargeID.";
      file_path += ID;
      file_path += "_d0z0_v13_";
      if(isPliv) file_path += "isol";
      file_path += ISO;
      file_path += (isPliv) ? "ECIDS.root" : "_ECIDSloose.root";
      file_path = el_calib_path + file_path;
    } else if (type == "PLV") {
      if (ID != "MediumLLH" && ID != "TightLLH")
        ATH_MSG_ERROR(
		      "The requested ID WP (" + ID +
		      ") is not supported for PLV SFs! try TightLH or MediumLH instead."
		      );
      file_path  = "/ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/isolation/";
      file_path += "efficiencySF.Isolation.MediumLHorTightLH_d0z0_v13_isol";
      file_path += ISO;
      if (ISO == "PLVTight" && m_config->useElectronChargeIDSelection()) {
        ATH_MSG_INFO(
		     "ECIDS tool and PLVTight isolation detected, switching to combined isolation SFs."
		     );
        file_path += "ECIDS";
      }
      file_path += ".root";
    } else if (type == "ChargeMisID") {
      // Protect against "None" Iso key
      std::string iso = ISO;
      if (iso == "None") iso = "";
      // Protect against Loose ID + any Iso
      if (ID == "LooseAndBLayerLLH") iso = "";
      file_path = "charge_misID/";
      file_path += "chargeEfficiencySF.";
      file_path += ID;
      file_path += "_d0z0_v13";
      if (iso != "" && iso != "PLVTight" && iso != "PLVLoose" && iso != "PLImprovedTight" && iso != "PLImprovedVeryTight") file_path += "_" + iso;
      if (iso == "PLVTight" || iso == "PLVLoose" || iso == "PLImprovedTight" || iso == "PLImprovedVeryTight") {
        // not supported for now! -> set up a dummy tool and return 1 as SF
        ATH_MSG_WARNING("The requested ISO WP (" + iso + ") is not supported for electron ChargeMisID SFs! Will set up a dummy tool and set the SFs to one.");
      }
      if (m_config->useElectronChargeIDSelection()) {
        if (ID != "MediumLLH" && ID != "TightLLH") ATH_MSG_WARNING("The requested ID WP (" + ID + ") is not supported for electron ECIDS+ChargeMisID SFs! Try TightLH or MediumLH instead. Will now switch to regular ChargeMisID SFs.");
        else if (iso != "FCTight" && iso != "Gradient") ATH_MSG_WARNING("The requested ISO WP (" + iso + ") is not supported for electron ECIDS+ChargeMisID SFs! Try FCTight or Gradient instead. Will now switch to regular ChargeMisID SFs.");
        else file_path += "_ECIDSloose";
      }
      file_path += ".root";
      file_path = el_calib_path + file_path;
    } else if (type == "EXPID"){
      file_path = m_config->electronIDSFFilePath();
    } else if (type == "EXPID_Loose"){
      file_path = m_config->electronIDSFFileLoosePath();
    } else {
      ATH_MSG_ERROR("Unknown electron SF type");
    }
    return PathResolverFindCalibFile(file_path);
  }

  std::string EgammaCPTools::electronSFMapFilePath(const std::string& type) {
    // Store here the paths to maps which may be updated with new recommendations
    // Currently can use maps for reco, id, iso, trigger but not ChargeID
    const std::string el_calib_path = "UseEgammaRecommended";

    if (type == "FWDID") {
      return PathResolverFindCalibFile("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/map3.txt");
    }

    return el_calib_path;
  }

  std::string EgammaCPTools::mapWorkingPoints(const std::string& type) {
    // Ian Connelly - 27 Sept 2017
    // When moving to the maps, the working points are converted to a nicer format
    // We will provide a mapping from the names used by analysers and the map WP names to prevent cutfiles breaking
    // See :
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/XAODElectronEfficiencyCorrectionTool#Configuration_of_the_tool_using
    std::string working_point = "";
    // ID
    if (type == "LooseAndBLayerLLH" || type == "LooseBLayer") {
      working_point = "LooseBLayer";
    }
    if (type == "MediumLLH" || type == "Medium") {
      working_point = "Medium";
    }
    if (type == "TightLLH" || type == "Tight") {
      working_point = "Tight";
    }
    if (type == "FwdLoose" || type == "FwdMedium" || type == "FwdTight") working_point = type;

    if (type.find("Pflow") != std::string::npos) {
      ATH_MSG_WARNING("You selected a Pflow isolation WP for at least one of your electron collections - BE WARNED THAT THESE ARE NOT YET READY TO BE RELEASED FOR USE IN PHYSICS ANALYSES AND OF COURSE DON'T HAVE ASSOCIATED SCALE FACTORS YET!!!");
      if (type == "PflowLoose") working_point = "FCLoose";
      if (type == "PflowTight") working_point = "FCTight";
    }
    if (type == "Tight_VarRad") working_point = "Tight_VarRad";
    if (type == "Loose_VarRad") working_point = "Loose_VarRad";
    if (type == "Tight") working_point = "FCTight";
    if (type == "Loose") working_point = "FCLoose";
    if (type == "HighPtCaloOnly") working_point = "FCHighPtCaloOnly";
    if (type == "TightTrackOnly") {
      ATH_MSG_WARNING("You selected the TightTrackOnly isolation WP for at least one of your electron collections - BE WARNED THAT THESE ARE NOT YET READY TO BE RELEASED FOR USE IN PHYSICS ANALYSES AND OF COURSE DON'T HAVE ASSOCIATED SCALE FACTORS YET!!! Setting to \"Gradient\" SFs to allow the code to run");
      working_point = "Gradient";
    }
    if (type == "TightTrackOnly_FixedRad") {
      ATH_MSG_WARNING("You selected the TightTrackOnly_FixedRad isolation WP for at least one of your electron collections - BE WARNED THAT THESE ARE NOT YET READY TO BE RELEASED FOR USE IN PHYSICS ANALYSES AND OF COURSE DON'T HAVE ASSOCIATED SCALE FACTORS YET!!! Setting to \"Gradient\" SFs to allow the code to run");
      working_point = "Gradient";
    }
    if (type == "FCTight" || type == "FCLoose" || type == "FCHighPtCaloOnly" || type == "Gradient" || type == "PLVTight" || type == "PLVLoose" || type == "PLImprovedTight" || type == "PLImprovedVeryTight") working_point = type;

    return working_point;
  }
}  // namespace top

