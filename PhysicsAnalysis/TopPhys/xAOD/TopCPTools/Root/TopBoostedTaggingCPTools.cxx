/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TopCPTools/TopBoostedTaggingCPTools.h"

#include <map>
#include <string>
#include <algorithm>
#include <iterator>

// Top includes
#include "TopConfiguration/TopConfig.h"
#include "TopConfiguration/Tokenize.h"
#include "TopEvent/EventTools.h"

// JetUncertaintiesTool for the tagger SFs
#include "JetUncertainties/JetUncertaintiesTool.h"

// Boosted tagging includes
#include "BoostedJetTaggers/SmoothedWZTagger.h"
#include "BoostedJetTaggers/JSSWTopTaggerDNN.h"

namespace top {
  BoostedTaggingCPTools::BoostedTaggingCPTools(const std::string& name) :
    asg::AsgTool(name) {
    declareProperty("config", m_config);
  }

  StatusCode BoostedTaggingCPTools::initialize() {
    ATH_MSG_INFO("top::BoostedTaggingCPTools initialize...");

    if (m_config->isTruthDxAOD()) {
      ATH_MSG_INFO("top::BoostedTaggingCPTools: no need to initialise anything on truth DxAOD");
      return StatusCode::SUCCESS;
    }

    if (!m_config->useLargeRJets()) {
      ATH_MSG_INFO("top::BoostedTaggingCPTools: no need to initialise anything since not using large-R jets");
      return StatusCode::SUCCESS;
    }


    const std::vector<std::pair<std::string, std::string> >& boostedJetTaggers = m_config->boostedJetTaggers();
    if (boostedJetTaggers.size() == 0) {
      ATH_MSG_INFO("top::BoostedTaggingCPTools: boostedJetTagging not set. No need to initialise anything.");
      return StatusCode::SUCCESS;
    }

    initTaggersMaps();
    initSFsMaps();

    top::check(std::find(std::begin(m_jetCollections),std::end(m_jetCollections),m_config->sgKeyLargeRJets())!=std::end(m_jetCollections),
      "Error in BoostedTaggingCPTools: boosted jet taggers are not available for this large-R jet collection.");

    for (const std::pair<std::string, std::string>& name : boostedJetTaggers) {
      const std::string& taggerType = name.first;
      const std::string& taggerName = name.second;

      top::check(m_taggersConfigs.find(taggerName) != std::end(m_taggersConfigs),
                 ("Error in BoostedTaggingCPTools: Unknown tagger in the config file: " + taggerName).c_str());

      top::check(std::find(m_taggersTypes.begin(), m_taggersTypes.end(), taggerType) != m_taggersTypes.end(),
                 "Error in BoostedTaggingCPTools: Unknown TAGGER_TYPE.");
      if (taggerType == "JSSWTopTaggerDNN")
        top::check(ASG_MAKE_ANA_TOOL(m_taggers[taggerName], JSSWTopTaggerDNN), "Failed to make " + taggerName);
      else if (taggerType == "SmoothedWZTagger")
        top::check(ASG_MAKE_ANA_TOOL(m_taggers[taggerName], SmoothedWZTagger), "Failed to make " + taggerName);

      m_taggers[taggerName].setName(taggerName);
      top::check(m_taggers[taggerName].setProperty("ConfigFile", m_taggersConfigs[taggerName]),
                 "Failed to set ConfigFile for " + taggerName);
      top::check(m_taggers[taggerName].setProperty("ContainerName", m_config->sgKeyLargeRJets()),
                 "Failed to set ContainerName " + taggerName);
      top::check(m_taggers[taggerName].setProperty("CalibArea", m_taggersCalibAreas[taggerType]),
                 "Failed to set CalibArea for " + taggerName);
      // not all BJT taggers implement IsMC property -- only those that have calibration SFs
      // so we have to check here that we try to set this property only where applicable
      if (taggerType == "JSSWTopTaggerDNN" || taggerType == "SmoothedWZTagger") {
        top::check(m_taggers[taggerName].setProperty("IsMC", m_config->isMC()),
                   "Failed to set IsMC for " + taggerName);
      }
      top::check(m_taggers[taggerName].initialize(), "Failed to initialize " + taggerName);

      // initialize SF uncertainty tools for supported WPs
      if (m_config->isMC()) {
        std::string jet_def = m_config->sgKeyLargeRJets();
        jet_def.erase(jet_def.length() - 4); // jet collection name sans 'Jets' suffix

        std::string mc_type = "MC20";

        // If we are in Run 3, we should use MC21 instead...
        if (m_config->isRun3()) mc_type = "MC21";

        const std::string name = "JetSFuncert_" + taggerName;
        try {
          const std::string& cfg = m_taggerSFsConfigs.at(taggerName);
          JetUncertaintiesTool* jet_SF_tmp = new JetUncertaintiesTool(name);

          top::check(jet_SF_tmp->setProperty("JetDefinition", jet_def), "Failed to set JetDefinition for " + name);
          top::check(jet_SF_tmp->setProperty("MCType", mc_type), "Failed to set MCType for " + name);
          top::check(jet_SF_tmp->setProperty("ConfigFile", cfg), "Failed to set ConfigFile for " + name);
          top::check(jet_SF_tmp->setProperty("IsData", false), "Failed to set IsData for " + name);
          top::check(jet_SF_tmp->initialize(), "Failed to initialize " + name);
          m_tagSFuncertTool[taggerName] = jet_SF_tmp;
          m_config->setCalibBoostedJetTagger(taggerName, m_taggerSFsNames[taggerName]);
        } catch (std::out_of_range& e) {
          // skip taggers which do not yet have SFs available
          ATH_MSG_WARNING("Boosted jet tagger " + taggerName + " is not yet calibrated! No SFs are available.");
        }
      }
    }

    return StatusCode::SUCCESS;
  }
  
  void BoostedTaggingCPTools::initTaggersMaps() {

    // Calib areas
    m_taggersCalibAreas["JSSWTopTaggerANN"] = "JSSWTopTaggerANN/Rel21/March2023/";
    m_taggersCalibAreas["JSSWTopTaggerDNN"] = "JSSWTopTaggerDNN/Rel21/February2022/";
    m_taggersCalibAreas["SmoothedWZTagger"] = "SmoothedWZTaggers/Rel21/February2022/";

    // Supported tagger types
    // (ANN tagger is not yet ported to R22)
    m_taggersTypes = {
      // "JSSWTopTaggerANN",
      "JSSWTopTaggerDNN",
      "SmoothedWZTagger",
    };

    // Supported jet collections
    m_jetCollections = {
      "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
    };


    auto setConfig = [this](const std::string& tagger, const std::string& configName) {
      m_taggersConfigs[tagger] = configName;
    };

    if (m_config->sgKeyLargeRJets() == "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets") {
      // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BoostedJetTaggingRecommendationFullRun2#UFO_jets

      // Top taggers
      setConfig("DNNTaggerTopQuarkContained50", "DNNTagger_AntiKt10UFOSD_TopContained50_Oct30.dat");
      setConfig("DNNTaggerTopQuarkContained80", "DNNTagger_AntiKt10UFOSD_TopContained80_Oct30.dat");
      setConfig("DNNTaggerTopQuarkInclusive50", "DNNTagger_AntiKt10UFOSD_TopInclusive50_Oct30.dat");
      setConfig("DNNTaggerTopQuarkInclusive80", "DNNTagger_AntiKt10UFOSD_TopInclusive80_Oct30.dat");

      // WZ taggers
      // (ANN tagger is not yet ported to R22)
      // setConfig("ANNTaggerWZContained50", "JSSANN50Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat");
      // setConfig("ANNTaggerWZContained60", "JSSANN60Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat");
      // setConfig("ANNTaggerWZContained70", "JSSANN70Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat");
      // setConfig("ANNTaggerWZContained80", "JSSANN80Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat");
      // setConfig("ANNTaggerWZContained90", "JSSANN90Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat");

      setConfig("SmoothWContained50", "SmoothedContainedWTagger_AntiKt10UFOCSSKSoftDrop_FixedSignalEfficiency50_20220221.dat");
      setConfig("SmoothWContained80", "SmoothedContainedWTagger_AntiKt10UFOCSSKSoftDrop_FixedSignalEfficiency80_20220221.dat");
      setConfig("SmoothZContained50", "SmoothedContainedZTagger_AntiKt10UFOCSSKSoftDrop_FixedSignalEfficiency50_20220221.dat");
      setConfig("SmoothZContained80", "SmoothedContainedZTagger_AntiKt10UFOCSSKSoftDrop_FixedSignalEfficiency80_20220221.dat");

      // (DNN tagger is not yet ported to R22)
      // setConfig("DNNTaggerWContained50", "JSSDNN50Tagger_AntiKt10UFOCSSKSoftDrop_Jan22.dat");
      // setConfig("DNNTaggerWContained80", "JSSDNN80Tagger_AntiKt10UFOCSSKSoftDrop_Jan22.dat");
    }
  }

  void BoostedTaggingCPTools::initSFsMaps() {
    // Here we initialize SFs maps

    // This lambda function is universal for all taggers
    // (Commented out until SFs are available to suppress compilation warning)
    // auto setMaps = [this](const std::string& configPath, const std::string& tagger, const std::string& configName) {
    //   m_taggerSFsConfigs[tagger] = configPath + "/" + configName;
    //   m_taggerSFsNames[tagger] = tagger + "_SF";
    // };

    if (m_config->sgKeyLargeRJets() == "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets") {
      ATH_MSG_WARNING("No Large-R jet tagging scale factors available yet!");

      // Leaving old R21 LCTopo-configs here for now to have some idea on how to apply the settings later on
      // const std::string configPath="rel21/Fall2020";
      // //Top taggers
      // setMaps(configPath,"DNNTaggerTopQuarkContained50","R10_SF_LCTopo_TopTagContained_SigEff50.config");
      // setMaps(configPath,"DNNTaggerTopQuarkContained80","R10_SF_LCTopo_TopTagContained_SigEff80.config");
      // setMaps(configPath,"DNNTaggerTopQuarkInclusive50","R10_SF_LCTopo_TopTagInclusive_SigEff50.config");
      // setMaps(configPath,"DNNTaggerTopQuarkInclusive80","R10_SF_LCTopo_TopTagInclusive_SigEff80.config");
      // //WZ taggers
      // setMaps(configPath,"SmoothWContained50","R10_SF_LCTopo_WTag_SigEff50.config");
      // setMaps(configPath,"SmoothWContained80","R10_SF_LCTopo_WTag_SigEff80.config");
    }
  }

}  // namespace top
