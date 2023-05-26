/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

#include "TopCPTools/TopIsolationCPTools.h"

#include <map>
#include <string>

// Top includes
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

// PathResolver include(s):
#include "PathResolver/PathResolver.h"

// Isolation include(s):
#include "IsolationSelection/IsolationSelectionTool.h"
#include "IsolationSelection/IsolationLowPtPLVTool.h"
#include "IsolationCorrections/IsolationCorrectionTool.h"

namespace top {
  IsolationCPTools::IsolationCPTools(const std::string& name) :
    asg::AsgTool(name),
    m_isolationCalibFile("IsolationSelection/v1/MC15_Z_Jpsi_cutMap.root") {
    declareProperty("config", m_config);

    declareProperty("IsolationCorrectionTool", m_isolationCorr);
  }

  StatusCode IsolationCPTools::initialize() {
    ATH_MSG_INFO("top::IsolationCPTools initialize...");

    if (!m_config->makeAllCPTools()) {
      ATH_MSG_INFO("top::IsolationCPTools: no need to initialise for mini-xAOD");
      return StatusCode::SUCCESS;
    }

    if (!m_config->useElectrons() && !m_config->useMuons()) {
      ATH_MSG_INFO("top::IsolationCPTools: no need to initialise since neither using electrons nor muons");
      return StatusCode::SUCCESS;
    }
    top::check(setupIsolation(), "Failed to setup Isolation tools");
    return StatusCode::SUCCESS;
  }

  StatusCode IsolationCPTools::setupIsolation() {
    // needed both for electrons and photons to apply the leakage correction,
    // [http://cern.ch/go/j6Lx]
    std::string iso_corr_tool_name = "CP::IsolationCorrectionTool";
    if (asg::ToolStore::contains<CP::IIsolationCorrectionTool>(iso_corr_tool_name)) {
      m_isolationCorr = asg::ToolStore::get<CP::IIsolationCorrectionTool>(iso_corr_tool_name);
    } else {
      CP::IIsolationCorrectionTool* isolationCorr = new CP::IsolationCorrectionTool(iso_corr_tool_name);
      top::check(asg::setProperty(isolationCorr, "IsMC", m_config->isMC()),
                 "Failed to setProperty IsMC");  // if MC, else false
      top::check(asg::setProperty(isolationCorr, "AFII_corr", m_config->isAFII()),
                 "Failed to setProperty AFII_corr"); // if AFII, else false
      if (m_config->useEgammaLeakageCorrection()) {
        top::check(asg::setProperty(isolationCorr, "Apply_SC_leakcorr", true),
                   "Failed to setProperty Apply_SC_leakcorr"); // super cluster based core correction
      }
      if (m_config->useEgammaPileupCorrection()) {
        top::check(asg::setProperty(isolationCorr, "Apply_etaEDParPU_correction", true),
                   "Failed to setProperty Apply_etaEDParPU_correction"); // improved energy density based pileup
                                                                         // correction with eta variations
        top::check(asg::setProperty(isolationCorr, "Apply_etaEDPar_mc_correction", m_config->isMC()),
                   "Failed to setProperty Apply_etaEDPar_mc_correction"); // pileup dependent correction to MC
      }
      top::check(isolationCorr->initialize(), "Failed to initialize");

      m_isolationCorr = isolationCorr;
    }

    // Muon Isolation WPs
    std::set<std::string> muon_isolations {{
                                             "FCTight",
                                             "FCLoose",
                                             "FCTightTrackOnly",
                                             "FCTightTrackOnly_FixedRad",
                                             "FCLoose_FixedRad",
                                             "FCTight_FixedRad",
                                             "FixedCutPflowTight",
                                             "FixedCutPflowLoose",
                                             "PflowTight_FixedRad",
                                             "PflowLoose_FixedRad",
                                             "PflowTight_VarRad",
                                             "PflowLoose_VarRad",
                                             "HighPtTrackOnly",
                                             "TightTrackOnly_VarRad",
                                             "TightTrackOnly_FixedRad",
                                             "PLVTight",
                                             "PLVLoose",
                                             "Tight_VarRad",
                                             "Tight_FixedRad",
                                             "Loose_VarRad",
                                             "Loose_FixedRad",
                                             "PLImprovedTight",
                                             "PLImprovedVeryTight",
                                           }};

    // Electron Isolation WPs
    std::set<std::string> electron_isolations {{
                                                 "Tight_VarRad",
                                                 "Loose_VarRad",
                                                 "TightTrackOnly_VarRad",
                                                 "Gradient",
                                                 "FCHighPtCaloOnly",
                                                 "FCTight",
                                                 "FCLoose",
                                                 "HighPtCaloOnly",
                                                 "Loose",
                                                 "Tight",
                                                 "TightTrackOnly",
                                                 "TightTrackOnly_FixedRad",
                                                 "PflowTight",
                                                 "PflowLoose",
                                                 "PLVTight",
                                                 "PLVLoose",
                                                 "PLImprovedTight",
                                                 "PLImprovedVeryTight",
                                               }};

    // Photon Isolation WPs
    std::set<std::string> photon_isolations {{
                                               "FixedCutTightCaloOnly",
                                               "FixedCutTight",
                                               "FixedCutLoose",
                                               "TightCaloOnly",
                                               "Tight",
                                               "Loose",
                                             }};

    std::set<std::string> all_isolations;
    all_isolations.insert(muon_isolations.begin(), muon_isolations.end());
    all_isolations.insert(electron_isolations.begin(), electron_isolations.end());
    all_isolations.insert(photon_isolations.begin(), photon_isolations.end());

    for (const std::string& isoWP : all_isolations) {
      std::string tool_name;
      if (isoWP.find("PL") != std::string::npos) {
        tool_name = "CP::IsolationTool_LowPtPLV";
        if (!asg::ToolStore::contains<CP::IIsolationLowPtPLVTool>(tool_name)) {
          CP::IIsolationLowPtPLVTool* iso_tool = new CP::IsolationLowPtPLVTool(tool_name);
          top::check(iso_tool->initialize(), "Failed to initialize " + tool_name);
          m_isolationToolsLowPtPLV.push_back(iso_tool);
        }
      }
      tool_name = "CP::IsolationTool_" + isoWP;
      if (!asg::ToolStore::contains<CP::IIsolationSelectionTool>(tool_name)) {
        CP::IIsolationSelectionTool* iso_tool = new CP::IsolationSelectionTool(tool_name);
        top::check(asg::setProperty(iso_tool, "CalibFileName", m_isolationCalibFile),
                   "Failed to set CalibFileName for " + tool_name);
        if (electron_isolations.find(isoWP) !=
            electron_isolations.end()) top::check(asg::setProperty(iso_tool, "ElectronWP",
                                                                   isoWP),
                                                  "Failed to set electron WP for " +
                                                  tool_name);
        if (muon_isolations.find(isoWP) != muon_isolations.end()) top::check(asg::setProperty(iso_tool, "MuonWP",
                                                                                              isoWP),
                                                                             "Failed to set muon WP for " + tool_name);
        if (photon_isolations.find(isoWP) !=
            photon_isolations.end()) top::check(asg::setProperty(iso_tool, "PhotonWP",
                                                                 isoWP),
                                                "Failed to set photon WP for " +
                                                tool_name);
        top::check(iso_tool->initialize(), "Failed to initialize " + tool_name);
        m_isolationTools.push_back(iso_tool);
      }
    }
    return StatusCode::SUCCESS;
  }
}  // namespace top
