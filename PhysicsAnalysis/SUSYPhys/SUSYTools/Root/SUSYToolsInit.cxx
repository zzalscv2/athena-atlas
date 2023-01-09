/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "SUSYTools/SUSYObjDef_xAOD.h"

using namespace ST;

// For the data types to be used in configuring tools
#include "PATCore/PATCoreEnums.h"

// For the tau tool initialization
#include "TauAnalysisTools/Enums.h"

// For the struct needed for OR init
#include "AssociationUtils/OverlapRemovalInit.h"

// Abstract interface classes
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

#include "JetInterface/IJetSelector.h"
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCPInterfaces/ICPJetUncertaintiesTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "JetInterface/IJetModifier.h"
#include "JetAnalysisInterfaces/IJetJvtEfficiency.h"
#include "JetAnalysisInterfaces/IJetSelectorTool.h"

#include "AsgAnalysisInterfaces/IEfficiencyScaleFactorTool.h"
#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgDeadHVCellRemovalTool.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "EgammaAnalysisInterfaces/IEGammaAmbiguityTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"

#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonEfficiencyScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"

#include "TauAnalysisTools/ITauSelectionTool.h"
#include "TauAnalysisTools/ITauSmearingTool.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"
#include "TauAnalysisTools/ITauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/ITauOverlappingElectronLLHDecorator.h"
#include "tauRecTools/ITauToolBase.h"

#include "IsolationSelection/IIsolationSelectionTool.h"
#include "IsolationSelection/IIsolationLowPtPLVTool.h"
#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "IsolationSelection/IIsolationCloseByCorrectionTool.h"

#include "METInterface/IMETMaker.h"
#include "METInterface/IMETSystematicsTool.h"
#include "METInterface/IMETSignificance.h"

#include "TrigConfInterfaces/ITrigConfigTool.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "TriggerAnalysisInterfaces/ITrigGlobalEfficiencyCorrectionTool.h"
// Can't use the abstract interface for this one (see header comment)
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "PATInterfaces/IWeightTool.h"
#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"
#include "PathResolver/PathResolver.h"
#include "AssociationUtils/IOverlapRemovalTool.h"
#include "BoostedJetTaggers/SmoothedWZTagger.h"
#include "BoostedJetTaggers/JSSWTopTaggerDNN.h"
#include "ParticleJetTools/JetTruthLabelingTool.h"


#define CONFIG_EG_EFF_TOOL( TOOLHANDLE, TOOLNAME, CORRFILE )                \
  if( !TOOLHANDLE.isUserConfigured() ) {                                \
    TOOLHANDLE.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+TOOLNAME); \
    std::vector< std::string > corrFileNameList = {CORRFILE}; \
    ATH_CHECK( TOOLHANDLE.setProperty("CorrectionFileNameList", corrFileNameList) ); \
    if(!isData())                                                        \
      ATH_CHECK (TOOLHANDLE.setProperty("ForceDataType", (int) data_type) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("CorrelationModel", m_EG_corrModel) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("OutputLevel", this->msg().level()) ); \
    ATH_CHECK( TOOLHANDLE.initialize() );                                \
  } else ATH_CHECK(TOOLHANDLE.retrieve());

#define CONFIG_EG_EFF_TOOL_KEY( TOOLHANDLE, TOOLNAME, KEYNAME, KEY )        \
  if( !TOOLHANDLE.isUserConfigured() ) {                                \
    TOOLHANDLE.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+TOOLNAME); \
    std::cout << "Will now set key \"" << KEYNAME << "\" to value \"" << KEY << "\" when configuring an AsgElectronEfficiencyCorrectionTool" << std::endl; \
    ATH_CHECK( TOOLHANDLE.setProperty(KEYNAME, KEY) );                  \
    if(!isData())                                                        \
      ATH_CHECK (TOOLHANDLE.setProperty("ForceDataType", (int) data_type) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("CorrelationModel", m_EG_corrModel) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("OutputLevel", this->msg().level()) ); \
    ATH_CHECK( TOOLHANDLE.initialize() );                                \
  } else if (!isData()) ATH_CHECK(TOOLHANDLE.retrieve());

StatusCode SUSYObjDef_xAOD::SUSYToolsInit()
{
  if (m_dataSource < 0) {
    ATH_MSG_FATAL("Data source incorrectly configured!!");
    ATH_MSG_FATAL("You must set the DataSource property to Data, FullSim or AtlfastII !!");
    ATH_MSG_FATAL("Expect segfaults if you're not checking status codes, which you should be !!");
    return StatusCode::FAILURE;
  }

  if (m_subtool_init) {
    ATH_MSG_INFO("SUSYTools subtools already created. Ignoring this call.");
    ATH_MSG_INFO("Note: No longer necessary to explicitly call SUSYToolsInit. Will avoid creating tools again.");
    return StatusCode::SUCCESS;
  }

  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise PileupReweighting Tool

  if (!m_prwTool.isUserConfigured()) {
    ATH_MSG_DEBUG("Will now init the PRW tool");
    std::vector<std::string> file_conf;
    for (UInt_t i = 0; i < m_prwConfFiles.size(); i++) {
      file_conf.push_back(m_prwConfFiles.at(i));
    }

    std::vector<std::string> file_ilumi;
    for (UInt_t i = 0; i < m_prwLcalcFiles.size(); i++) {
      ATH_MSG_INFO("Adding ilumicalc file: " << m_prwLcalcFiles.at(i));
      file_ilumi.push_back(m_prwLcalcFiles.at(i));
    }

    m_prwTool.setTypeAndName("CP::PileupReweightingTool/PrwTool");
    ATH_CHECK( m_prwTool.setProperty("ConfigFiles", file_conf) );
    ATH_CHECK( m_prwTool.setProperty("LumiCalcFiles", file_ilumi) );
    ATH_CHECK( m_prwTool.setProperty("DataScaleFactor",     m_prwDataSF) ); // 1./1.03 -> default for mc16, see: https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ExtendedPileupReweighting#Tool_Properties
    ATH_CHECK( m_prwTool.setProperty("DataScaleFactorUP",   m_prwDataSF_UP) ); // 1. -> old value (mc15), as the one for mc16 is still missing
    ATH_CHECK( m_prwTool.setProperty("DataScaleFactorDOWN", m_prwDataSF_DW) ); // 1./1.18 -> old value (mc15), as the one for mc16 is still missing
    ATH_CHECK( m_prwTool.setProperty("TrigDecisionTool", m_trigDecTool.getHandle()) );
    ATH_CHECK( m_prwTool.setProperty("UseRunDependentPrescaleWeight", m_runDepPrescaleWeightPRW) );
    ATH_CHECK( m_prwTool.setProperty("OutputLevel", MSG::WARNING) );
    ATH_CHECK( m_prwTool.retrieve() );
  } else {
    ATH_MSG_INFO("Using user-configured PRW tool");
    ATH_CHECK( m_prwTool.retrieve() );
  }
  std::string toolName; // to be used for tool init below, keeping explicit string constants a minimum /CO

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise jet calibration tool

  // pick the right config file for the JES tool : https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR21
  std::string jetname("AntiKt4" + xAOD::JetInput::typeName(xAOD::JetInput::Type(m_jetInputType)));
  std::string jetcoll(jetname + "Jets");

  if (!m_jetCalibTool.isUserConfigured()) {
    toolName = "JetCalibTool_" + jetname;
    m_jetCalibTool.setTypeAndName("JetCalibrationTool/"+toolName);
    std::string JES_config_file, calibseq;

    if (m_jetInputType != xAOD::JetInput::EMTopo && m_jetInputType != xAOD::JetInput::EMPFlow) {
      ATH_MSG_ERROR("Unknown (unsupported) jet collection is used, (m_jetInputType = " << m_jetInputType << ")");
      return StatusCode::FAILURE;
    }

    std::string JESconfig = isAtlfast() ? m_jesConfigAFII : m_jesConfig;
    calibseq = m_jesCalibSeq;

    JES_config_file = JESconfig;
    if (m_jetInputType == xAOD::JetInput::EMPFlow) {
      JES_config_file = TString(JESconfig).ReplaceAll("_EMTopo_","_PFlow_").Data();
    }

    // remove Insitu if it's in the string if not data, and add _Smear
    if (!isData()) {
      std::string insitu("_Insitu");
      auto found = calibseq.find(insitu);
      if(found != std::string::npos){
        calibseq.erase(found, insitu.length());
        calibseq.append("_Smear");
      }
    }

    // JMS calibration (if requested)
    if (m_JMScalib){ 
      if (isAtlfast()) {
        ATH_MSG_ERROR("JMS calibration is not supported for AF-II samples. Please modify your settings.");
        return StatusCode::FAILURE;
      }

      std::string JMSconfig = isData() ? m_jesConfigJMSData : m_jesConfigJMS;
      JES_config_file = JMSconfig;
      if (m_jetInputType == xAOD::JetInput::EMPFlow) {  
        JES_config_file = TString(JMSconfig).ReplaceAll("_EMTopo_","_PFlow_").Data();
      }

      calibseq = m_jesCalibSeqJMS;
      if (isData()) calibseq.append("_JMS_Insitu");
      else calibseq.append("_Smear_JMS");
    }

    // now instantiate the tool
    ATH_CHECK( m_jetCalibTool.setProperty("JetCollection", jetname) );
    ATH_CHECK( m_jetCalibTool.setProperty("ConfigFile", JES_config_file) );
    ATH_CHECK( m_jetCalibTool.setProperty("CalibSequence", calibseq) );
    ATH_CHECK( m_jetCalibTool.setProperty("IsData", isData()) );
    ATH_CHECK( m_jetCalibTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetCalibTool.retrieve() );
  }

  else ATH_CHECK(m_jetCalibTool.retrieve());
  //same for fat groomed jets
  std::string fatjetcoll(m_fatJets);
  if (fatjetcoll.size()>3) fatjetcoll = fatjetcoll.substr(0,fatjetcoll.size()-4); //remove "Jets" suffix
  if (!m_jetFatCalibTool.isUserConfigured() && !m_fatJets.empty()) {
    toolName = "JetFatCalibTool_" + m_fatJets;
    m_jetFatCalibTool.setTypeAndName("JetCalibrationTool/"+toolName);

    std::string jesConfigFat = m_jesConfigFat;
    std::string jesCalibSeqFat = m_jesCalibSeqFat;
    // add Insitu if data
    if (isData()) {
      jesConfigFat = m_jesConfigFatData;
      jesCalibSeqFat += "_Insitu_InsituCombinedMass";
    }

    // now instantiate the tool
    ATH_CHECK( m_jetFatCalibTool.setProperty("JetCollection", fatjetcoll) );
    ATH_CHECK( m_jetFatCalibTool.setProperty("ConfigFile", jesConfigFat) );
    ATH_CHECK( m_jetFatCalibTool.setProperty("CalibSequence", jesCalibSeqFat) );
    // always set to false : https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR21
    ATH_CHECK( m_jetFatCalibTool.setProperty("IsData", isData()) );
    ATH_CHECK( m_jetFatCalibTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetFatCalibTool.retrieve() );
  } else if (m_jetFatCalibTool.isUserConfigured()) ATH_CHECK(m_jetFatCalibTool.retrieve());

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise Boson taggers: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BoostedJetTaggingRecommendationFullRun2#Higgs_taggers
  if (!m_WTaggerTool.isUserConfigured() && !m_WtagConfig.empty()) {
    m_WTaggerTool.setTypeAndName("SmoothedWZTagger/WTagger");
    ATH_CHECK( m_WTaggerTool.setProperty("ConfigFile", m_WtagConfig) );
    ATH_CHECK( m_WTaggerTool.setProperty("CalibArea", m_WZTaggerCalibArea) );
    ATH_CHECK( m_WTaggerTool.setProperty("IsMC",!isData()));
    ATH_CHECK( m_WTaggerTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container;
    ATH_CHECK( m_WTaggerTool.setProperty("TruthTopQuarkContainerName", "TruthTop") );  // Set this if you are using a TRUTH3 style truth boson container;
    ATH_CHECK( m_WTaggerTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_WTaggerTool.retrieve() );
  } else if (m_WTaggerTool.isUserConfigured()) ATH_CHECK(m_WTaggerTool.retrieve());

  if (!m_ZTaggerTool.isUserConfigured() && !m_ZtagConfig.empty()) {
    m_ZTaggerTool.setTypeAndName("SmoothedWZTagger/ZTagger");
    ATH_CHECK( m_ZTaggerTool.setProperty("ConfigFile", m_ZtagConfig) );
    ATH_CHECK( m_ZTaggerTool.setProperty("CalibArea", m_WZTaggerCalibArea) );
    ATH_CHECK( m_ZTaggerTool.setProperty("IsMC",!isData() ));
    ATH_CHECK( m_ZTaggerTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container;
    ATH_CHECK( m_ZTaggerTool.setProperty("TruthTopQuarkContainerName", "TruthTop") );  // Set this if you are using a TRUTH3 style truth boson container;
    ATH_CHECK( m_ZTaggerTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_ZTaggerTool.retrieve() );
  } else if (m_ZTaggerTool.isUserConfigured()) ATH_CHECK(m_ZTaggerTool.retrieve());

  if (!m_TopTaggerTool.isUserConfigured() && !m_ToptagConfig.empty()) {
    m_TopTaggerTool.setTypeAndName("JSSWTopTaggerDNN/TopTagger");
    ATH_CHECK( m_TopTaggerTool.setProperty("ConfigFile", m_ToptagConfig) );
    ATH_CHECK( m_TopTaggerTool.setProperty("CalibArea", m_TopTaggerCalibArea) );
    ATH_CHECK( m_TopTaggerTool.setProperty("IsMC",!isData() ));
    ATH_CHECK( m_TopTaggerTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container;
    ATH_CHECK( m_TopTaggerTool.setProperty("TruthTopQuarkContainerName", "TruthTop") );  // Set this if you are using a TRUTH3 style truth boson container
    ATH_CHECK( m_TopTaggerTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_TopTaggerTool.retrieve() );
  } else if (m_TopTaggerTool.isUserConfigured()) ATH_CHECK(m_TopTaggerTool.retrieve());

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise JetTruthLabelingTool: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2019LargeR#AnalysisBase_21_2_114_and_newer
  if (!m_jetTruthLabelingTool.isUserConfigured()) {
    m_jetTruthLabelingTool.setTypeAndName("JetTruthLabelingTool/ST_JetTruthLabelingTool");
    ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthLabelName", "R10TruthLabel_R21Consolidated") );
    ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container
    ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthTopQuarkContainerName", "TruthTop") ); // Set this if you are using a TRUTH3 style truth top quark container

    ATH_CHECK( m_jetTruthLabelingTool.setProperty("UseTRUTH3", m_useTRUTH3) );                 // Set this to false only if you have the FULL !TruthParticles container in your input file
    //ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthParticleContainerName", "") );
    ATH_CHECK( m_jetTruthLabelingTool.retrieve() );
  } else if (m_jetTruthLabelingTool.isUserConfigured()) ATH_CHECK(m_jetTruthLabelingTool.retrieve());

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise jet uncertainty tool
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2018SmallR
  ATH_MSG_INFO("Set up Jet Uncertainty tool...");

  if (!m_jetUncertaintiesTool.isUserConfigured()) {
    std::string jetdef("AntiKt4" + xAOD::JetInput::typeName(xAOD::JetInput::Type(m_jetInputType)));

    if(jetdef != "AntiKt4EMTopo" && jetdef !="AntiKt4EMPFlow"){
      ATH_MSG_WARNING("Jet Uncertaintes recommendations only exist for EMTopo and PFlow jets, falling back to AntiKt4EMTopo");
      jetdef = "AntiKt4EMTopo";
    }
    toolName = "JetUncertaintiesTool_" + jetdef;

    m_jetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);


    ATH_CHECK( m_jetUncertaintiesTool.setProperty("JetDefinition", jetdef) );
    ATH_CHECK( m_jetUncertaintiesTool.setProperty("MCType", isAtlfast() ? "AFII" : "MC16") );
    ATH_CHECK( m_jetUncertaintiesTool.setProperty("IsData", false) ); // Never use the PDSmearing for the nominal tool. 
    ATH_CHECK( m_jetUncertaintiesTool.setProperty("ConfigFile", m_jetUncertaintiesConfig) );
    if(m_jetUncertaintiesAnalysisFile!="default") ATH_CHECK( m_jetUncertaintiesTool.setProperty("AnalysisFile", m_jetUncertaintiesAnalysisFile) );
    if (m_jetUncertaintiesCalibArea != "default") ATH_CHECK( m_jetUncertaintiesTool.setProperty("CalibArea", m_jetUncertaintiesCalibArea) );
    ATH_CHECK( m_jetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetUncertaintiesTool.retrieve() );
  } else  ATH_CHECK( m_jetUncertaintiesTool.retrieve() );
  
  ATH_MSG_INFO("Set up Jet PD Smear Uncertainty tool...");
  
  if (!m_jetUncertaintiesPDSmearTool.isUserConfigured() && m_jetUncertaintiesPDsmearing == true) {
    std::string jetdef("AntiKt4" + xAOD::JetInput::typeName(xAOD::JetInput::Type(m_jetInputType)));
    
    if(jetdef != "AntiKt4EMTopo" && jetdef !="AntiKt4EMPFlow"){
      ATH_MSG_WARNING("Jet Uncertaintes recommendations only exist for EMTopo and PFlow jets, falling back to AntiKt4EMTopo");
      jetdef = "AntiKt4EMTopo";
    }
    toolName = "JetUncertaintiesPDSmearTool_" + jetdef;
    
    m_jetUncertaintiesPDSmearTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
    
    // If, for some reason, you're trying to use the PDSmear, with the reduced set return an error (you shouldn't do this, you're just going to duplicate the SimpleJER results. 
    bool JERUncPDsmearing = isData() ? isData() : m_jetUncertaintiesPDsmearing;
    if (m_jetUncertaintiesConfig.find("SimpleJER") != std::string::npos && JERUncPDsmearing){
      ATH_MSG_ERROR("You are trying to use the SimpleJER set, with PDsmearing. There is no functionality for this. Please fix your config file. Either run with PDSmear set to false, or run with the AllJER or FullJER sets.");
      return StatusCode::FAILURE;
    }
    ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("JetDefinition", jetdef) );
    ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("MCType", isAtlfast() ? "AFII" : "MC16") );
    ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("IsData", true) ); // Set to True by default for PDSmear-named tool. 
    ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("ConfigFile", m_jetUncertaintiesConfig) );
    if (m_jetUncertaintiesCalibArea != "default") ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("CalibArea", m_jetUncertaintiesCalibArea) );
    ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetUncertaintiesPDSmearTool.retrieve() );
  } else{
    ATH_MSG_DEBUG("Do not retrieve the jet PD Smearing tool if it is not configured");
    //ATH_CHECK( m_jetUncertaintiesPDSmearTool.retrieve() );
  }

  ATH_MSG_INFO("Set up FatJet Uncertainty tool if using...");
  // Initialise jet uncertainty tool for fat jets
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2019LargeR
  if (!m_fatjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_fatJetUncConfig.empty()) {

    toolName = "JetUncertaintiesTool_" + m_fatJets;
    m_fatjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);

    ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
    ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("MCType", "MC16") );
    ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("IsData", isData()) );
    ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("ConfigFile", m_fatJetUncConfig) );
    if (m_jetUncertaintiesCalibArea != "default") ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("CalibArea", m_jetUncertaintiesCalibArea) );

    //Restrict variables to be shifted if (required)
    if( m_fatJetUncVars != "default" ){
      std::vector<std::string> shift_vars = {};

      std::string temp(m_fatJetUncVars);
      do {
        auto pos = temp.find(",");
        shift_vars.push_back(temp.substr(0, pos));
        if (pos == std::string::npos)
          temp = "";
        else
          temp = temp.substr(pos + 1);

      }
      while (!temp.empty() );

      ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("VariablesToShift", shift_vars) );
    }

    ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_fatjetUncertaintiesTool.retrieve() );
  } else if (m_fatjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_fatjetUncertaintiesTool.retrieve());


  ATH_MSG_INFO("Set up FatJet tagger Uncertainty tool if using...");
  // Initialise jet uncertainty tool for fat jets
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2019LargeR
  if (!m_WTagjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_WtagConfig.empty() && !m_WTagUncConfig.empty()) {
    
    toolName = "WTagJetUncertaintiesTool_" + m_fatJets;
    m_WTagjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
    ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
    ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("MCType", "MC16") );
    ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("IsData", isData()) );
    ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("ConfigFile", "rel21/Fall2020/"+m_WTagUncConfig) );
    ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_WTagjetUncertaintiesTool.retrieve() );
  } else if (m_WTagjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_WTagjetUncertaintiesTool.retrieve());
  
  if (!m_ZTagjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_ZtagConfig.empty() && !m_ZTagUncConfig.empty()) {
    
    toolName = "ZTagJetUncertaintiesTool_" + m_fatJets;
    m_ZTagjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
    ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
    ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("ConfigFile", "rel21/Fall2020/"+m_ZTagUncConfig) );
    ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("MCType", "MC16") );
    ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("IsData", isData()) );
    ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_ZTagjetUncertaintiesTool.retrieve() );
  } else if (m_ZTagjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_ZTagjetUncertaintiesTool.retrieve());

  if (!m_TopTagjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_ToptagConfig.empty() && !m_TopTagUncConfig.empty()) {
   
    toolName = "TopTagJetUncertaintiesTool_" + m_fatJets;
    m_TopTagjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
    ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
    ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("MCType", "MC16") );
    ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("IsData", isData()) );
    ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("ConfigFile", "rel21/Fall2020/"+m_TopTagUncConfig) );  
    ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_TopTagjetUncertaintiesTool.retrieve() );
  } else if (m_TopTagjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_TopTagjetUncertaintiesTool.retrieve());
  

  // tagger SF and uncertainties
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BoostedJetTaggingRecommendationFullRun2
  // To be implemented here

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise jet cleaning tools

  if (m_badJetCut!="" && !m_jetCleaningTool.isUserConfigured()) {
    toolName = "JetCleaningTool";
    m_jetCleaningTool.setTypeAndName("JetCleaningTool/"+toolName);
    ATH_CHECK( m_jetCleaningTool.setProperty("CutLevel", m_badJetCut) );
    ATH_CHECK( m_jetCleaningTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetCleaningTool.retrieve() );
  } else if (m_jetCleaningTool.isUserConfigured()) ATH_CHECK( m_jetCleaningTool.retrieve() );


  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise JVT tool

  if (!m_jetJvtUpdateTool.isUserConfigured()) {
    toolName = "JetVertexTaggerTool";
    m_jetJvtUpdateTool.setTypeAndName("JetVertexTaggerTool/"+toolName);
    ATH_CHECK( m_jetJvtUpdateTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetJvtUpdateTool.retrieve() );
  } else  ATH_CHECK( m_jetJvtUpdateTool.retrieve() );
 

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise jet JVT efficiency tool (scale factors)

  m_applyJVTCut = !m_JvtWP.empty();
  if (!m_jetJvtEfficiencyTool.isUserConfigured() && m_applyJVTCut) {
    toolName = "JVTEfficiencyTool";
    m_jetJvtEfficiencyTool.setTypeAndName("CP::JetJvtEfficiency/"+toolName);

    // build SFFile path with folder name from config
    m_JvtConfig_SFFile = "JetJvtEfficiency/" + m_JvtConfig; 
    if (m_jetInputType == xAOD::JetInput::EMTopo) { m_JvtConfig_SFFile += "JvtSFFile_EMTopoJets.root"; }
    else if (m_jetInputType == xAOD::JetInput::LCTopo) { m_JvtConfig_SFFile += "JvtSFFile_LC.root"; }
    else if (m_jetInputType == xAOD::JetInput::EMPFlow) { m_JvtConfig_SFFile += "JvtSFFile_EMPFlowJets.root"; }
    else {
      ATH_MSG_ERROR("Cannot configure JVT uncertainties for unsupported jet input type (neither EM nor LC)");
      return StatusCode::FAILURE;
    }
    
    ATH_CHECK( m_jetJvtEfficiencyTool.setProperty("WorkingPoint", m_JvtWP) );
    ATH_CHECK( m_jetJvtEfficiencyTool.setProperty("MaxPtForJvt", m_JvtPtMax) );
    ATH_CHECK( m_jetJvtEfficiencyTool.setProperty("ScaleFactorDecorationName", "jvtscalefact") ); // set decoration name
    ATH_CHECK( m_jetJvtEfficiencyTool.setProperty("SFFile", m_JvtConfig_SFFile) );
    ATH_CHECK( m_jetJvtEfficiencyTool.setProperty("TruthJetContainerName", m_defaultTruthJets ) );
    ATH_CHECK( m_jetJvtEfficiencyTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetJvtEfficiencyTool.retrieve() );
  } else if (m_jetJvtEfficiencyTool.isUserConfigured()) ATH_CHECK( m_jetJvtEfficiencyTool.retrieve() );
 

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise FwdJVT tool

  if (!m_jetFwdJvtTool.isUserConfigured()) {
    toolName = m_doFwdJVT ? m_metJetSelection+"_fJVT" : m_metJetSelection+"_NOfJVT";
    m_jetFwdJvtTool.setTypeAndName("JetForwardJvtTool/FJVTTool_"+toolName);

    ATH_CHECK( m_jetFwdJvtTool.setProperty("OutputDec", "passFJvt") ); //Output decoration
    ATH_CHECK( m_jetFwdJvtTool.setProperty("UseTightOP", (m_fJvtWP=="Tight")) );
    ATH_CHECK( m_jetFwdJvtTool.setProperty("ForwardMaxPt", m_fJvtPtMax) ); // Max Pt to define fwdJets for JVT
    ATH_CHECK( m_jetFwdJvtTool.setProperty("EtaThresh", m_fJvtEtaMin) );   // Eta dividing central from forward jets
    ATH_CHECK( m_jetFwdJvtTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetFwdJvtTool.retrieve() );
  } else ATH_CHECK( m_jetFwdJvtTool.retrieve() );
 

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise jet FwdJVT efficiency tool for scale factors

  if (!m_jetFwdJvtEfficiencyTool.isUserConfigured()) {
    toolName = m_doFwdJVT ? m_metJetSelection+"_fJVT" : m_metJetSelection+"_NOfJVT";
    m_jetFwdJvtEfficiencyTool.setTypeAndName("CP::JetJvtEfficiency/FJVTEfficiencyTool_"+toolName);

    // build SFFile path with folder name from config
    m_fJvtConfig_SFFile = "JetJvtEfficiency/" + m_fJvtConfig; 
    if (m_jetInputType == xAOD::JetInput::EMTopo) { m_fJvtConfig_SFFile += "fJvtSFFile.EMtopo.root"; }
    else if (m_jetInputType == xAOD::JetInput::EMPFlow) { m_fJvtConfig_SFFile += "fJvtSFFile.EMPFlow.root"; }
    else {
      ATH_MSG_ERROR("Cannot configure fJVT uncertainties for unsupported jet input type (neither EMTopo nor EMPFlow)");
      return StatusCode::FAILURE;
    }

    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("JetfJvtMomentName", "passFJvt") );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("ScaleFactorDecorationName", "fJVTSF") ); // set decoration name
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("WorkingPoint", m_fJvtWP) );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("MaxPtForJvt", m_fJvtPtMax) );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("UseMuSFFormat", true) );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("SFFile", m_fJvtConfig_SFFile) );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("TruthJetContainerName", m_defaultTruthJets ) );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_jetFwdJvtEfficiencyTool.retrieve() );
  } else  ATH_CHECK( m_jetFwdJvtEfficiencyTool.retrieve() );
  

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Check muon baseline ID

  std::string muQualBaseline = "";
  switch (m_muIdBaseline) {
  case (int)xAOD::Muon::Quality(xAOD::Muon::VeryLoose):  muQualBaseline = "VeryLoose";
    ATH_MSG_WARNING("No muon scale factors are available for VeryLoose working point.");
    break;
  case (int)xAOD::Muon::Quality(xAOD::Muon::Loose):  muQualBaseline = "Loose";  break;
  case (int)xAOD::Muon::Quality(xAOD::Muon::Medium): muQualBaseline = "Medium"; break;
  case (int)xAOD::Muon::Quality(xAOD::Muon::Tight):  muQualBaseline = "Tight";  break;
  case 4:  muQualBaseline = "HighPt";  break;
  case 5:  muQualBaseline = "LowPt";  break;
  case 6:  muQualBaseline = "LowPtMVA"; break;
  case 7:  muQualBaseline = "HighPt3Layers"; break;
  default:
    ATH_MSG_ERROR("Invalid muon working point provided: " << m_muIdBaseline << ". Cannot initialise!");
    return StatusCode::FAILURE;
    break;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise muon calibration tool
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MuonMomentumCorrectionsSubgroup#CP_MuonCalibrationAndSmearingToo

  if (!m_muonCalibrationAndSmearingTool.isUserConfigured()) {
    m_muonCalibrationAndSmearingTool.setTypeAndName("CP::MuonCalibrationPeriodTool/MuonCalibrationAndSmearingTool");
    ATH_CHECK( m_muonCalibrationAndSmearingTool.setProperty("calibrationMode", m_muCalibrationMode) );
    ATH_CHECK( m_muonCalibrationAndSmearingTool.setProperty("OutputLevel", this->msg().level()) );
    int IdBaselineInt = m_muIdBaseline;
    if (IdBaselineInt == 4) {
      ATH_CHECK( m_muonCalibrationAndSmearingTool.setProperty("do2StationsHighPt", true) );
    }
    ATH_CHECK( m_muonCalibrationAndSmearingTool.setProperty("doExtraSmearing", m_muHighPtExtraSmear) );
    ATH_CHECK( m_muonCalibrationAndSmearingTool.retrieve() );
  } else  ATH_CHECK( m_muonCalibrationAndSmearingTool.retrieve() );

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise muon selection tool

  if (!m_muonSelectionToolBaseline.isUserConfigured()) {
    toolName = "MuonSelectionTool_Baseline_" + muQualBaseline;
    m_muonSelectionToolBaseline.setTypeAndName("CP::MuonSelectionTool/"+toolName);
    
    if (m_muBaselineEta<m_muEta){  // Test for inconsistent configuration
      ATH_MSG_ERROR( "Requested a baseline eta cut for muons (" << m_muBaselineEta <<
                     ") that is tighter than the signal cut (" << m_muEta << ").  Please check your config." );
      return StatusCode::FAILURE;
    }
    int IdBaselineInt = m_muIdBaseline;
    ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "MaxEta", m_muBaselineEta) );
    if (IdBaselineInt == 6){
        ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "MuQuality", 5 ) );
        ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "UseMVALowPt", true));
    } else if (IdBaselineInt == 7){
        ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "MuQuality", 4 ) );
        ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "Use2stationMuonsHighPt", false));        
    } else ATH_CHECK(m_muonSelectionToolBaseline.setProperty( "MuQuality", m_muIdBaseline ));
    ATH_CHECK( m_muonSelectionToolBaseline.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonSelectionToolBaseline.retrieve() );
  } else ATH_CHECK( m_muonSelectionToolBaseline.retrieve() );
 

  std::string muQual = "";
  switch (m_muId) {
  case (int)xAOD::Muon::Quality(xAOD::Muon::VeryLoose):  muQual = "VeryLoose";
    ATH_MSG_WARNING("No muon scale factors are available for VeryLoose working point.");
    break;
  case (int)xAOD::Muon::Quality(xAOD::Muon::Loose):  muQual = "Loose";  break;
  case (int)xAOD::Muon::Quality(xAOD::Muon::Medium): muQual = "Medium"; break;
  case (int)xAOD::Muon::Quality(xAOD::Muon::Tight):  muQual = "Tight";  break;
  case 4:  muQual = "HighPt";  break;
  case 5:  muQual = "LowPt";  break;
  case 6:  muQual = "LowPtMVA"; break;
  case 7:  muQual = "HighPt3Layers"; break;
  default:
    ATH_MSG_ERROR("Invalid muon working point provided: " << m_muId << ". Cannot initialise!");
    return StatusCode::FAILURE;
    break;
  }

  if (!m_muonSelectionTool.isUserConfigured()) {
    toolName = "MuonSelectionTool_" + muQual;
    m_muonSelectionTool.setTypeAndName("CP::MuonSelectionTool/"+toolName);
    ATH_CHECK( m_muonSelectionTool.setProperty( "MaxEta", m_muEta) );
    int IdInt = m_muId;
    if (IdInt == 6){
        ATH_CHECK( m_muonSelectionTool.setProperty( "MuQuality", 5 ) );
        ATH_CHECK( m_muonSelectionTool.setProperty( "UseMVALowPt", true));
    } else if (IdInt == 7){
        ATH_CHECK( m_muonSelectionTool.setProperty( "MuQuality", 4 ) );
        ATH_CHECK( m_muonSelectionTool.setProperty( "Use2stationMuonsHighPt", false));        
    } else ATH_CHECK(m_muonSelectionTool.setProperty( "MuQuality", m_muId ));
    ATH_CHECK( m_muonSelectionTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonSelectionTool.retrieve() );
  } else ATH_CHECK( m_muonSelectionTool.retrieve() );
 

  if (!m_muonSelectionHighPtTool.isUserConfigured()) { //Fixed to HighPt WP
    toolName = "MuonSelectionHighPtTool_" + muQual;
    m_muonSelectionHighPtTool.setTypeAndName("CP::MuonSelectionTool/"+toolName);
    ATH_CHECK( m_muonSelectionHighPtTool.setProperty( "MaxEta", m_muEta) );
    ATH_CHECK( m_muonSelectionHighPtTool.setProperty( "MuQuality", 4 ) );
    ATH_CHECK( m_muonSelectionHighPtTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonSelectionHighPtTool.retrieve() );
  } else  ATH_CHECK( m_muonSelectionHighPtTool.retrieve() );
  

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise muon efficiency tools
  if (!m_muonEfficiencySFTool.isUserConfigured() && m_muId != xAOD::Muon::VeryLoose) {
    toolName = "MuonEfficiencyScaleFactors_" + muQual;
    m_muonEfficiencySFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
    ATH_CHECK( m_muonEfficiencySFTool.setProperty("WorkingPoint", muQual) );
    ATH_CHECK( m_muonEfficiencySFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonEfficiencySFTool.retrieve() );
  } else  ATH_CHECK( m_muonEfficiencySFTool.retrieve() );


  if (!m_muonEfficiencyBMHighPtSFTool.isUserConfigured()){
    toolName = "MuonEfficiencyScaleFactorsBMHighPt_" + muQual;
    m_muonEfficiencyBMHighPtSFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
    ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.setProperty("WorkingPoint", "BadMuonVeto_HighPt") );
    ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.retrieve() );
  } else  ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.retrieve() );
 

  if (m_doTTVAsf && m_mud0sig<0 && m_muz0<0){
    ATH_MSG_WARNING("Requested TTVA SFs without d0sig and z0 cuts. Disabling scale factors as they will not make sense.");
    m_doTTVAsf=false;
  }

  if (m_doTTVAsf && !m_muonTTVAEfficiencySFTool.isUserConfigured()) {
    toolName = "MuonTTVAEfficiencyScaleFactors";
    m_muonTTVAEfficiencySFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
    ATH_CHECK( m_muonTTVAEfficiencySFTool.setProperty("WorkingPoint", "TTVA") );
    ATH_CHECK( m_muonTTVAEfficiencySFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonTTVAEfficiencySFTool.retrieve() );
  } else if (m_muonTTVAEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_muonTTVAEfficiencySFTool.retrieve() );



  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise muon isolation tool
  if (!m_muonIsolationSFTool.isUserConfigured() && !m_muIso_WP.empty()) {
    toolName = "MuonIsolationScaleFactors_" + m_muIso_WP;

    std::string tmp_muIso_WP = m_muIso_WP;
    if ( !check_isOption(m_muIso_WP, m_mu_iso_support) ) { //check if supported
      ATH_MSG_WARNING("Your selected muon Iso WP ("
        << m_muIso_WP
        << ") does not have SFs defined. Will try to find an appropriate fall-back.");
      if (m_mu_iso_fallback.count(m_muIso_WP) > 0){
        tmp_muIso_WP = m_mu_iso_fallback[m_muIso_WP];
        ATH_MSG_WARNING("Your selected muon Iso WP ("
          << m_muIso_WP
          << " is not supported, and does not have SFs available.  Falling back to "
          << tmp_muIso_WP
          << " for SF determination.");
      } else {
        ATH_MSG_ERROR("***  The muon isolation WP you selected (" << m_muIso_WP << ") is not currentely supported, and no known fall-back option for SFs exists. Sorry! ***");
        return StatusCode::FAILURE;
      }
    }

    m_muonIsolationSFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
    // Use for the low-pt WP a dedicated set of isolation scale-factors having an extra uncertainty in place
    ATH_CHECK( m_muonIsolationSFTool.setProperty("WorkingPoint", tmp_muIso_WP + "Iso") );
    ATH_CHECK( m_muonIsolationSFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonIsolationSFTool.retrieve() );

  } else if (m_muonIsolationSFTool.isUserConfigured()) ATH_CHECK( m_muonIsolationSFTool.retrieve() );


  if (!m_muonHighPtIsolationSFTool.isUserConfigured() && !m_muIsoHighPt_WP.empty()) {
    toolName = "MuonHighPtIsolationScaleFactors_" + m_muIsoHighPt_WP;

    std::string tmp_muIsoHighPt_WP = m_muIsoHighPt_WP;
    if ( !check_isOption(m_muIsoHighPt_WP, m_mu_iso_support) ) { //check if supported
      ATH_MSG_WARNING("Your selected muon high-pt Iso WP ("
        << m_muIsoHighPt_WP
        << ") does not have SFs defined. Will try to find an appropriate fall-back.");
      if (m_mu_iso_fallback.count(m_muIsoHighPt_WP) > 0){
        tmp_muIsoHighPt_WP = m_mu_iso_fallback[m_muIsoHighPt_WP];
        ATH_MSG_WARNING("Your selected muon high-pt Iso WP ("
          << m_muIsoHighPt_WP
          << " is not supported, and does not have SFs available.  Falling back to "
          << tmp_muIsoHighPt_WP
          << " for SF determination.");
      } else {
        ATH_MSG_ERROR("***  The muon isolation WP you selected (" << m_muIsoHighPt_WP << ") is not currentely supported, and no known fall-back option for SFs exists. Sorry! ***");
        return StatusCode::FAILURE;
      }
    }

    m_muonHighPtIsolationSFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
    // Use for the low-pt WP a dedicated set of isolation scale-factors having an extra uncertainty in place
    ATH_CHECK( m_muonHighPtIsolationSFTool.setProperty("WorkingPoint", tmp_muIsoHighPt_WP + "Iso") );
    ATH_CHECK( m_muonHighPtIsolationSFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonHighPtIsolationSFTool.retrieve() );

  } else if (m_muonHighPtIsolationSFTool.isUserConfigured()) ATH_CHECK( m_muonHighPtIsolationSFTool.retrieve() );


  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise muon trigger scale factor tools

  if (!m_muonTriggerSFTool.isUserConfigured()) {
    toolName = "MuonTriggerScaleFactors_" + muQual;
    m_muonTriggerSFTool.setTypeAndName("CP::MuonTriggerScaleFactors/"+toolName);
    if ( muQual=="LowPt" ) {
      ATH_MSG_WARNING("You're using the LowPt muon selection, which is not supported yet in terms of muon trigger scale facorts. TEMPORAIRLY configuring the muonTriggerSFTool for Medium muons. Beware!");
      ATH_CHECK( m_muonTriggerSFTool.setProperty("MuonQuality", "Medium" ) );
    }
    else ATH_CHECK( m_muonTriggerSFTool.setProperty("MuonQuality", muQual) );
    //ATH_CHECK( m_muonTriggerSFTool.setProperty("Isolation", m_muIso_WP)); This property has been depreacted long time ago
    ATH_CHECK( m_muonTriggerSFTool.setProperty("AllowZeroSF", true) );
    ATH_CHECK( m_muonTriggerSFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_muonTriggerSFTool.retrieve() );
    m_muonTrigSFTools.push_back(m_muonTriggerSFTool.getHandle());
  } else {
        ATH_CHECK( m_muonTriggerSFTool.retrieve() );
        m_muonTrigSFTools.push_back(m_muonTriggerSFTool.getHandle());
   }

  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise electron selector tools

  // Signal Electrons
  if (!m_elecSelLikelihood.isUserConfigured()) {
    toolName = "EleSelLikelihood_" + m_eleId;
    m_elecSelLikelihood.setTypeAndName("AsgElectronLikelihoodTool/"+toolName);

    if (! m_eleConfig.empty() ){
      ATH_MSG_INFO("Overriding specified Ele.Id working point in favour of configuration file");
      ATH_CHECK( m_elecSelLikelihood.setProperty("ConfigFile", m_eleConfig) );
    } else if ( !check_isOption(m_eleId, m_el_id_support) ) { //check if supported
      ATH_MSG_ERROR("Invalid electron ID selected: " << m_eleId);
      return StatusCode::FAILURE;
    }
    else if (m_eleId == "VeryLooseLLH" || m_eleId == "LooseLLH") {
      ATH_MSG_WARNING(" ****************************************************************************");
      ATH_MSG_WARNING(" CAUTION: Setting " << m_eleId << " as signal electron ID");
      ATH_MSG_WARNING(" These may be used for loose electron CRs but no scale factors are provided.");
      ATH_MSG_WARNING(" ****************************************************************************");
      ATH_CHECK( m_elecSelLikelihood.setProperty("WorkingPoint", EG_WP(m_eleId) ));
    } else {
      ATH_CHECK( m_elecSelLikelihood.setProperty("WorkingPoint", EG_WP(m_eleId) ));
    }

    ATH_CHECK( m_elecSelLikelihood.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_elecSelLikelihood.retrieve() );
  } else  ATH_CHECK( m_elecSelLikelihood.retrieve() );

  // Baseline Electrons
  if (!m_elecSelLikelihoodBaseline.isUserConfigured()) {
    toolName = "EleSelLikelihoodBaseline_" + m_eleIdBaseline;
    m_elecSelLikelihoodBaseline.setTypeAndName("AsgElectronLikelihoodTool/"+toolName);

    if (! m_eleConfigBaseline.empty() ){
      ATH_MSG_INFO("Overriding specified EleBaseline.Id working point in favour of configuration file");
      ATH_CHECK( m_elecSelLikelihoodBaseline.setProperty("ConfigFile", m_eleConfigBaseline ));
    } else if ( !check_isOption(m_eleIdBaseline, m_el_id_support) ) { //check if supported
      ATH_MSG_ERROR("Invalid electron ID selected: " << m_eleIdBaseline);
      return StatusCode::FAILURE;
    } else {
      ATH_CHECK( m_elecSelLikelihoodBaseline.setProperty("WorkingPoint", EG_WP(m_eleIdBaseline)) );
    }

    ATH_CHECK( m_elecSelLikelihoodBaseline.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_elecSelLikelihoodBaseline.retrieve() );
  } else ATH_CHECK( m_elecSelLikelihoodBaseline.retrieve() );
 


  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise photon selector tools

  if (!m_photonSelIsEM.isUserConfigured()) {
    toolName = "PhotonSelIsEM_" + m_photonId;
    m_photonSelIsEM.setTypeAndName("AsgPhotonIsEMSelector/"+toolName);

    if (!check_isOption(m_photonId, m_ph_id_support)){ //check if supported
      ATH_MSG_ERROR("Invalid photon ID selected: " << m_photonId);
      return StatusCode::FAILURE;
    }

    ATH_CHECK( m_photonSelIsEM.setProperty("WorkingPoint", m_photonId+"Photon") );
    ATH_CHECK( m_photonSelIsEM.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_photonSelIsEM.retrieve() );
  } else ATH_CHECK( m_photonSelIsEM.retrieve() );

  if (!m_photonSelIsEMBaseline.isUserConfigured()) {
    toolName = "PhotonSelIsEMBaseline_" + m_photonIdBaseline;
    m_photonSelIsEMBaseline.setTypeAndName("AsgPhotonIsEMSelector/"+toolName);

    if(!check_isOption(m_photonIdBaseline, m_ph_id_support)){ //check if supported
      ATH_MSG_ERROR("Invalid photon ID selected: " << m_photonIdBaseline);
      return StatusCode::FAILURE;
    }

    ATH_CHECK( m_photonSelIsEMBaseline.setProperty("WorkingPoint", m_photonIdBaseline+"Photon") );
    ATH_CHECK( m_photonSelIsEMBaseline.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_photonSelIsEMBaseline.retrieve() );
  } else  ATH_CHECK( m_photonSelIsEMBaseline.retrieve() );

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise DeadHVCellRemovalTool
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EGammaIdentificationRun2#Removal_of_Electron_Photon_clust

  ATH_MSG_DEBUG("Setup AsgDeadHVCellRemovalTool/deadHVTool");
  m_deadHVTool.setTypeAndName("AsgDeadHVCellRemovalTool/deadHVTool");
  ATH_CHECK(m_deadHVTool.retrieve());

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise electron efficiency tool

  PATCore::ParticleDataType::DataType data_type(PATCore::ParticleDataType::Data);
  if (!isData()) {
    if (isAtlfast()) data_type = PATCore::ParticleDataType::Fast;
    else data_type = PATCore::ParticleDataType::Full;
    ATH_MSG_DEBUG( "Setting data type to " << data_type);
  }

  toolName = "AsgElectronEfficiencyCorrectionTool_reco";
  CONFIG_EG_EFF_TOOL_KEY(m_elecEfficiencySFTool_reco, toolName, "RecoKey", "Reconstruction");

  if (m_eleId == "VeryLooseLLH" || m_eleId == "LooseLLH" || m_eleId == "Loose" || m_eleId == "Medium" || m_eleId == "Tight") {
    ATH_MSG_WARNING("Not configuring electron ID and trigger scale factors for " << m_eleId);
  }
  else {

    // This needs to be formatted for the scale factors: no _Rel20, no LH label, etc.
    std::string eleId = TString(m_eleId).ReplaceAll("AndBLayer", "BLayer").ReplaceAll("LLH", "").Data();

    // electron id
    toolName = "AsgElectronEfficiencyCorrectionTool_id_" + m_eleId;
    CONFIG_EG_EFF_TOOL_KEY(m_elecEfficiencySFTool_id, toolName, "IdKey", eleId);

    // override map file use if correction file list is set for WP
    std::map<std::string,std::string> corrFNList;
    if ( !m_EG_corrFNList.empty() ) {
       for ( auto WP_fname : split( m_EG_corrFNList, "," ) ) {
          std::string WP = WP_fname.substr(0,WP_fname.find(":"));
          std::string fname = WP_fname.substr(WP_fname.find(":")+1);
          corrFNList[WP] = fname; 
          ATH_MSG_WARNING( "Correction file list defined for WP " << WP << ": " << fname << "." );
          ATH_MSG_WARNING( "Will use correction file rather than central map file." );
       }
    }

    //-- get KEYS supported by egamma SF tools
    std::vector<std::string> eSF_keys = getElSFkeys(m_eleEffMapFilePath);

    // electron iso
    std::string EleIso("");
    if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_eleIso_WP) != eSF_keys.end()){
      EleIso   = m_eleIso_WP;
    } else if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_el_iso_fallback[m_eleIso_WP]) != eSF_keys.end()){
      //--- Check to see if the only issue is an unknown isolation working point
      EleIso = m_el_iso_fallback[m_eleIso_WP];
      ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_iso_*) Your selected electron Iso WP ("
        << m_eleIso_WP
        << ") does not have iso SFs defined. Falling back to "
        << m_el_iso_fallback[m_eleIso_WP]
        << " for SF calculations");
    }
    else{
      ATH_MSG_ERROR("***  THE ELECTRON ISOLATION SF YOU SELECTED (" << m_eleIso_WP << ") GOT NO SUPPORT ***");
      return StatusCode::FAILURE;
    }

    toolName = "AsgElectronEfficiencyCorrectionTool_iso_" + m_eleId + EleIso;
    
    // if running with correction file list
    if ( (!m_EG_corrFNList.empty()) && corrFNList.find(EleIso)!=corrFNList.end() ) {                // overriding central map file
      CONFIG_EG_EFF_TOOL( m_elecEfficiencySFTool_iso, toolName, corrFNList[EleIso] );
    } 
    // can't do the iso tool via the macro, it needs two properties set
    else {                                                                                               // default: use map file
      if ( !m_elecEfficiencySFTool_iso.isUserConfigured() ) {
        if ( !check_isOption(EleIso, m_el_iso_support) ) { //check if supported
          ATH_MSG_WARNING( "(" << toolName << ") Your electron Iso WP: " << EleIso<< " is no longer supported. This will almost certainly cause a crash now.");
        }

        m_elecEfficiencySFTool_iso.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);

        if ( m_EG_corrFNList.empty() ) {
           ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("MapFilePath", m_eleEffMapFilePath) );
        } else {
           ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("CorrectionFileNameList", corrFNList) );
        }
        ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("IdKey", eleId) );
        ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("IsoKey", EleIso) );
        if (!isData() && ((EleIso.find("TightTrackOnly_VarRad")!=std::string::npos)||
                          (EleIso.find("TightTrackOnly_FixedRad")!=std::string::npos)||
                          (EleIso.find("Tight_VarRad")!=std::string::npos)||
                          (EleIso.find("Loose_VarRad")!=std::string::npos))) {
          if (isAtlfast()) ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool/"+toolName+"). Your selected electron Iso WP (" + EleIso + ") don't have AFII SF. Falling back to FullSim");
          ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("ForceDataType", (int) PATCore::ParticleDataType::Full) );
        }
        else if (!isData()){
          ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("ForceDataType", (int) data_type) );
        }
        ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("CorrelationModel", m_EG_corrModel) );
        ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_elecEfficiencySFTool_iso.initialize() );
      } else ATH_CHECK( m_elecEfficiencySFTool_iso.initialize() );
    }
   
    // electron iso high-pt
    std::string EleIsohighPt("");
    if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_eleIsoHighPt_WP) != eSF_keys.end()){
      EleIsohighPt   = m_eleIsoHighPt_WP;
    } else if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_el_iso_fallback[m_eleIsoHighPt_WP]) != eSF_keys.end()){
      //--- Check to see if the only issue is an unknown isolation working point
      EleIsohighPt = m_el_iso_fallback[m_eleIsoHighPt_WP];
      ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_iso_*) Your selected high-pT electron Iso WP ("
        << m_eleIsoHighPt_WP
        << ") does not have iso SFs defined. Falling back to "
        << m_el_iso_fallback[m_eleIsoHighPt_WP]
        << " for SF calculations");
    }
    else{
      ATH_MSG_ERROR("***  THE HIGH PT ELECTRON ISOLATION SF YOU SELECTED (" << m_eleIsoHighPt_WP << ") GOT NO SUPPORT");
      return StatusCode::FAILURE;
    }

    toolName = "AsgElectronEfficiencyCorrectionTool_isoHigPt_" + m_eleId + EleIsohighPt;
    
    // if running with correction file list
    if ( (!m_EG_corrFNList.empty()) && corrFNList.find(EleIsohighPt)!=corrFNList.end() ) {                // overriding central map file
      CONFIG_EG_EFF_TOOL( m_elecEfficiencySFTool_isoHighPt, toolName, corrFNList[EleIsohighPt] );
    } 
    // can't do the iso tool via the macro, it needs two properties set
    else {                                                                                                     // default: use map file
      if ( !m_elecEfficiencySFTool_isoHighPt.isUserConfigured() ) {
  
        if ( !check_isOption(EleIsohighPt, m_el_iso_support) ) { //check if supported
          ATH_MSG_WARNING( "(" << toolName << ") Your electron high-pt Iso WP: " << EleIsohighPt << " is no longer supported. This will almost certainly cause a crash now.");
        }
  
        m_elecEfficiencySFTool_isoHighPt.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);
  
        ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("MapFilePath", m_eleEffMapFilePath) );
        ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("IdKey", eleId) );
        ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("IsoKey", EleIsohighPt) );
        if (!isData()) {
          ATH_CHECK (m_elecEfficiencySFTool_isoHighPt.setProperty("ForceDataType", (int) data_type) );
        }
        ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("CorrelationModel", m_EG_corrModel) );
        ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.initialize() );
      } else   ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.initialize() );
    }
    

    // electron triggers - first SFs (but we need to massage the id string since all combinations are not supported)

    //single lepton
    std::string triggerEleIso("");
    if (std::find(eSF_keys.begin(), eSF_keys.end(), m_electronTriggerSFStringSingle+"_"+eleId+"_"+m_eleIso_WP) != eSF_keys.end()){
      triggerEleIso   = m_eleIso_WP;
    } else if (std::find(eSF_keys.begin(), eSF_keys.end(), m_electronTriggerSFStringSingle+"_"+eleId+"_"+m_el_iso_fallback[m_eleIso_WP]) != eSF_keys.end()){
      //--- Check to see if the only issue is an unknown isolation working point
      triggerEleIso = m_el_iso_fallback[m_eleIso_WP];
      ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_trig_singleLep_*) Your selected electron Iso WP ("
        << m_eleIso_WP
        << ") does not have trigger SFs defined. Falling back to "
        << triggerEleIso
        << " for SF calculations");
    }
    else{
      ATH_MSG_ERROR("***  THE ELECTRON TRIGGER SF YOU SELECTED (" << m_electronTriggerSFStringSingle << ") GOT NO SUPPORT FOR YOUR ID+ISO WPs (" << m_eleId << "+" << m_eleIso_WP << ") ***");
      return StatusCode::FAILURE;
    }

    toolName = "AsgElectronEfficiencyCorrectionTool_trig_singleLep_" + m_eleId;
    
    if ( !m_elecEfficiencySFTool_trig_singleLep.isUserConfigured() ) {
      m_elecEfficiencySFTool_trig_singleLep.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("MapFilePath", m_eleEffMapFilePath) );
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("TriggerKey", m_electronTriggerSFStringSingle) );
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("IdKey", eleId) );
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("IsoKey", triggerEleIso) );
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("CorrelationModel", m_EG_corrModel) );
      if (!isData()) {
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
      }
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.initialize() );
    } else ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.retrieve() );
    

    toolName = "AsgElectronEfficiencyCorrectionTool_trigEff_singleLep_" + m_eleId;
    
    if ( !m_elecEfficiencySFTool_trigEff_singleLep.isUserConfigured() ) {
      m_elecEfficiencySFTool_trigEff_singleLep.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("MapFilePath", m_eleEffMapFilePath) );
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("TriggerKey", "Eff_"+m_electronTriggerSFStringSingle) );
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("IdKey", eleId) );
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("IsoKey", triggerEleIso) );
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("CorrelationModel", m_EG_corrModel) );
      if (!isData()) {
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
      }
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.initialize() );
    } else  ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.retrieve() );
  

    //mixed-leptons
    std::map<std::string,std::string> electronTriggerSFMapMixedLepton {
      // legs, Trigger keys, 
      {"e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose,e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0", m_electronTriggerSFStringSingle},
      {"e24_lhvloose_nod0_L1EM20VH,e17_lhvloose_nod0,e12_lhloose_L1EM10VH","DI_E_2015_e12_lhloose_L1EM10VH_2016_e17_lhvloose_nod0_2017_2018_e24_lhvloose_nod0_L1EM20VH"},
      {"e26_lhmedium_nod0_L1EM22VHI,e26_lhmedium_nod0","MULTI_L_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_e26_lhmedium_nod0_L1EM22VHI_2017_2018_e26_lhmedium_nod0"},
      {"e17_lhloose,e17_lhloose_nod0","MULTI_L_2015_e17_lhloose_2016_2018_e17_lhloose_nod0"},
      {"e12_lhloose,e12_lhloose_nod0","MULTI_L_2015_e12_lhloose_2016_2018_e12_lhloose_nod0"},
      {"e7_lhmedium,e7_lhmedium_nod0","MULTI_L_2015_e7_lhmedium_2016_2018_e7_lhmedium_nod0"},
      {"e9_lhloose,e9_lhloose_nod0,e12_lhvloose_nod0_L1EM10VH","TRI_E_2015_e9_lhloose_2016_e9_lhloose_nod0_2017_2018_e12_lhvloose_nod0_L1EM10VH"}
      
    };
    
    // 2e17 trigger is used in 2017 or 2018?
    std::string triglist_2017to2018 = m_trig2017combination_diLep + "_" + m_trig2018combination_diLep + "_" + m_trig2017combination_multiLep + "_" + m_trig2018combination_multiLep;
    if (triglist_2017to2018.find("2e17_lhvloose_nod0_L12EM15VHI") != std::string::npos) { 
      electronTriggerSFMapMixedLepton["e17_lhvloose_nod0_L1EM15VHI"] = "DI_E_2015_e12_lhloose_L1EM10VH_2016_e17_lhvloose_nod0_2017_2018_e17_lhvloose_nod0_L1EM15VHI";
    }

    std::string triggerMixedEleIso("");

    for(auto const& item : electronTriggerSFMapMixedLepton){

      if (std::find(eSF_keys.begin(), eSF_keys.end(), item.second+"_"+eleId+"_"+m_eleIso_WP) != eSF_keys.end()){
        triggerMixedEleIso = m_eleIso_WP;
      } else if (std::find(eSF_keys.begin(), eSF_keys.end(), item.second+"_"+eleId+"_"+m_el_iso_fallback[m_eleIso_WP]) != eSF_keys.end()){
        //--- Check to see if the only issue is an unknown isolation working point
        triggerMixedEleIso = m_el_iso_fallback[m_eleIso_WP];
        ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_trig_mixLep_*) Your selected electron Iso WP ("
          << m_eleIso_WP
          << ") does not have trigger SFs defined. Falling back to "
          << triggerMixedEleIso
          << " for SF calculations");
      } else {
        ATH_MSG_ERROR("***  THE ELECTRON TRIGGER SF YOU SELECTED (" << item.second << ") GOT NO SUPPORT FOR YOUR ID+ISO WPs (" << m_eleId << "+" << m_eleIso_WP << "). The fallback options failed as well sorry! ***");
        return StatusCode::FAILURE;
      }

      ATH_MSG_VERBOSE ("Selected WP: " << item.second << "_" << eleId << "_" << triggerMixedEleIso);

      toolName = "AsgElectronEfficiencyCorrectionTool_trig_mixLep_" + (item.first).substr(0,8) + m_eleId;
      
      auto t_sf = m_elecEfficiencySFTool_trig_mixLep.emplace(m_elecEfficiencySFTool_trig_mixLep.end(), "AsgElectronEfficiencyCorrectionTool/"+toolName);
      ATH_CHECK( t_sf->setProperty("MapFilePath", m_eleEffMapFilePath) );
      ATH_CHECK( t_sf->setProperty("TriggerKey", item.second) );
      ATH_CHECK( t_sf->setProperty("IdKey", eleId) );
      ATH_CHECK( t_sf->setProperty("IsoKey", triggerMixedEleIso) );
      ATH_CHECK( t_sf->setProperty("CorrelationModel", m_EG_corrModel) );
      if (!isData()) {
        ATH_CHECK( t_sf->setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
      }
      ATH_CHECK( t_sf->setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( t_sf->initialize() );
      m_elecTrigSFTools.push_back(t_sf->getHandle());
#ifndef XAOD_STANDALONE
      m_legsPerTool[toolName] = item.first;
#else
      m_legsPerTool["ToolSvc."+toolName] = item.first;
#endif

      toolName = "AsgElectronEfficiencyCorrectionTool_trigEff_mixLep_" + (item.first).substr(0,8) + m_eleId;
      auto t_eff = m_elecEfficiencySFTool_trigEff_mixLep.emplace(m_elecEfficiencySFTool_trigEff_mixLep.end(), "AsgElectronEfficiencyCorrectionTool/"+toolName);
      ATH_CHECK( t_eff->setProperty("MapFilePath", m_eleEffMapFilePath) );
      ATH_CHECK( t_eff->setProperty("TriggerKey", "Eff_"+item.second) );
      ATH_CHECK( t_eff->setProperty("IdKey", eleId) );
      ATH_CHECK( t_eff->setProperty("IsoKey", triggerMixedEleIso) );
      ATH_CHECK( t_eff->setProperty("CorrelationModel", m_EG_corrModel) );
      if (!isData()) {
        ATH_CHECK( t_eff->setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
      }
      ATH_CHECK( t_eff->setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( t_eff->initialize() );
      m_elecTrigEffTools.push_back(t_eff->getHandle());
#ifndef XAOD_STANDALONE
      m_legsPerTool[toolName] = item.first;
#else
      m_legsPerTool["ToolSvc."+toolName] = item.first;
#endif

    }
  }

  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ElectronChargeFlipTaggerTool#Calculating_the_ECIDS_decision
  std::string tmpIsoWP = m_el_iso_fallback[m_eleIso_WP];
  std::string tmpIDWP = m_eleId;
  // Only Medium/TightIDs supported for now. Only Gradient and FCTight supported for now
  if (tmpIDWP != "MediumLLH" && tmpIDWP != "TightLLH") {
    ATH_MSG_WARNING("Your Electron ID WP ("+tmpIDWP+") is not supported for ECID SFs, falling back to MediumLLH for SF purposes");
    tmpIDWP = "MediumLLH";
  }
  if (tmpIsoWP != "FCTight" && tmpIsoWP != "Gradient") {
    ATH_MSG_WARNING("Your Electron Iso WP ("+tmpIsoWP+") is not supported for ECID SFs, falling back to FCTight for SF purposes");
    tmpIsoWP = "FCTight";
  }

  toolName = "AsgElectronEfficiencyCorrectionTool_chf_" + tmpIDWP + tmpIsoWP + m_eleChID_WP;
  
  if( !m_elecEfficiencySFTool_chf.isUserConfigured() ) { 
    m_elecEfficiencySFTool_chf.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName); 
    std::vector<std::string> corrFileNamechf = {"ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/additional/efficiencySF.ChargeID."+tmpIDWP+"_d0z0_v13_"+tmpIsoWP+"_ECIDSloose.root"};
    ATH_CHECK( m_elecEfficiencySFTool_chf.setProperty("CorrectionFileNameList", corrFileNamechf) ); 
    if(!isData()){
      ATH_CHECK( m_elecEfficiencySFTool_chf.setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
    }
    ATH_CHECK( m_elecEfficiencySFTool_chf.setProperty("CorrelationModel", m_EG_corrModel) );
    ATH_CHECK( m_elecEfficiencySFTool_chf.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_elecEfficiencySFTool_chf.initialize() );
  } else ATH_CHECK(m_elecEfficiencySFTool_chf.retrieve());
  m_runECIS = m_eleChID_WP.empty() ? false : true;

  // Electron charge mis-identification SFs
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/LatestRecommendationsElectronIDRun2#Scale_factors_for_electrons_char
  toolName = "ElectronChargeEfficiencyCorrectionTool_" + m_eleId + m_el_iso_fallback[m_eleIso_WP];
  if ( !m_elecChargeEffCorrTool.isUserConfigured() ) {
    m_elecChargeEffCorrTool.setTypeAndName("CP::ElectronChargeEfficiencyCorrectionTool/"+toolName);

    // Reset this variable as more Iso WPs are supported for the below
    std::string tmpIsoWP = m_el_iso_fallback[m_eleIso_WP]; 
    std::string tmpIDWP = m_eleId;
    if ( m_eleChIso && !check_isOption(tmpIsoWP, m_el_iso_support) ) { //check if supported
      ATH_MSG_WARNING( "(" << toolName << ") Your electron Iso WP: " << m_el_iso_fallback[m_eleIso_WP] << " is no longer supported. This will almost certainly cause a crash now.");
    }
    if (tmpIDWP != "MediumLLH" && tmpIDWP != "TightLLH" && !(tmpIDWP=="LooseAndBLayerLLH" && m_eleChIso==false) ) { //check if supported
      ATH_MSG_WARNING("Your Electron ID WP ("+tmpIDWP+") is not supported for charge ID SFs, falling back to MediumLLH for SF purposes");
      tmpIDWP = "MediumLLH";
    }

    std::string chfFile("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/charge_misID/chargeEfficiencySF."+tmpIDWP+"_d0z0_v13_"+tmpIsoWP+".root");
    if (!m_eleChIso) { 
       chfFile = "ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/charge_misID/chargeEfficiencySF."+tmpIDWP+"_d0z0_v13.root"; 
       ATH_MSG_WARNING( "ElectronChargeEfficiencyCorrectionTool: using charge ID SF without Iso applied!");
    }
    ATH_MSG_DEBUG( "ElectronChargeEfficiencyCorrectionTool correctionFileName: " << chfFile );

    ATH_CHECK( m_elecChargeEffCorrTool.setProperty("CorrectionFileName", chfFile) );

    if (!isData()) {
      ATH_CHECK ( m_elecChargeEffCorrTool.setProperty("ForceDataType", (int) data_type) );
    }
    ATH_CHECK( m_elecChargeEffCorrTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_elecChargeEffCorrTool.initialize() );
  } else ATH_CHECK( m_elecChargeEffCorrTool.initialize() );


  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise photon efficiency tool

  if (!m_photonEfficiencySFTool.isUserConfigured() && !isData()) {
    m_photonEfficiencySFTool.setTypeAndName("AsgPhotonEfficiencyCorrectionTool/AsgPhotonEfficiencyCorrectionTool_" + m_photonId);

    if (m_photonId != "Tight" ) {
      ATH_MSG_WARNING( "No Photon efficiency available for " << m_photonId << ", using Tight instead..." );
    }

    ATH_CHECK( m_photonEfficiencySFTool.setProperty("ForceDataType", 1) ); // Set data type: 1 for FULLSIM, 3 for AF2
    ATH_CHECK( m_photonEfficiencySFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_photonEfficiencySFTool.retrieve() );
  } else if (m_photonEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_photonEfficiencySFTool.retrieve() );
 
  if (!m_photonIsolationSFTool.isUserConfigured() && !isData()) {
    m_photonIsolationSFTool.setTypeAndName("AsgPhotonEfficiencyCorrectionTool/AsgPhotonEfficiencyCorrectionTool_isol" + m_photonIso_WP);

    if (m_photonIso_WP != "FixedCutTight" && m_photonIso_WP != "FixedCutLoose" && m_photonIso_WP != "FixedCutTightCaloOnly") {
      ATH_MSG_WARNING( "No Photon efficiency available for " << m_photonIso_WP);
    }

    ATH_CHECK( m_photonIsolationSFTool.setProperty("IsoKey", m_photonIso_WP.substr(8) ));    // Set isolation WP: Loose,Tight,TightCaloOnly
    ATH_CHECK( m_photonIsolationSFTool.setProperty("ForceDataType", 1) ); // Set data type: 1 for FULLSIM, 3 for AF2
    ATH_CHECK( m_photonIsolationSFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_photonIsolationSFTool.retrieve() );
  } else if (m_photonEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_photonIsolationSFTool.retrieve() );
 

  // trigger scale factors 
  if (!m_photonTriggerSFTool.isUserConfigured() && !isData()) {
    m_photonTriggerSFTool.setTypeAndName("AsgPhotonEfficiencyCorrectionTool/AsgPhotonEfficiencyCorrectionTool_trig" + m_photonTriggerName);

    // Fallback to TightCaloOnly if Tight is selected
    std::string photonIso_forTrigSF = m_photonIso_WP;
    if (m_photonIso_WP == "FixedCutTight") {
      ATH_MSG_WARNING( "No Photon trigger SF available for " << m_photonIso_WP << ", using TightCaloOnly instead... Use at your own risk" );  
      photonIso_forTrigSF = "TightCaloOnly";
    } else { //  isolation WP supported: Loose or TightCaloOnly, removing "FixedCut" suffix..
      photonIso_forTrigSF = TString(m_photonIso_WP).ReplaceAll("FixedCut","").Data();
    } 

    // "symmetric" diphoton triggers (year dependent)
    ATH_CHECK( m_photonTriggerSFTool.setProperty("IsoKey", photonIso_forTrigSF ));    // Set isolation WP: Loose,TightCaloOnly 
    ATH_CHECK( m_photonTriggerSFTool.setProperty("TriggerKey", m_photonTriggerName ));    
    ATH_CHECK( m_photonTriggerSFTool.setProperty("ForceDataType", 1) ); // Set data type: 1 for FULLSIM, 3 for AF2
    ATH_CHECK( m_photonTriggerSFTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_photonTriggerSFTool.retrieve() );

    // "asymmetric" diphoton triggers
    std::map<std::string,std::string> diphotonTriggerSFMapAsymmetric {
      // legs, Trigger keys, 
      {"g25_loose,g25_medium_L1EM20VH", "DI_PH_2015_g25_loose_2016_g25_loose_2017_g25_medium_L1EM20VH_2018_g25_medium_L1EM20VH"},
      {"g35_loose,g35_medium_L1EM20VH", "DI_PH_2015_g35_loose_2016_g35_loose_2017_g35_medium_L1EM20VH_2018_g35_medium_L1EM20VH"},
    };

    for(auto const& item : diphotonTriggerSFMapAsymmetric){

      toolName = "AsgPhotonEfficiencyCorrectionTool_trigSF_asymm_diphoton_" + (item.first).substr(0,9) + photonIso_forTrigSF;
      auto ph_trigSF = m_photonEfficiencySFTool_trigSF_AsymDiphoton.emplace(m_photonEfficiencySFTool_trigSF_AsymDiphoton.end(), "AsgPhotonEfficiencyCorrectionTool/"+toolName);
      ATH_CHECK( ph_trigSF->setProperty("IsoKey", photonIso_forTrigSF) );
      ATH_CHECK( ph_trigSF->setProperty("TriggerKey", item.second) );
      ATH_CHECK( ph_trigSF->setProperty("ForceDataType", 1) ); // Set DataType: 1 for FullSim and 3 for AFII
      ATH_CHECK( ph_trigSF->setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( ph_trigSF->initialize() );
      m_photonTrigSFTools.push_back(ph_trigSF->getHandle());
#ifndef XAOD_STANDALONE
      m_legsPerTool_ph[toolName] = item.first;
#else
      m_legsPerTool_ph["ToolSvc."+toolName] = item.first;
#endif

      toolName = "AsgPhotonEfficiencyCorrectionTool_trigEff_asymm_diphoton_" + (item.first).substr(0,9) + photonIso_forTrigSF;
      auto ph_trigEff = m_photonEfficiencySFTool_trigEff_AsymDiphoton.emplace(m_photonEfficiencySFTool_trigEff_AsymDiphoton.end(), "AsgPhotonEfficiencyCorrectionTool/"+toolName);
      ATH_CHECK( ph_trigEff->setProperty("IsoKey", photonIso_forTrigSF) );
      ATH_CHECK( ph_trigEff->setProperty("TriggerKey", "Eff_"+item.second) );
      ATH_CHECK( ph_trigEff->setProperty("ForceDataType", 1) ); // Set DataType: 1 for FullSim and 3 for AFII
      ATH_CHECK( ph_trigEff->setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( ph_trigEff->initialize() );
      m_photonTrigEffTools.push_back(ph_trigEff->getHandle());
#ifndef XAOD_STANDALONE
      m_legsPerTool_ph[toolName] = item.first;
#else
      m_legsPerTool_ph["ToolSvc."+toolName] = item.first;
#endif

    }
  }

 ///////////////////////////////////////////////////////////////////////////////////////////
 // Initialize the MC fudge tool

  if (!m_electronPhotonShowerShapeFudgeTool.isUserConfigured()) {
    m_electronPhotonShowerShapeFudgeTool.setTypeAndName("ElectronPhotonShowerShapeFudgeTool/ElectronPhotonShowerShapeFudgeTool");

    int FFset = 22;
    ATH_CHECK( m_electronPhotonShowerShapeFudgeTool.setProperty("Preselection", FFset) );
    ATH_CHECK( m_electronPhotonShowerShapeFudgeTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_electronPhotonShowerShapeFudgeTool.retrieve() );
  } else ATH_CHECK( m_electronPhotonShowerShapeFudgeTool.retrieve() );
  

///////////////////////////////////////////////////////////////////////////////////////////
// Initialize the EgammaAmbiguityTool

  if (!m_egammaAmbiguityTool.isUserConfigured()) {
    m_egammaAmbiguityTool.setTypeAndName("EGammaAmbiguityTool/EGammaAmbiguityTool");
    ATH_CHECK( m_egammaAmbiguityTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_egammaAmbiguityTool.retrieve() );
  } else ATH_CHECK( m_egammaAmbiguityTool.retrieve() );
 

///////////////////////////////////////////////////////////////////////////////////////////
// Initialize the AsgElectronChargeIDSelector

  if (!m_elecChargeIDSelectorTool.isUserConfigured()) {

    // For the selector, can use the nice function
    std::string eleId = EG_WP(m_eleId);
    m_elecChargeIDSelectorTool.setTypeAndName("AsgElectronChargeIDSelectorTool/ElectronChargeIDSelectorTool_"+eleId);
    //default cut value for https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ElectronChargeFlipTaggerTool
    float BDTcut = -0.337671; // Loose 97%
    if (m_eleChID_WP != "Loose" && !m_eleChID_WP.empty()) {
      ATH_MSG_ERROR("Only Loose WP is supported in R21. Invalid ChargeIDSelector WP selected : " << m_eleChID_WP);
      return StatusCode::FAILURE;
    } 

    ATH_CHECK( m_elecChargeIDSelectorTool.setProperty("TrainingFile", "ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root"));
    ATH_CHECK( m_elecChargeIDSelectorTool.setProperty("CutOnBDT", BDTcut));
    ATH_CHECK( m_elecChargeIDSelectorTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_elecChargeIDSelectorTool.retrieve() );
  } else  ATH_CHECK( m_elecChargeIDSelectorTool.retrieve() );
 

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise egamma calibration tool

  if (!m_egammaCalibTool.isUserConfigured()) {
    m_egammaCalibTool.setTypeAndName("CP::EgammaCalibrationAndSmearingTool/EgammaCalibrationAndSmearingTool");
    ATH_MSG_DEBUG( "Initialising EgcalibTool " );
    ATH_CHECK( m_egammaCalibTool.setProperty("ESModel", "es2018_R21_v0") ); //used for analysis using data processed with 21.0
    ATH_CHECK( m_egammaCalibTool.setProperty("decorrelationModel", "1NP_v1") );
    ATH_CHECK( m_egammaCalibTool.setProperty("useAFII", isAtlfast()?1:0) );
    ATH_CHECK( m_egammaCalibTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_egammaCalibTool.retrieve() );
  } else ATH_CHECK( m_egammaCalibTool.retrieve() );



///////////////////////////////////////////////////////////////////////////////////////////
// No tau score re-decorator in R21; might come back some day, would go here

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise tau selection tools

  if (!m_tauSelTool.isUserConfigured()) {
    std::string inputfile = "";
    if (!m_tauConfigPath.empty() && (m_tauConfigPath!="default")) inputfile = m_tauConfigPath;
    else if (m_tauId == "VeryLoose") inputfile = "SUSYTools/tau_selection_veryloose.conf";
    else if (m_tauId == "Loose") inputfile = "SUSYTools/tau_selection_loose.conf";
    else if (m_tauId == "Medium") inputfile = "SUSYTools/tau_selection_medium.conf";
    else if (m_tauId == "Tight") inputfile = "SUSYTools/tau_selection_tight.conf";
    else {
      ATH_MSG_ERROR("Invalid tau ID selected: " << m_tauId);
      return StatusCode::FAILURE;
    }
    toolName = "TauSelectionTool_" + m_tauId;
    m_tauSelTool.setTypeAndName("TauAnalysisTools::TauSelectionTool/"+toolName);
    ATH_CHECK( m_tauSelTool.setProperty("ConfigPath", inputfile) );

    ATH_CHECK( m_tauSelTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_tauSelTool.retrieve() );
  } else  ATH_CHECK( m_tauSelTool.retrieve() );
  

  if (!m_tauSelToolBaseline.isUserConfigured()) {
    std::string inputfile = "";
    if (!m_tauConfigPathBaseline.empty() && (m_tauConfigPathBaseline!="default")) inputfile = m_tauConfigPathBaseline;
    else if (m_tauIdBaseline == "VeryLoose") inputfile = "SUSYTools/tau_selection_veryloose.conf";
    else if (m_tauIdBaseline == "Loose") inputfile = "SUSYTools/tau_selection_loose.conf";
    else if (m_tauIdBaseline == "Medium") inputfile = "SUSYTools/tau_selection_medium.conf";
    else if (m_tauIdBaseline == "Tight") inputfile = "SUSYTools/tau_selection_tight.conf";
    else {
      ATH_MSG_ERROR("Invalid baseline tau ID selected: " << m_tauIdBaseline);
      return StatusCode::FAILURE;
    }
    toolName = "TauSelectionToolBaseline_" + m_tauIdBaseline;
    m_tauSelToolBaseline.setTypeAndName("TauAnalysisTools::TauSelectionTool/"+toolName);
    ATH_CHECK( m_tauSelToolBaseline.setProperty("ConfigPath", inputfile) );

    ATH_CHECK( m_tauSelToolBaseline.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_tauSelToolBaseline.retrieve() );
  } else  ATH_CHECK( m_tauSelToolBaseline.retrieve() );
 

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise tau efficiency tool

  if (!m_tauEffTool.isUserConfigured()) {
    toolName = "TauEffTool_" + m_tauId;
    m_tauEffTool.setTypeAndName("TauAnalysisTools::TauEfficiencyCorrectionsTool/"+toolName);
    ATH_CHECK( m_tauEffTool.setProperty("PileupReweightingTool",m_prwTool.getHandle()) );

    if (!m_tauSelTool.empty()) {
      ATH_CHECK( m_tauEffTool.setProperty("TauSelectionTool", m_tauSelTool.getHandle()) );
    }
    ATH_CHECK( m_tauEffTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_tauEffTool.setProperty("isAFII", isAtlfast()) );
    ATH_CHECK( m_tauEffTool.retrieve() );
  } else ATH_CHECK( m_tauEffTool.retrieve() );


  // TODO: add SF tool for baseline tau id as well? /CO

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise tau trigger efficiency tool(s)

  if (!isData()) {
    int iTauID = (int) TauAnalysisTools::JETIDNONEUNCONFIGURED;
    if (m_tauId == "VeryLoose")   iTauID = (int) TauAnalysisTools::JETIDRNNVERYLOOSE;
    else if (m_tauId == "Loose")  iTauID = (int) TauAnalysisTools::JETIDRNNLOOSE;
    else if (m_tauId == "Medium") iTauID = (int) TauAnalysisTools::JETIDRNNMEDIUM;
    else if (m_tauId == "Tight")  iTauID = (int) TauAnalysisTools::JETIDRNNTIGHT;
    else {
      ATH_MSG_ERROR("Invalid tau ID selected: " << m_tauId);
      return StatusCode::FAILURE;
    }
    
    for(auto const& trigger : m_tau_trig_support) {
      toolName = "TauTrigEffTool_" + m_tauId + "_" + trigger.first;
      auto tau_trigSF = m_tauTrigEffTool.emplace(m_tauTrigEffTool.end(), "TauAnalysisTools::TauEfficiencyCorrectionsTool/"+toolName);
      ATH_CHECK( tau_trigSF->setProperty("EfficiencyCorrectionTypes", std::vector<int>({TauAnalysisTools::SFTriggerHadTau})) );
      ATH_CHECK( tau_trigSF->setProperty("TriggerName", trigger.first) );
      ATH_CHECK( tau_trigSF->setProperty("IDLevel", iTauID) );
      ATH_CHECK( tau_trigSF->setProperty("PileupReweightingTool", m_prwTool.getHandle()) );
      ATH_CHECK( tau_trigSF->setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( tau_trigSF->setProperty("isAFII", isAtlfast()) );
      ATH_CHECK( tau_trigSF->initialize() );
    }
  }


///////////////////////////////////////////////////////////////////////////////////////////
// Initialise tau smearing tool

  if (!m_tauSmearingTool.isUserConfigured()) {
    m_tauSmearingTool.setTypeAndName("TauAnalysisTools::TauSmearingTool/TauSmearingTool");
    ATH_MSG_INFO("'TauMVACalibration' is the default procedure in R21");
    ATH_CHECK( m_tauSmearingTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_tauSmearingTool.retrieve() );
  } else ATH_CHECK( m_tauSmearingTool.retrieve() );
  

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise tau truth matching tool

  if (!m_tauTruthMatch.isUserConfigured() && m_tauDoTTM) {
    m_tauTruthMatch.setTypeAndName("TauAnalysisTools::TauTruthMatchingTool/TauTruthMatch");
    ATH_CHECK( m_tauTruthMatch.setProperty("WriteTruthTaus", true) );
    ATH_CHECK( m_tauTruthMatch.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_tauTruthMatch.retrieve() );
  } else if (m_tauTruthMatch.isUserConfigured()) ATH_CHECK( m_tauTruthMatch.retrieve() );
 

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise TauOverlappingElectronLLHDecorator tool

  if (!m_tauElORdecorator.isUserConfigured()) {
    m_tauElORdecorator.setTypeAndName("TauAnalysisTools::TauOverlappingElectronLLHDecorator/TauEleORDecorator");
    ATH_CHECK( m_tauElORdecorator.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_tauElORdecorator.retrieve() );
  } else  ATH_CHECK( m_tauElORdecorator.retrieve() );


///////////////////////////////////////////////////////////////////////////////////////////
// Initialise B-tagging tools

  // Warnings for invalid timestamps, or timestamped containers with old CDI file & vice versa
  int cdiYear = atoi(m_bTaggingCalibrationFilePath.substr(m_bTaggingCalibrationFilePath.find("13TeV/")+6,4).c_str());
  // Regular Jets
  if (m_useBtagging && (!m_BtagTimeStamp.empty() && !(m_BtagTimeStamp.compare("201810")==0||m_BtagTimeStamp.compare("201903")==0))) {
    ATH_MSG_ERROR("Only 201810 & 201903 are valid BTag container timestamps. Current = " << m_BtagTimeStamp);
    return StatusCode::FAILURE;
  }
  if (m_useBtagging && (!m_BtagTimeStamp.empty() && cdiYear<2019)) {
    ATH_MSG_ERROR("Shouldn't use timestamped Jet collection (" << m_BtagTimeStamp << ") with older CDI file (" << m_bTaggingCalibrationFilePath << ")");
    return StatusCode::FAILURE;
  }
  if (m_useBtagging && (m_BtagTimeStamp.empty() && cdiYear>2017)) {
     ATH_MSG_ERROR("Should provide a BTag container timestamp (default = Btag.TimeStamp: 201810) to use with Jets and newer CDI files (" << m_bTaggingCalibrationFilePath << ")");
     return StatusCode::FAILURE;
  }
  // TrackJets
  if (m_useBtagging_trkJet && (!m_BtagTimeStamp_trkJet.empty() && !(m_BtagTimeStamp_trkJet.compare("201810")==0||m_BtagTimeStamp_trkJet.compare("201903")==0))) {
    ATH_MSG_ERROR("Only 201810 & 201903 are valid BTag container timestamps for TrackJets. Current = " << m_BtagTimeStamp_trkJet);
    return StatusCode::FAILURE;
  }
  if (m_useBtagging_trkJet && (!m_BtagTimeStamp_trkJet.empty() && cdiYear<2020)) {
    ATH_MSG_ERROR("Shouldn't use timestamped TrackJet collection (" << m_BtagTimeStamp_trkJet << ") with older CDI file (" << m_bTaggingCalibrationFilePath << ")");
    return StatusCode::FAILURE;
  }
  if (m_useBtagging_trkJet && (m_BtagTimeStamp_trkJet.empty() && cdiYear>2019)) {
     ATH_MSG_ERROR("Should provide a BTag container timestamp (default = Btag.TimeStamp_trkJet: 201903) to use with TrackJets and newer CDI files (" << m_bTaggingCalibrationFilePath << ")");
     return StatusCode::FAILURE;
  }

  // btagSelectionTool
  std::string jetcollBTag = jetcoll;
  if (jetcoll == "AntiKt4LCTopoJets") {
    ATH_MSG_WARNING("  *** HACK *** Treating LCTopoJets jets as EMTopo -- use at your own risk!");
    jetcollBTag = "AntiKt4EMTopoJets";
  }
  if (!m_BtagTimeStamp.empty()) jetcollBTag += "_BTagging" + m_BtagTimeStamp;

  if (m_useBtagging && !m_btagSelTool.isUserConfigured() && !m_BtagWP.empty()) {
    if (jetcollBTag.find("AntiKt4EMTopoJets") == std::string::npos && jetcollBTag.find("AntiKt4EMPFlowJets")==std::string::npos) {
      ATH_MSG_WARNING("** Only AntiKt4EMTopoJets and AntiKt4EMPFlowJets are supported with FTAG scale factors!");
        return StatusCode::FAILURE;
    } 

    toolName = "BTagSel_" + jetcollBTag + m_BtagTagger + m_BtagWP;

    m_btagSelTool.setTypeAndName("BTaggingSelectionTool/"+toolName);
    ATH_CHECK( m_btagSelTool.setProperty("TaggerName",     m_BtagTagger ) );
    ATH_CHECK( m_btagSelTool.setProperty("OperatingPoint", m_BtagWP  ) );
    ATH_CHECK( m_btagSelTool.setProperty("JetAuthor",      jetcollBTag   ) );
    ATH_CHECK( m_btagSelTool.setProperty("MinPt",          m_BtagMinPt   ) );
    ATH_CHECK( m_btagSelTool.setProperty("FlvTagCutDefinitionsFileName",  m_bTaggingCalibrationFilePath) );
    ATH_CHECK( m_btagSelTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_btagSelTool.retrieve() );
  } else if (m_btagSelTool.isUserConfigured()) ATH_CHECK( m_btagSelTool.retrieve() );
 

  if (m_useBtagging && !m_btagSelTool_OR.isUserConfigured() && !m_orBtagWP.empty()) {
    if (jetcoll != "AntiKt4EMTopoJets" && jetcoll != "AntiKt4EMPFlowJets") {
      ATH_MSG_WARNING("** Only AntiKt4EMTopoJets and AntiKt4EMPFlowJets are supported with FTAG scale factors!");
        return StatusCode::FAILURE;
    }

    toolName = "BTagSelOR_" + jetcollBTag + m_orBtagWP;
    m_btagSelTool_OR.setTypeAndName("BTaggingSelectionTool/"+toolName);
    ATH_CHECK( m_btagSelTool_OR.setProperty("TaggerName",     m_BtagTagger  ) );
    ATH_CHECK( m_btagSelTool_OR.setProperty("OperatingPoint", m_orBtagWP  ) );
    ATH_CHECK( m_btagSelTool_OR.setProperty("JetAuthor",      jetcollBTag   ) );
    ATH_CHECK( m_btagSelTool_OR.setProperty("MinPt",          m_BtagMinPt   ) );
    ATH_CHECK( m_btagSelTool_OR.setProperty("FlvTagCutDefinitionsFileName",  m_bTaggingCalibrationFilePath) );
    ATH_CHECK( m_btagSelTool_OR.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_btagSelTool_OR.retrieve() );
  } else if (m_btagSelTool_OR.isUserConfigured()) ATH_CHECK( m_btagSelTool_OR.retrieve() );
 

  std::string trkjetcoll = m_defaultTrackJets;
  std::string BTagColl_TrkJet = trkjetcoll;
  if (!m_BtagTimeStamp_trkJet.empty()) BTagColl_TrkJet += "_BTagging"+m_BtagTimeStamp_trkJet;
  
  if (m_useBtagging_trkJet && !m_btagSelTool_trkJet.isUserConfigured() && !m_BtagWP_trkJet.empty()) {
    if (trkjetcoll.find("AntiKt2PV0TrackJets")==std::string::npos && trkjetcoll.find("AntiKtVR30Rmax4Rmin02TrackJets")==std::string::npos) {
      ATH_MSG_WARNING("** Only AntiKt2PV0TrackJets and AntiKtVR30Rmax4Rmin02TrackJets are supported with FTAG scale factors!");
        return StatusCode::FAILURE;
    }

    toolName = "BTagSel_" + trkjetcoll + m_BtagTagger_trkJet + m_BtagWP_trkJet;
    
    m_btagSelTool_trkJet.setTypeAndName("BTaggingSelectionTool/"+toolName);
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("TaggerName",     m_BtagTagger_trkJet ) );
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("OperatingPoint", m_BtagWP_trkJet  ) );
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("MinPt",          m_BtagMinPt_trkJet  ) );
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("JetAuthor",      BTagColl_TrkJet   ) );
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("MinPt",          m_BtagMinPt_trkJet ) );
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("FlvTagCutDefinitionsFileName",  m_bTaggingCalibrationFilePath) );
    ATH_CHECK( m_btagSelTool_trkJet.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_btagSelTool_trkJet.retrieve() );
  } else if (m_btagSelTool_trkJet.isUserConfigured()) ATH_CHECK( m_btagSelTool_trkJet.retrieve() );
  

  // Set MCshowerType for FTAG MC/MC SFs
  std::string MCshowerID = "410470";                 // Powheg+Pythia8 (default)
  if (m_showerType == 1) MCshowerID = "410558";      // Powheg+Herwig7
  else if (m_showerType == 2) MCshowerID = "426131"; // Sherpa 2.1
  else if (m_showerType == 3) MCshowerID = "410250"; // Sherpa 221 or 222
  else if (m_showerType == 4) MCshowerID = "410464"; // aMC@NLO+Pythia8
  else if (m_showerType == 5) MCshowerID = "421152"; // Sherpa 228
  else if (m_showerType == 6) MCshowerID = "700122"; // Sherpa 2210 
  else if (m_showerType == 7) MCshowerID = "700122"; // Sherpa 2211 -> FallBack to Sherpa 2210
  else if (m_showerType == 8) MCshowerID = "700122"; // Sherpa 2212 -> FallBack to Sherpa 2210

  // btagEfficiencyTool
  if (m_useBtagging && !m_btagEffTool.isUserConfigured() && !m_BtagWP.empty()) {
    if (jetcoll != "AntiKt4EMTopoJets" && jetcoll != "AntiKt4EMPFlowJets") {
      ATH_MSG_WARNING("** Only AntiKt4EMTopoJets and AntiKt4EMPFlowJets are supported with FTAG scale factors!");
        return StatusCode::FAILURE;
    }

    // AntiKt4EMPFlowJets MC/MC SF isn't complete yet
    if (jetcollBTag == "AntiKt4EMPFlowJets" && MCshowerID == "426131") { // sherpa 2.1 isn't available
      ATH_MSG_WARNING ("MC/MC SFs for AntiKt4EMPFlowJets are not available yet! Falling back to AntiKt4EMTopoJets for the SFs.");
      jetcollBTag = "AntiKt4EMTopoJets";
    }

    // AntiKt4EMTopoJets MC/MC SF doesn't support sherpa 2.2.8 and sherpa 2.2.10
    if (jetcollBTag == "AntiKt4EMTopoJets" && (MCshowerID == "421152" || MCshowerID == "700122")) { // sherpa 2.1 isn't available
      ATH_MSG_WARNING ("MC/MC SFs for AntiKt4EMPFlowJets are not available yet! Falling back to Sherpa2.2.1 for the SFs.");
      MCshowerID == "410250";
    }

    toolName = "BTagSF_" + jetcollBTag + m_BtagTagger + m_BtagWP;
    m_btagEffTool.setTypeAndName("BTaggingEfficiencyTool/"+toolName);
    ATH_CHECK( m_btagEffTool.setProperty("TaggerName",     m_BtagTagger ) );
    ATH_CHECK( m_btagEffTool.setProperty("ScaleFactorFileName",  m_bTaggingCalibrationFilePath) );
    ATH_CHECK( m_btagEffTool.setProperty("OperatingPoint", m_BtagWP ) );
    ATH_CHECK( m_btagEffTool.setProperty("JetAuthor",      jetcollBTag ) );
    ATH_CHECK( m_btagEffTool.setProperty("MinPt",          m_BtagMinPt ) );
    ATH_CHECK( m_btagEffTool.setProperty("SystematicsStrategy", m_BtagSystStrategy ) );
    ATH_CHECK( m_btagEffTool.setProperty("EfficiencyBCalibrations",     MCshowerID   ));
    ATH_CHECK( m_btagEffTool.setProperty("EfficiencyCCalibrations",     MCshowerID   ));
    ATH_CHECK( m_btagEffTool.setProperty("EfficiencyTCalibrations",     MCshowerID   ));
    ATH_CHECK( m_btagEffTool.setProperty("EfficiencyLightCalibrations", MCshowerID   ));
    ATH_CHECK( m_btagEffTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_btagEffTool.retrieve() );
  } else ATH_CHECK( m_btagEffTool.retrieve() );
  

  if (m_useBtagging_trkJet && !m_btagEffTool_trkJet.isUserConfigured() && !m_BtagWP_trkJet.empty()) {
    if (trkjetcoll.find("AntiKt2PV0TrackJets")==std::string::npos && trkjetcoll.find("AntiKtVR30Rmax4Rmin02TrackJets")==std::string::npos) {
      ATH_MSG_WARNING("** Only AntiKt2PV0TrackJets and AntiKtVR30Rmax4Rmin02TrackJets are supported with FTAG scale factors!");
        return StatusCode::FAILURE;
    }

    toolName = "BTagSF_" + trkjetcoll;
    m_btagEffTool_trkJet.setTypeAndName("BTaggingEfficiencyTool/"+toolName);
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("TaggerName",     m_BtagTagger_trkJet ) );
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("ScaleFactorFileName",  m_bTaggingCalibrationFilePath) );
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("OperatingPoint", m_BtagWP_trkJet ) );
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("JetAuthor",      BTagColl_TrkJet ) );
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("MinPt",          m_BtagMinPt_trkJet ) );
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("SystematicsStrategy", m_BtagSystStrategy ) );
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyBCalibrations",     MCshowerID   ));
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyCCalibrations",     MCshowerID   ));
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyTCalibrations",     MCshowerID   ));
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyLightCalibrations", MCshowerID   ));
    ATH_CHECK( m_btagEffTool_trkJet.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_btagEffTool_trkJet.retrieve() );
  } else if (m_btagEffTool_trkJet.isUserConfigured()) ATH_CHECK( m_btagEffTool_trkJet.retrieve() );


///////////////////////////////////////////////////////////////////////////////////////////
// Initialise MET tools

  if (!m_metMaker.isUserConfigured()) {
    toolName = m_doFwdJVT ? m_metJetSelection+"_fJVT" : m_metJetSelection+"_NOfJVT";
    m_metMaker.setTypeAndName("met::METMaker/METMaker_ST_"+toolName);

    ATH_CHECK( m_metMaker.setProperty("ORCaloTaggedMuons", m_metRemoveOverlappingCaloTaggedMuons) );
    ATH_CHECK( m_metMaker.setProperty("DoSetMuonJetEMScale", m_metDoSetMuonJetEMScale) );
    ATH_CHECK( m_metMaker.setProperty("DoRemoveMuonJets", m_metDoRemoveMuonJets) );
    ATH_CHECK( m_metMaker.setProperty("UseGhostMuons", m_metUseGhostMuons) );
    ATH_CHECK( m_metMaker.setProperty("DoMuonEloss", m_metDoMuonEloss) );
    ATH_CHECK( m_metMaker.setProperty("GreedyPhotons", m_metGreedyPhotons) );
    ATH_CHECK( m_metMaker.setProperty("VeryGreedyPhotons", m_metVeryGreedyPhotons) );
    ATH_CHECK( m_metMaker.setProperty("DoMuonPFlowBugfix", m_metDoMuonPFlowBugFix) );

    // set the jet selection if default empty string is overridden through config file
    if (m_metJetSelection.size()) {
      ATH_CHECK( m_metMaker.setProperty("JetSelection", m_metJetSelection) );
    }
    if (m_doFwdJVT || m_metJetSelection == "Tenacious" || m_metJetSelection == "TenaciousJVT641") {
      ATH_CHECK( m_metMaker.setProperty("JetRejectionDec", "passFJvt") );
    }
    if (m_jetInputType == xAOD::JetInput::EMPFlow) {
      ATH_CHECK( m_metMaker.setProperty("DoPFlow", true) );
    }

    ATH_CHECK( m_metMaker.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_metMaker.retrieve() );
  } else ATH_CHECK( m_metMaker.retrieve() );


  if (!m_metSystTool.isUserConfigured()) {
    m_metSystTool.setTypeAndName("met::METSystematicsTool/METSystTool");
    ATH_CHECK( m_metSystTool.setProperty("ConfigPrefix", m_metsysConfigPrefix) );

    if (m_trkMETsyst && m_caloMETsyst){
      ATH_MSG_ERROR( "Can only have CST *or* TST configured for MET maker.  Please unset either METDoCaloSyst or METDoTrkSyst in your config file" );
      return StatusCode::FAILURE;
    }

    if (m_trkMETsyst) {
      ATH_CHECK( m_metSystTool.setProperty("ConfigSoftCaloFile", "") );
      ATH_CHECK( m_metSystTool.setProperty("ConfigSoftTrkFile", "TrackSoftTerms.config") );
      if (m_jetInputType == xAOD::JetInput::EMPFlow) {
        ATH_CHECK( m_metSystTool.setProperty("ConfigSoftTrkFile", "TrackSoftTerms-pflow.config") );
      }
    }

    if (m_caloMETsyst) {
      ATH_MSG_WARNING( "CST is no longer recommended by Jet/MET group");
      ATH_CHECK( m_metSystTool.setProperty("ConfigSoftTrkFile", "") );
      // Recommendations from a thread with TJ.  CST is not officially supported, but might be used for cross-checks
      ATH_CHECK( m_metSystTool.setProperty("DoIsolMuonEloss",true) );
      ATH_CHECK( m_metSystTool.setProperty("DoMuonEloss",true) );
      if ("AntiKt4EMTopoJets"==jetcoll) {
        // Recommendation from TJ: if we are using EM topo jets, make sure the clusters are considered at LC scale
        ATH_CHECK( m_metSystTool.setProperty("JetConstitScaleMom","JetLCScaleMomentum") );
      }
    } 

    if (m_trkJetsyst) {
      ATH_CHECK( m_metSystTool.setProperty("ConfigJetTrkFile", "JetTrackSyst.config") );
    }

    ATH_CHECK( m_metSystTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_metSystTool.retrieve());
  } else ATH_CHECK( m_metSystTool.retrieve());
 

  if (!m_metSignif.isUserConfigured()) {
    // See https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MetSignificance 
    m_metSignif.setTypeAndName("met::METSignificance/metSignificance_"+jetname);
    ATH_CHECK( m_metSignif.setProperty("SoftTermParam", m_softTermParam) );
    ATH_CHECK( m_metSignif.setProperty("TreatPUJets", m_treatPUJets) );
    ATH_CHECK( m_metSignif.setProperty("DoPhiReso", m_doPhiReso) );
    ATH_CHECK( m_metSignif.setProperty("IsAFII", isAtlfast()) ); 
    if(jetname == "AntiKt4EMTopo" || jetname =="AntiKt4EMPFlow"){
      ATH_CHECK( m_metSignif.setProperty("JetCollection", jetname) );
    } else {
      ATH_MSG_WARNING("Object-based METSignificance recommendations only exist for EMTopo and PFlow, falling back to AntiKt4EMTopo");
      ATH_CHECK( m_metSignif.setProperty("JetCollection", "AntiKt4EMTopo") );
    }
    ATH_CHECK( m_metSignif.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_metSignif.retrieve() );
  } else ATH_CHECK( m_metSignif.retrieve() );
  

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise trigger tools

  if (!m_trigDecTool.isUserConfigured()) {
    m_trigConfTool.setTypeAndName("TrigConf::xAODConfigTool/xAODConfigTool");
    ATH_CHECK(m_trigConfTool.retrieve() );

    // The decision tool
    m_trigDecTool.setTypeAndName("Trig::TrigDecisionTool/TrigDecisionTool");
    ATH_CHECK( m_trigDecTool.setProperty("ConfigTool", m_trigConfTool.getHandle()) );
    ATH_CHECK( m_trigDecTool.setProperty("TrigDecisionKey", "xTrigDecision") );
    ATH_CHECK( m_trigDecTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigDecTool.retrieve() );
  } else  ATH_CHECK( m_trigDecTool.retrieve() );
  

  if (!m_trigMatchingTool.isUserConfigured()) {
    if (!m_upstreamTriggerMatching){
      m_trigMatchingTool.setTypeAndName("Trig::MatchingTool/TrigMatchingTool");
      ATH_CHECK( m_trigMatchingTool.setProperty("TrigDecisionTool", m_trigDecTool.getHandle()) );
      ATH_CHECK( m_trigMatchingTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_trigMatchingTool.retrieve() );
    } else {
      m_trigMatchingTool.setTypeAndName("Trig::MatchFromCompositeTool/TrigMatchFromCompositeTool");
      ATH_CHECK( m_trigMatchingTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_trigMatchingTool.setProperty("InputPrefix", m_trigMatchingPrefix) );
      ATH_CHECK( m_trigMatchingTool.retrieve() );
    }
  } else  ATH_CHECK( m_trigMatchingTool.retrieve() );
  

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise trigGlobalEfficiencyCorrection tool

  if (!m_trigGlobalEffCorrTool_diLep.isUserConfigured()) {

    std::string no2e17("");
    if (m_trig2017combination_diLep.find("||2e17_lhvloose_nod0_L12EM15VHI") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_diLep.find("||2e17_lhvloose_nod0_L12EM15VHI");
      no2e17 = m_trig2017combination_diLep.substr(0, pos_2e17) + m_trig2017combination_diLep.substr(pos_2e17+31, m_trig2017combination_diLep.size()); 
    } else if (m_trig2017combination_diLep.find("2e17_lhvloose_nod0_L12EM15VHI||") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_diLep.find("2e17_lhvloose_nod0_L12EM15VHI||");
      no2e17 = m_trig2017combination_diLep.substr(0, pos_2e17) + m_trig2017combination_diLep.substr(pos_2e17+31, m_trig2017combination_diLep.size());
    } else {
      no2e17 = m_trig2017combination_diLep;
    }
    ATH_MSG_DEBUG( "TrigGlobalEfficiencyCorrectionTool/TrigGlobal_diLep: no2e17 trigger string: " << no2e17 );

    std::map<std::string,std::string> triggers_diLep;
    triggers_diLep["2015"] = m_trig2015combination_diLep;
    triggers_diLep["2016"] = m_trig2016combination_diLep;
    triggers_diLep["324320-326695"] = m_trig2017combination_diLep; // 2017 before accidental prescale of L12EM15VHI
    triggers_diLep["326834-328393"] = no2e17;                      // 2017 during accidental prescale
    triggers_diLep["329385-340453"] = m_trig2017combination_diLep; // 2017 after accidental prescale
    triggers_diLep["2018"] = m_trig2018combination_diLep;
    
    m_trigGlobalEffCorrTool_diLep.setTypeAndName("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_diLep");
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("ElectronEfficiencyTools", m_elecTrigEffTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("ElectronScaleFactorTools", m_elecTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("MuonTools", m_muonTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("TriggerCombination", triggers_diLep) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("TriggerMatchingTool", m_trigMatchingTool.getHandle()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("ListOfLegsPerTool", m_legsPerTool) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("NumberOfToys", 250) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.initialize() );
  } else  ATH_CHECK( m_trigGlobalEffCorrTool_diLep.initialize() );
 

  if (!m_trigGlobalEffCorrTool_multiLep.isUserConfigured()) {

    std::string no2e17("");
    if (m_trig2017combination_multiLep.find("||2e17_lhvloose_nod0_L12EM15VHI") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_multiLep.find("||2e17_lhvloose_nod0_L12EM15VHI");
      no2e17 = m_trig2017combination_multiLep.substr(0, pos_2e17) + m_trig2017combination_multiLep.substr(pos_2e17+31, m_trig2017combination_multiLep.size()); 
    } else if (m_trig2017combination_multiLep.find("2e17_lhvloose_nod0_L12EM15VHI||") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_multiLep.find("2e17_lhvloose_nod0_L12EM15VHI||");
      no2e17 = m_trig2017combination_multiLep.substr(0, pos_2e17) + m_trig2017combination_multiLep.substr(pos_2e17+31, m_trig2017combination_multiLep.size());
    } else {
      no2e17 = m_trig2017combination_multiLep;
    }
    ATH_MSG_DEBUG( "TrigGlobalEfficiencyCorrectionTool/TrigGlobal_multiLep: no2e17 trigger string: " << no2e17 );

    std::map<std::string,std::string> triggers_multiLep;
    triggers_multiLep["2015"] = m_trig2015combination_multiLep;
    triggers_multiLep["2016"] = m_trig2016combination_multiLep;
    triggers_multiLep["324320-326695"] = m_trig2017combination_multiLep; // 2017 before accidental prescale of L12EM15VHI
    triggers_multiLep["326834-328393"] = no2e17;                         // 2017 during accidental prescale
    triggers_multiLep["329385-340453"] = m_trig2017combination_multiLep; // 2017 after accidental prescale
    triggers_multiLep["2018"] = m_trig2018combination_multiLep;

    m_trigGlobalEffCorrTool_multiLep.setTypeAndName("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_multiLep");
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("ElectronEfficiencyTools", m_elecTrigEffTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("ElectronScaleFactorTools", m_elecTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("MuonTools", m_muonTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("TriggerCombination", triggers_multiLep) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("TriggerMatchingTool", m_trigMatchingTool.getHandle()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("ListOfLegsPerTool", m_legsPerTool) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("NumberOfToys", 250) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.initialize() );
  } else ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.initialize() );
  

  if (!m_trigGlobalEffCorrTool_diPhoton.isUserConfigured()) {
    m_trigGlobalEffCorrTool_diPhoton.setTypeAndName("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_diPhoton");
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("PhotonEfficiencyTools", m_photonTrigEffTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("PhotonScaleFactorTools", m_photonTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2015", m_trig2015combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2016", m_trig2016combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2017", m_trig2017combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2018", m_trig2018combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("ListOfLegsPerTool", m_legsPerTool_ph) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("NumberOfToys", 250) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.initialize() );
  } else  ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.initialize() );
 

// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise Isolation Correction Tool

  if ( !m_isoCorrTool.isUserConfigured() ) {
    m_isoCorrTool.setTypeAndName("CP::IsolationCorrectionTool/IsoCorrTool");
    ATH_CHECK( m_isoCorrTool.setProperty( "IsMC", !isData()) );
    ATH_CHECK( m_isoCorrTool.setProperty( "AFII_corr", isAtlfast()) );
    ATH_CHECK( m_isoCorrTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_isoCorrTool.retrieve() );
  } else  ATH_CHECK( m_isoCorrTool.retrieve() );
  

// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise Isolation Tool
  if (!m_isoTool.isUserConfigured()) {
    m_isoTool.setTypeAndName("CP::IsolationSelectionTool/IsoTool");
    ATH_CHECK( m_isoTool.setProperty("ElectronWP", m_eleIso_WP.empty()    ? "FCLoose" : m_eleIso_WP) );
    ATH_CHECK( m_isoTool.setProperty("MuonWP",     m_muIso_WP.empty()     ? "Loose_VarRad" : m_muIso_WP) );
    ATH_CHECK( m_isoTool.setProperty("PhotonWP",   m_photonIso_WP.empty() ? "FixedCutTight" : m_photonIso_WP ) );
    ATH_CHECK( m_isoTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_isoTool.retrieve() );
  } else  ATH_CHECK( m_isoTool.retrieve() );
 
  if (!m_isoToolLowPtPLV.isUserConfigured()) {
    m_isoToolLowPtPLV.setTypeAndName("CP::IsolationLowPtPLVTool/IsoToolLowPtPLV");
    ATH_CHECK( m_isoToolLowPtPLV.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_isoToolLowPtPLV.retrieve() );
  } else  ATH_CHECK( m_isoToolLowPtPLV.retrieve() );
 

  if (!m_isoBaselineTool.isUserConfigured()) {
    m_isoBaselineTool.setTypeAndName("CP::IsolationSelectionTool/IsoBaselineTool");
    ATH_CHECK( m_isoBaselineTool.setProperty("ElectronWP", m_eleBaselineIso_WP.empty()    ? "FCLoose" : m_eleBaselineIso_WP    ) );
    ATH_CHECK( m_isoBaselineTool.setProperty("MuonWP",     m_muBaselineIso_WP.empty()     ? "Loose_VarRad" : m_muBaselineIso_WP     ) );
    ATH_CHECK( m_isoBaselineTool.setProperty("PhotonWP",   m_photonBaselineIso_WP.empty() ? "FixedCutTight" : m_photonBaselineIso_WP ) );
    ATH_CHECK( m_isoBaselineTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_isoBaselineTool.retrieve() );
  } else ATH_CHECK( m_isoBaselineTool.retrieve() );
  

  if (!m_isoHighPtTool.isUserConfigured()) {
    m_isoHighPtTool.setTypeAndName("CP::IsolationSelectionTool/IsoHighPtTool");
    ATH_CHECK( m_isoHighPtTool.setProperty("ElectronWP", m_eleIsoHighPt_WP.empty() ? "FCLoose" : m_eleIsoHighPt_WP) );
    ATH_CHECK( m_isoHighPtTool.setProperty("MuonWP",     m_muIsoHighPt_WP.empty()  ? "Loose_VarRad" : m_muIsoHighPt_WP ) );
    ATH_CHECK( m_isoHighPtTool.setProperty("PhotonWP",   m_photonIso_WP.empty()    ? "FixedCutTight" : m_photonIso_WP ) );
    ATH_CHECK( m_isoHighPtTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_isoHighPtTool.retrieve() );
  } else ATH_CHECK( m_isoHighPtTool.retrieve() );
  

// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise IsolationCloseByCorrectionTool Tool
  if (!m_isoCloseByTool.isUserConfigured()) {
    m_isoCloseByTool.setTypeAndName("CP::IsolationCloseByCorrectionTool/IsoCloseByTool");
    // Actually we could debate about what is the proper tool to choose if the users have different baseline & signal islation WP's
    ATH_CHECK( m_isoCloseByTool.setProperty("IsolationSelectionTool", m_useSigLepForIsoCloseByOR ? m_isoTool : m_isoBaselineTool));
    ATH_CHECK( m_isoCloseByTool.setProperty("PassoverlapDecorator", m_IsoCloseByORpassLabel) );
    ATH_CHECK( m_isoCloseByTool.setProperty("SelectionDecorator", m_useSigLepForIsoCloseByOR ? "signal" : "baseline") );
    // Make this propery configurable as well?
    ATH_CHECK( m_isoCloseByTool.setProperty("BackupPrefix", "ORIG") );
    // The isolation selection decorator is updated as well by the tool
    ATH_CHECK( m_isoCloseByTool.setProperty("IsolationSelectionDecorator", "isol") );

    ATH_CHECK( m_isoCloseByTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_isoCloseByTool.retrieve() );
  } else  ATH_CHECK( m_isoCloseByTool.retrieve() );
  

// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise Overlap Removal Tool
  if ( m_orToolbox.masterTool.empty() ){

    // set up the master tool
    std::string suffix = "";
    if (m_orDoTau) suffix += "Tau";
    if (m_orDoPhoton) suffix += "Gamma";
    if (m_orDoBjet) suffix += "Bjet";
    std::string toolName = "ORTool" + suffix;
    ATH_MSG_INFO("SUSYTools: Autoconfiguring " << toolName);

    std::string bJetLabel = "";
    //overwrite lepton flags if the global is false (yes?)
    if (!m_orDoBjet || !m_useBtagging) {
      m_orDoElBjet = false;
      m_orDoMuBjet = false;
      m_orDoTauBjet = false;
    }
    if (m_orDoElBjet || m_orDoMuBjet || m_orDoTauBjet) {
      bJetLabel = "bjet_loose";
    }

    // Set the generic flags
    ORUtils::ORFlags orFlags(toolName, m_orInputLabel, "passOR");
    orFlags.bJetLabel      = bJetLabel;
    orFlags.boostedLeptons = (m_orDoBoostedElectron || m_orDoBoostedMuon);
    orFlags.outputPassValue = true;
    orFlags.linkOverlapObjects = m_orLinkOverlapObjects;
    if (m_jetInputType == xAOD::JetInput::EMPFlow) orFlags.doMuPFJetOR = true;
    if (m_orDoElEl) {
      orFlags.doEleEleOR = true;
    } else orFlags.doEleEleOR = false;
    orFlags.doElectrons = true;
    orFlags.doMuons = true;
    orFlags.doJets = true;
    orFlags.doTaus = m_orDoTau;
    orFlags.doPhotons = m_orDoPhoton;
    orFlags.doFatJets = m_orDoFatjets;

    //set up all recommended tools
    ATH_CHECK( ORUtils::recommendedTools(orFlags, m_orToolbox));

    // We don't currently have a good way to determine here which object
    // definitions are disabled, so we currently just configure all overlap
    // tools and disable the pointer safety checks
    ATH_CHECK( m_orToolbox.masterTool.setProperty("RequireExpectedPointers", false) );
    ATH_CHECK( m_orToolbox.masterTool.setProperty("OutputLevel", this->msg().level()) );

    // If using p-flow jets, set the debug error message for the specific p-flow jet removal (to be used in addition to the standard muon-jet OR)
    if (m_jetInputType == xAOD::JetInput::EMPFlow){
      ATH_CHECK( m_orToolbox.muPFJetORT.setProperty("OutputLevel", this->msg().level()) );
    }

    // Override boosted OR sliding cone options
    ATH_CHECK( m_orToolbox.eleJetORT.setProperty("UseSlidingDR", m_orDoBoostedElectron) );
    ATH_CHECK( m_orToolbox.muJetORT.setProperty("UseSlidingDR", m_orDoBoostedMuon) );

    //add custom tau-jet OR tool
    if(m_orDoTau){
      m_orToolbox.tauJetORT.setTypeAndName("ORUtils::TauJetOverlapTool/" + orFlags.masterName + ".TauJetORT");
      ATH_CHECK( m_orToolbox.tauJetORT.setProperty("BJetLabel", m_orDoTauBjet?bJetLabel:"") );
    }

    // override sliding cone params if sliding dR is on and the user-provided parameter values are non-negative
    if (m_orDoBoostedElectron) {
      if (m_orBoostedElectronC1 > 0) ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRC1", m_orBoostedElectronC1) );
      if (m_orBoostedElectronC2 > 0) ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRC2", m_orBoostedElectronC2) );
      if (m_orBoostedElectronMaxConeSize > 0) ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRMaxCone", m_orBoostedElectronMaxConeSize) );
    }
    if (m_orDoBoostedMuon) {
      if (m_orBoostedMuonC1 > 0) ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRC1", m_orBoostedMuonC1) );
      if (m_orBoostedMuonC2 > 0) ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRC2", m_orBoostedMuonC2) );
      if (m_orBoostedMuonMaxConeSize > 0) ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRMaxCone", m_orBoostedMuonMaxConeSize) );
    }

    // and switch off lep-bjet check if not requested
    if (!m_orDoElBjet) {
      ATH_CHECK(m_orToolbox.eleJetORT.setProperty("BJetLabel", ""));
    }
    if (!m_orDoMuBjet) {
      ATH_CHECK(m_orToolbox.muJetORT.setProperty("BJetLabel", ""));
    }

    // propagate the mu-jet ghost-association option which might be set by the user (default is true)
    ATH_CHECK(m_orToolbox.muJetORT.setProperty("UseGhostAssociation", m_orDoMuonJetGhostAssociation));

    // propagate mu-jet OR settings if requested
    ATH_CHECK(m_orToolbox.muJetORT.setProperty("ApplyRelPt", m_orApplyRelPt) );
    if(m_orApplyRelPt){
      if (m_orMuJetPtRatio > 0)    ATH_CHECK(m_orToolbox.muJetORT.setProperty("MuJetPtRatio", m_orMuJetPtRatio) );
      if (m_orMuJetTrkPtRatio > 0) ATH_CHECK(m_orToolbox.muJetORT.setProperty("MuJetTrkPtRatio", m_orMuJetTrkPtRatio) );
    }
    if (m_orMuJetInnerDR > 0)    ATH_CHECK(m_orToolbox.muJetORT.setProperty("InnerDR", m_orMuJetInnerDR) );

    // propagate the calo muon setting for EleMuORT
    ATH_CHECK(m_orToolbox.eleMuORT.setProperty("RemoveCaloMuons", m_orRemoveCaloMuons) );
    
    // Use electron-muon DR matching to remove electrons within DR <  0.01 of Muons.
    if (m_orDoElMu){
      ATH_CHECK(m_orToolbox.eleMuORT.setProperty("UseDRMatching", m_orDoElMu) );
    }
    
    // propagate the fatjets OR settings
    if(m_orDoFatjets){
      if(m_EleFatJetDR>0) ATH_CHECK(m_orToolbox.eleFatJetORT.setProperty("DR", m_EleFatJetDR));
      if(m_JetFatJetDR>0) ATH_CHECK(m_orToolbox.jetFatJetORT.setProperty("DR", m_JetFatJetDR));
    }

    // Make sure that we deal with prorities correctly
    ATH_CHECK(m_orToolbox.eleJetORT.setProperty("EnableUserPriority", true));
    ATH_CHECK(m_orToolbox.muJetORT.setProperty("EnableUserPriority", true));
    if (m_orDoTau) ATH_CHECK(m_orToolbox.tauJetORT.setProperty("EnableUserPriority", true));
    if (m_orDoPhoton) ATH_CHECK(m_orToolbox.phoJetORT.setProperty("EnableUserPriority", true));

    if ( m_orPhotonFavoured ) {
       ATH_CHECK(m_orToolbox.phoEleORT.setProperty("SwapContainerPrecedence", true));
       ATH_CHECK(m_orToolbox.phoMuORT.setProperty("SwapContainerPrecedence", true));
    }

    if (!m_orDoEleJet){
      // Disable the electron removal part of e-j overlap removal
      ATH_CHECK( m_orToolbox.eleJetORT.setProperty("OuterDR",-1.) );
      ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRMaxCone",-1.) );
    }
    if (!m_orDoMuonJet){
      // Disable the muon removal part of m-j overlap removal
      ATH_CHECK( m_orToolbox.muJetORT.setProperty("OuterDR",-1.) );
      ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRMaxCone",-1.) );
    }

    ATH_CHECK( m_orToolbox.initialize() );

  } 
  // Done with the OR toolbox setup!

// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise PMG Tools
  if (!m_pmgSHnjetWeighter.isUserConfigured()) {
    m_pmgSHnjetWeighter.setTypeAndName("PMGTools::PMGSherpa22VJetsWeightTool/PMGSHVjetReweighter");
    ATH_CHECK( m_pmgSHnjetWeighter.setProperty( "TruthJetContainer", "AntiKt4TruthJets"));
    ATH_CHECK( m_pmgSHnjetWeighter.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_pmgSHnjetWeighter.retrieve());
  } else  ATH_CHECK( m_pmgSHnjetWeighter.retrieve());


  if (!m_pmgSHnjetWeighterWZ.isUserConfigured()) {
    m_pmgSHnjetWeighterWZ.setTypeAndName("PMGTools::PMGSherpa22VJetsWeightTool/PMGSHVjetReweighterWZ");
    ATH_CHECK( m_pmgSHnjetWeighterWZ.setProperty( "TruthJetContainer", "AntiKt4TruthWZJets"));
    ATH_CHECK( m_pmgSHnjetWeighterWZ.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_pmgSHnjetWeighterWZ.retrieve() );
  } else  ATH_CHECK( m_pmgSHnjetWeighterWZ.retrieve() );
  

  // prevent these initialiation snippets from being run again
  m_subtool_init = true;

  ATH_MSG_INFO("Done initialising SUSYTools");

  return StatusCode::SUCCESS;
}
