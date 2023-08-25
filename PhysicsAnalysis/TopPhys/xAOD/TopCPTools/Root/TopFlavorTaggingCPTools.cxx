/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#include "TopCPTools/TopFlavorTaggingCPTools.h"

#include <map>
#include <string>
#include <algorithm>
#include <iterator>

// Top includes
#include "TopConfiguration/TopConfig.h"
#include "TopConfiguration/Tokenize.h"
#include "TopEvent/EventTools.h"

// PathResolver include(s):
#include "PathResolver/PathResolver.h"

#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"

namespace top {
  FlavorTaggingCPTools::FlavorTaggingCPTools(const std::string& name) :
    asg::AsgTool(name) {
    declareProperty("config", m_config);
  }

  StatusCode FlavorTaggingCPTools::initialize() {
    ATH_MSG_INFO("top::FlavorTaggingCPTools initialize...");

    if (m_config->isTruthDxAOD()) {
      ATH_MSG_INFO("top::FlavorTaggingCPTools: no need to initialise anything on truth DxAOD");
      return StatusCode::SUCCESS;
    }

    if (!m_config->useJets()) {
      ATH_MSG_INFO("top::FlavorTaggingCPTools: no need to initialise anything since not using jets");
      return StatusCode::SUCCESS;
    }

    static const std::string cdi_file_default =
      "xAODBTaggingEfficiency/13TeV/2023-21-13TeV-MC16-CDI-2023-07-18_v1.root";


    m_tagger = ""; // Extract in the loop
    if (m_config->bTaggingCDIPath() != "Default") {
      if (m_config->bTaggingCDIPath() != cdi_file_default) {
        m_config->setPrintCDIpathWarning(true);
      }
      m_cdi_file = m_config->bTaggingCDIPath();
    } else {
      m_cdi_file = cdi_file_default;
    }
    // This ordering needs to match the indexing in TDP (for missing cases, we use default which gives a MC/MC of 1 as
    // its the same as the eff used in the calibration
    // Pythia6;Herwigpp;Pythia8;Sherpa(2.2);Sherpa(2.1);aMC@NLO+Pythia8;Herwig7.1.3;Sherpa228;Sherpa2210;Herwigpp721
    // Default changed from 410501 to 410470 in the CDI release of October 2018
    m_efficiency_maps = "default;410558;410470;410250;default;410464;411233;421152;700122;600666";

    // Configure all tagger/WP/calibration with helper function touching member variables
    // Calibrated and uncalibrated working points for EMTopo jets for all algorithms
    top::check(setTaggerWorkingPoints("AntiKt4EMTopoJets", true, "MV2c10",
                                      {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85",
                                       "Continuous"}),
                                       "Error setting AntiKt4EMTopoJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMTopoJets", true, "DL1",
                                      {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85",
                                       "Continuous"}), "Error setting AntiKt4EMTopoJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMTopoJets", false, "DL1r", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt4EMTopoJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMTopoJets", false, "DL1rmu", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "CTag_Loose", "CTag_Tight", "Continuous"}), "Error setting AntiKt4EMTopoJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMTopoJets", false, "DL1", {"CTag_Loose", "CTag_Tight"}), "Error setting AntiKt4EMTopoJets WP");

    // Calibrated and uncalibrated working points for EMPflow jets for all algorithms
    top::check(setTaggerWorkingPoints("AntiKt4EMPFlowJets", false, "MV2c10", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt4EMPFlowJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMPFlowJets", true, "DL1", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt4EMPFlowJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMPFlowJets", true, "DL1r", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt4EMPFlowJets WP");
    top::check(setTaggerWorkingPoints("AntiKt4EMPFlowJets", false, "DL1rmu", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt4EMPFlowJets WP");

    // Calibrated and uncalibrated working points for R=0.2 track jets for all algorithms
    top::check(setTaggerWorkingPoints("AntiKt2PV0TrackJets", true, "MV2c10", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt2PV0TrackJets WP");
    top::check(setTaggerWorkingPoints("AntiKt2PV0TrackJets", true, "DL1", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt2PV0TrackJets WP");
    top::check(setTaggerWorkingPoints("AntiKt2PV0TrackJets", false, "DL1r", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt2PV0TrackJets WP");
    top::check(setTaggerWorkingPoints("AntiKt2PV0TrackJets", false, "DL1rmu", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKt2PV0TrackJets WP");

    // Calibrated and uncalibrated working points for VR track jets for all algorithms
    top::check(setTaggerWorkingPoints("AntiKtVR30Rmax4Rmin02TrackJets", true, "MV2c10", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKtVR30Rmax4Rmin02TrackJets WP");
    top::check(setTaggerWorkingPoints("AntiKtVR30Rmax4Rmin02TrackJets", true, "DL1", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKtVR30Rmax4Rmin02TrackJets WP");
    top::check(setTaggerWorkingPoints("AntiKtVR30Rmax4Rmin02TrackJets", true, "DL1r", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKtVR30Rmax4Rmin02TrackJets WP");
    top::check(setTaggerWorkingPoints("AntiKtVR30Rmax4Rmin02TrackJets", false, "DL1rmu", {"FixedCutBEff_60", "FixedCutBEff_70", "FixedCutBEff_77", "FixedCutBEff_85", "Continuous"}), "Error setting AntiKtVR30Rmax4Rmin02TrackJets WP");


    std::string caloJets_type = m_config->sgKeyJetsType();
    std::string caloJets_collection = m_config->sgKeyJets();
    if (m_config->useHIJets()) {
      if (m_config->isMC()) {
        caloJets_collection = "BTagging_AntiKt4HI";
      } else {
        caloJets_collection = "BTagging_DFAntiKt4HI";
      }
    }

    std::string trackJets_type = m_config->sgKeyTrackJetsType();
    std::string trackJets_collection = m_config->sgKeyTrackJets();

    const std::string calib_file_path = PathResolverFindCalibFile(m_cdi_file);
    const std::string excludedSysts = m_config->bTagSystsExcludedFromEV() == "none" ? "" : m_config->bTagSystsExcludedFromEV();

    //------------------------------------------------------------
    // Loop through all the different working points we have and create a
    // BTaggingSelectionTool and corresponding BTaggingEfficiencyTool if the working point is calibrated.
    //------------------------------------------------------------

    // check if the WP requested by the user are available, and if yes, initialize the tools
    // loop through all btagging WPs requested
    for (auto TaggerBtagWP : m_config->bTagWP()) {
      // Overwrite m_tagger anyway (default has to be mv2c10 for R20.7
      m_tagger = TaggerBtagWP.first;
      std::string btagWP = TaggerBtagWP.second;
      std::string bTagWPName = m_tagger + "_" + btagWP;
      if ((caloJets_type == "AntiKt4EMTopoJets" && std::find(m_calo_WPs.begin(), m_calo_WPs.end(), bTagWPName) == m_calo_WPs.end()) ||
          (caloJets_type == "AntiKt4EMPFlowJets" && std::find(m_pflow_WPs.begin(), m_pflow_WPs.end(), bTagWPName) == m_pflow_WPs.end())) {
        ATH_MSG_WARNING("top::FlavorTaggingCPTools::initialize");
        ATH_MSG_WARNING("     b-tagging WP: " + bTagWPName + " not supported for jet collection " + caloJets_collection + " with algorithm " + m_tagger);
        ATH_MSG_WARNING("     it will therefore be ignored");
      } else {
        //------------------------------------------------------------
        // Setup BTaggingSelectionTool
        //------------------------------------------------------------
        // Updated name to use m_tagger
        std::string btagsel_tool_name = "BTaggingSelectionTool_" + bTagWPName + "_" + caloJets_collection;
        BTaggingSelectionTool* btagsel = new BTaggingSelectionTool(btagsel_tool_name);
        top::check(btagsel->setProperty("TaggerName", m_tagger),
                   "Failed to set b-tagging selecton tool TaggerName");
        top::check(btagsel->setProperty("JetAuthor", caloJets_collection),
                   "Failed to set b-tagging selection JetAuthor");
        top::check(btagsel->setProperty("FlvTagCutDefinitionsFileName",
                                        m_cdi_file),
                   "Failed to set b-tagging selection tool CDI file");
        top::check(btagsel->setProperty("OperatingPoint", btagWP),
                   "Failed to set b-tagging selection tool OperatingPoint");
        top::check(btagsel->setProperty("MinPt",
                                        static_cast<double>(m_config->jetPtcut())),
                   "Failed to set b-tagging selection tool MinPt");
        top::check(btagsel->setProperty("MaxEta",
                                        static_cast<double>(m_config->jetEtacut())),
                   "Failed to set b-tagging selection tool MaxEta");
        top::check(btagsel->initialize(),
                   "Failed to initialize b-tagging selection tool");
        m_btagging_selection_tools.push_back(btagsel);
        m_config->setBTagAlgo_available(m_tagger, btagsel_tool_name);

        if ((caloJets_type == "AntiKt4EMTopoJets" && std::find(m_calo_WPs_calib.begin(), m_calo_WPs_calib.end(), bTagWPName) == m_calo_WPs_calib.end()) ||
            (caloJets_type == "AntiKt4EMPFlowJets" && std::find(m_pflow_WPs_calib.begin(), m_pflow_WPs_calib.end(), bTagWPName) == m_pflow_WPs_calib.end())) {
          ATH_MSG_WARNING("top::FlavorTaggingCPTools::initialize");
          ATH_MSG_WARNING("     b-tagging WP: " + bTagWPName + " is not calibrated for jet collection " + caloJets_collection);
          ATH_MSG_WARNING("     it will therefore be ignored for the scale-factors, although the tagging decisions will be saved");
        } else {
          //------------------------------------------------------------
          // Setup BTaggingEfficiencyTool
          //------------------------------------------------------------
          std::string btageff_tool_name = "BTaggingEfficiencyTool_" + bTagWPName + "_" + caloJets_collection;
          BTaggingEfficiencyTool* btageff = new BTaggingEfficiencyTool(btageff_tool_name);
          top::check(btageff->setProperty("TaggerName", m_tagger),
                     "Failed to set b-tagging TaggerName");
          top::check(btageff->setProperty("OperatingPoint", btagWP),
                     "Failed to set b-tagging OperatingPoint");
          top::check(btageff->setProperty("JetAuthor", caloJets_collection),
                     "Failed to set b-tagging JetAuthor");
          top::check(btageff->setProperty("MinPt",
                                          static_cast<double>(m_config->jetPtcut())),
                                          "Failed to set b-tagging selection tool MinPt");
          top::check(btageff->setProperty("EfficiencyFileName", calib_file_path),
                     "Failed to set path to b-tagging CDI file");
          top::check(btageff->setProperty("ScaleFactorFileName", calib_file_path),
                     "Failed to set path to b-tagging CDI file");
          top::check(btageff->setProperty("ScaleFactorBCalibration", m_config->bTaggingCalibration_B()),
                     "Failed to set b-tagging calibration (B): " + m_config->bTaggingCalibration_B());
          top::check(btageff->setProperty("ScaleFactorCCalibration", m_config->bTaggingCalibration_C()),
                     "Failed to set b-tagging calibration (C): " + m_config->bTaggingCalibration_C());
          // using same calibration for T as for C
          top::check(btageff->setProperty("ScaleFactorTCalibration", m_config->bTaggingCalibration_C()),
                     "Failed to set b-tagging calibration (T): " + m_config->bTaggingCalibration_C());
          top::check(btageff->setProperty("ScaleFactorLightCalibration", m_config->bTaggingCalibration_Light()),
                     "Failed to set b-tagging calibration (Light): " + m_config->bTaggingCalibration_Light());
          for (auto jet_flav : m_jet_flavors) {
            // 09/02/18 IC: The pseudo-continuous does not have MC/MC SF so we need to only apply default for this case
            // 08/05/18 Francesco La Ruffa: The pseudo-continuous has now its own MC/MC SFs, no needed to set default
            top::check(btageff->setProperty("Efficiency" + jet_flav + "Calibrations", m_efficiency_maps),
                       "Failed to set " + jet_flav + "-calibrations efficiency maps");
          }
          top::check(btageff->setProperty("ExcludeFromEigenVectorTreatment", excludedSysts),
                     "Failed to set b-tagging systematics to exclude from EV treatment");
          top::check(btageff->initialize(), "Failed to initialize " + bTagWPName);
          // Check the excludedSysts - Cannot check before the tool is initialised
          if (this->checkExcludedSysts(btageff, excludedSysts) != StatusCode::SUCCESS) {
              ATH_MSG_WARNING("Incorrect excluded systematics have been provided.");
          }
          m_btagging_efficiency_tools.push_back(btageff);
          m_config->setBTagWP_calibrated(bTagWPName);
        }
        m_config->setBTagWP_available(bTagWPName);
      }
    }
    if (m_config->useTrackJets()) {
      for (auto TaggerBtagWP : m_config->bTagWP_trkJet()) {
        m_tagger = TaggerBtagWP.first;
        std::string btagWP = TaggerBtagWP.second;
        std::string bTagWPName = m_tagger + "_" + btagWP;
        std::vector<std::string> track_WPs = {};
        std::vector<std::string> track_WPs_calib = {};
        if (trackJets_type == "AntiKtVR30Rmax4Rmin02TrackJets") {
          track_WPs = m_trackAntiKtVR_WPs;
          track_WPs_calib = m_trackAntiKtVR_WPs_calib;
        } else if (trackJets_type == "AntiKt2PV0TrackJets") {
          track_WPs = m_trackAntiKt2_WPs;
          track_WPs_calib = m_trackAntiKt2_WPs_calib;
        } else if (trackJets_type == "AntiKt4PV0TrackJets") {
          track_WPs = m_trackAntiKt4_WPs;
          track_WPs_calib = m_trackAntiKt4_WPs_calib;
        }

        if (std::find(track_WPs.begin(), track_WPs.end(), bTagWPName) == track_WPs.end()) {
          ATH_MSG_WARNING("top::FlavorTaggingCPTools::initialize");
          ATH_MSG_WARNING("     b-tagging WP: " + bTagWPName + " not supported for jet collection " + trackJets_collection);
          ATH_MSG_WARNING("     it will therefore be ignored");
        } else {
          //------------------------------------------------------------
          // Setup BTaggingSelectionTool
          //------------------------------------------------------------
          std::string btagsel_tool_name = "BTaggingSelectionTool_" + bTagWPName + "_" + trackJets_collection;
          BTaggingSelectionTool* btagsel = new BTaggingSelectionTool(btagsel_tool_name);
          top::check(btagsel->setProperty("TaggerName", m_tagger),
                     "Failed to set b-tagging selecton tool TaggerName");
          top::check(btagsel->setProperty("JetAuthor", trackJets_collection),
                     "Failed to set b-tagging selection JetAuthor");
          top::check(btagsel->setProperty("FlvTagCutDefinitionsFileName",
                                          m_cdi_file),
                     "Failed to set b-tagging selection tool CDI file");
          top::check(btagsel->setProperty("OperatingPoint", btagWP),
                     "Failed to set b-tagging selection tool OperatingPoint");
          top::check(btagsel->setProperty("MinPt",
                                          static_cast<double>(m_config->trackJetPtcut())),
                     "Failed to set b-tagging selection tool MinPt");
          top::check(btagsel->setProperty("MaxEta",
                                          static_cast<double>(m_config->trackJetEtacut())),
                     "Failed to set b-tagging selection tool MaxEta");
          top::check(btagsel->initialize(),
                     "Failed to initialize b-tagging selection tool");
          m_btagging_selection_tools.push_back(btagsel);
          m_config->setBTagAlgo_available_trkJet(m_tagger, btagsel_tool_name);

          if (std::find(track_WPs_calib.begin(),
                        track_WPs_calib.end(), bTagWPName) == track_WPs_calib.end()) {
            ATH_MSG_WARNING("top::FlavorTaggingCPTools::initialize");
            ATH_MSG_WARNING("     b-tagging WP: " + bTagWPName + " is not calibrated for jet collection " + trackJets_collection);
            ATH_MSG_WARNING("     it will therefore be ignored for the scale-factors, although the tagging decisions will be saved");
          } else {
            //------------------------------------------------------------
            // Setup BTaggingEfficiencyTool
            //------------------------------------------------------------
            std::string btageff_tool_name = "BTaggingEfficiencyTool_" + bTagWPName + "_" + trackJets_collection;
            BTaggingEfficiencyTool* btageff = new BTaggingEfficiencyTool(btageff_tool_name);
            top::check(btageff->setProperty("TaggerName", m_tagger),
                       "Failed to set b-tagging TaggerName");
            top::check(btageff->setProperty("OperatingPoint", btagWP),
                       "Failed to set b-tagging OperatingPoint");
            top::check(btageff->setProperty("JetAuthor", trackJets_collection),
                       "Failed to set b-tagging JetAuthor");
            top::check(btageff->setProperty("MinPt",
                                        static_cast<double>(m_config->trackJetPtcut())),
		       "Failed to set b-tagging selection tool MinPt");
            top::check(btageff->setProperty("EfficiencyFileName", calib_file_path),
                       "Failed to set path to b-tagging CDI file");
            top::check(btageff->setProperty("ScaleFactorFileName", calib_file_path),
                       "Failed to set path to b-tagging CDI file");
            top::check(btageff->setProperty("ScaleFactorBCalibration", m_config->bTaggingCalibration_B()),
                       "Failed to set b-tagging calibration (B): " + m_config->bTaggingCalibration_B());
            top::check(btageff->setProperty("ScaleFactorCCalibration", m_config->bTaggingCalibration_C()),
                       "Failed to set b-tagging calibration (C): " + m_config->bTaggingCalibration_C());
            // using same calibration for T as for C
            top::check(btageff->setProperty("ScaleFactorTCalibration", m_config->bTaggingCalibration_C()),
                       "Failed to set b-tagging calibration (T): " + m_config->bTaggingCalibration_C());
            top::check(btageff->setProperty("ScaleFactorLightCalibration", m_config->bTaggingCalibration_Light()),
                       "Failed to set b-tagging calibration (Light): " + m_config->bTaggingCalibration_Light());
            for (auto jet_flav : m_jet_flavors) {
              top::check(btageff->setProperty("Efficiency" + jet_flav + "Calibrations", m_efficiency_maps),
                         "Failed to set " + jet_flav + "-calibrations efficiency maps");
            }

            top::check(btageff->setProperty("ExcludeFromEigenVectorTreatment", excludedSysts),
                       "Failed to set b-tagging systematics to exclude from EV treatment");
            top::check(btageff->initialize(), "Failed to initialize " + bTagWPName);
            // Check the excludedSysts - Cannot check before the tool is initialised
            if (this->checkExcludedSysts(btageff, excludedSysts) != StatusCode::SUCCESS) {
                ATH_MSG_WARNING("Incorrect excluded systematics have been provided.");
            }
            m_btagging_efficiency_tools.push_back(btageff);
            m_config->setBTagWP_calibrated_trkJet(bTagWPName);
          }
          m_config->setBTagWP_available_trkJet(bTagWPName);
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode FlavorTaggingCPTools::checkExcludedSysts(BTaggingEfficiencyTool* btageff, std::string excludedSysts) {
    // We pass the pointer to the btagging efficiency tool which is being created and also the excludedSysts string
    // which will be used
    // If the string is empty, then nothing to check
    if (excludedSysts == "") return StatusCode::SUCCESS;

    // Split by a semi-colon delimiter and then check the individual syst strings against the list from the CDI
    std::vector<std::string> listOfExcludedSysts;
    top::tokenize(excludedSysts, listOfExcludedSysts, ";");
    ATH_MSG_INFO(" ------------------------------------------------ ");
    ATH_MSG_INFO(" ------------- EXPERIMENTAL FEATURE ------------- ");
    ATH_MSG_INFO(" ------ Please provide feedback to TopReco ------ ");
    ATH_MSG_INFO(" ------------- EXPERIMENTAL FEATURE ------------- ");
    ATH_MSG_INFO(" ------------------------------------------------ ");
    ATH_MSG_INFO(" AnalysisTop - Checking excludedSysts for flavour tagging EV");
    ATH_MSG_INFO(" This has been split on the semi-colon delimiter to find...");
    for (auto s : listOfExcludedSysts) ATH_MSG_INFO("... " + s);
    // Get the map(string, vector<string>) from the CDI tool
    // Don't care about the flavours (this will be handled in the CDI)
    std::vector<std::string> listOfScaleFactorSystematics;
    for (auto flavour : btageff->listScaleFactorSystematics(false)) {
      for (auto sys : flavour.second) {
        listOfScaleFactorSystematics.push_back(sys);
      }
    }
    // Make this a unique set and then we need to check that all systematics provided by the user are expected by the
    // CDI
    std::set<std::string> setOfExcludedSysts, setOfScaleFactorSystematics;

    for (auto sys : listOfExcludedSysts) {
      if (setOfExcludedSysts.find(sys) == setOfExcludedSysts.end()) setOfExcludedSysts.insert(sys);
    }

    for (auto sys : listOfScaleFactorSystematics) {
      if (setOfScaleFactorSystematics.find(sys) == setOfScaleFactorSystematics.end()) setOfScaleFactorSystematics.insert(sys);
    }

    //
    std::vector<std::string> unionOfSystematics;
    std::set_intersection(setOfExcludedSysts.begin(), setOfExcludedSysts.end(),
                          setOfScaleFactorSystematics.begin(), setOfScaleFactorSystematics.end(),
                          std::back_inserter(unionOfSystematics));
    // Check we have the same systematics listed in unionOfSystematics
    if (unionOfSystematics.size() != listOfExcludedSysts.size()) {
      ATH_MSG_WARNING("Have not found all systematics listed to be excluded from b-tagging eigenvector method");
      ATH_MSG_INFO("Permitted values are...");
      for (auto sys : setOfScaleFactorSystematics) {
        ATH_MSG_INFO(" ... " + sys);
      }
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO(" Summary of EV impact ");
      for (auto sysRemove : listOfExcludedSysts) {
        std::string flavourAffected = "";
        for (auto flavour : btageff->listScaleFactorSystematics(false)) {
          for (auto sysCDI : flavour.second) {
            if (sysRemove == sysCDI) flavourAffected += flavour.first;
          }
        }
        ATH_MSG_INFO(" ... " + sysRemove + " -> Removed from calibration(s) : [" + flavourAffected + "]");
      }
      ATH_MSG_INFO(" These will be dynamically matched to systematic tree names (if available)");
      ATH_MSG_INFO(" All systematics are accepted by CDI file ");
    }
    // We have passed all the tests so now we store the systematics removed from the EV method and use a mapping to
    // ASG/AT naming and return
    ATH_MSG_INFO(" ------------------------------------------------ ");
    return StatusCode::SUCCESS;
  }

  StatusCode FlavorTaggingCPTools::setTaggerWorkingPoints(std::string jetcollection, bool isCalibrated, std::string tagger, std::vector<std::string> list_of_WP) {
    // To try to reduce errors, make a helper function for setting the lists of tagger_WP which are required
    if (jetcollection == "AntiKt4EMTopoJets" && isCalibrated) {
      // use m_calo_WPs_calib
      for (auto s : list_of_WP) {
        m_calo_WPs_calib.push_back(tagger + "_" + s);
        m_calo_WPs.push_back(tagger + "_" + s);
      }
    } else if (jetcollection == "AntiKt4EMTopoJets" && !isCalibrated) {
      // use m_calo_WPs
      for (auto s : list_of_WP) m_calo_WPs.push_back(tagger + "_" + s);
    } else if (jetcollection == "AntiKt4EMPFlowJets" && isCalibrated) {
      // use m_pflow_WPs_calib
      for (auto s : list_of_WP) {
        m_pflow_WPs_calib.push_back(tagger + "_" + s);
        m_pflow_WPs.push_back(tagger + "_" + s);
      }
    } else if (jetcollection == "AntiKt4EMPFlowJets" && !isCalibrated) {
      // use m_pflow_WPs
      for (auto s : list_of_WP) m_pflow_WPs.push_back(tagger + "_" + s);
    } else if (jetcollection == "AntiKtVR30Rmax4Rmin02TrackJets" && isCalibrated) {
      // use m_trackAntiKt2_WPs_calib
      for (auto s : list_of_WP) {
        m_trackAntiKtVR_WPs_calib.push_back(tagger + "_" + s);
        m_trackAntiKtVR_WPs.push_back(tagger + "_" + s);
      }
    } else if (jetcollection == "AntiKtVR30Rmax4Rmin02TrackJets" && !isCalibrated) {
      // use m_trackAntiKt2_WPs
      for (auto s : list_of_WP) m_trackAntiKtVR_WPs.push_back(tagger + "_" + s);
    } else if (jetcollection == "AntiKt2PV0TrackJets" && isCalibrated) {
      // use m_trackAntiKt2_WPs_calib
      for (auto s : list_of_WP) {
        m_trackAntiKt2_WPs_calib.push_back(tagger + "_" + s);
        m_trackAntiKt2_WPs.push_back(tagger + "_" + s);
      }
    } else if (jetcollection == "AntiKt2PV0TrackJets" && !isCalibrated) {
      // use m_trackAntiKt2_WPs
      for (auto s : list_of_WP) m_trackAntiKt2_WPs.push_back(tagger + "_" + s);
    } else if (jetcollection == "AntiKt4PV0TrackJets" && isCalibrated) {
      // use m_trackAntiKt4_WPs_calib
      for (auto s : list_of_WP) {
        m_trackAntiKt4_WPs_calib.push_back(tagger + "_" + s);
        m_trackAntiKt4_WPs.push_back(tagger + "_" + s);
      }
    } else if (jetcollection == "AntiKt4PV0TrackJets" && !isCalibrated) {
      // use m_trackAntiKt2_WPs_calib
      for (auto s : list_of_WP) m_trackAntiKt4_WPs.push_back(tagger + "_" + s);
    } else {
      ATH_MSG_ERROR("Unknown jet collection and calibration options");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  void FlavorTaggingCPTools::printConfigurations() {
    // Debugging function, not used in release
    ATH_MSG_INFO("AntiKt4EMTopoJets - Calibrated WP");
    for (auto s : m_calo_WPs_calib) ATH_MSG_INFO(" -> " << s);
    ATH_MSG_INFO("AntiKt4EMTopoJets - Available selection WP");
    for (auto s : m_calo_WPs) ATH_MSG_INFO(" -> " << s);

    return;
  }
}  // namespace top
