/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TOPCPTOOLS_TOPJETMETCPTOOLS_H_
#define TOPCPTOOLS_TOPJETMETCPTOOLS_H_

// Include what you use
#include <vector>
#include <string>
#include <cstdlib>

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/ToolHandleArray.h"
#include "AsgTools/AnaToolHandle.h"

// Jet include(s):
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCPInterfaces/ICPJetUncertaintiesTool.h"
#include "JetCPInterfaces/ICPJetCorrectionTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "JetInterface/IJetSelector.h"
#include "JetInterface/IJetModifier.h"
#include "JetAnalysisInterfaces/IJetJvtEfficiency.h"
#include "JetSelectorTools/IEventCleaningTool.h"

// MET include(s):
#include "METInterface/IMETMaker.h"
#include "METInterface/IMETSystematicsTool.h"
#include "METInterface/IMETSignificance.h"


namespace top {
  class TopConfig;

  class JetMETCPTools final: public asg::AsgTool {
  public:
    explicit JetMETCPTools(const std::string& name);
    virtual ~JetMETCPTools() {}

    StatusCode initialize();
  private:
    std::shared_ptr<top::TopConfig> m_config;

    std::string m_jetJVT_ConfigFile;
    std::string m_truthJetCollForHS;

    std::string m_jetAntiKt4_Data_ConfigFile;
    std::string m_jetAntiKt4_Data_CalibSequence;

    std::string m_jetAntiKt4_MCFS_ConfigFile;
    std::string m_jetAntiKt4_MCFS_CalibSequence;
    
    std::string m_jetAntiKt4HI_MC_ConfigFile;
    std::string m_jetAntiKt4HI_MC_CalibSequence;

    std::string m_jetAntiKt4HI_Data_ConfigFile;
    std::string m_jetAntiKt4HI_Data_CalibSequence;

    std::string m_jetAntiKt4HI_pPb_ConfigFile;
    std::string m_jetAntiKt4HI_Pbp_ConfigFile;

    // small-R calibration with jet mass calibration included
    std::string m_jetAntiKt4_Data_JMS_ConfigFile;
    std::string m_jetAntiKt4_Data_JMS_CalibSequence;

    // only for fullsim, AFII not supported
    std::string m_jetAntiKt4_MCFS_JMS_ConfigFile;
    std::string m_jetAntiKt4_MCFS_JMS_CalibSequence;

    std::string m_jetAntiKt4_MCAFII_ConfigFile;
    std::string m_jetAntiKt4_MCAFII_CalibSequence;

    std::string m_jetAntiKt4_MCAFII_PFlow_ConfigFile;
    std::string m_jetAntiKt4_MCAFII_PFlow_CalibSequence;

    std::string m_jetAntiKt4_Data_PFlow_ConfigFile;
    std::string m_jetAntiKt4_Data_PFlow_CalibSequence;

    std::string m_jetAntiKt4_PFlow_MCFS_ConfigFile;
    std::string m_jetAntiKt4_PFlow_MCFS_CalibSequence;

    // small-R calibration with jet mass calibration included
    std::string m_jetAntiKt4_Data_PFlow_JMS_ConfigFile;
    std::string m_jetAntiKt4_Data_PFlow_JMS_CalibSequence;

    // only for fullsim, AFII not supported
    std::string m_jetAntiKt4_PFlow_MCFS_JMS_ConfigFile;
    std::string m_jetAntiKt4_PFlow_MCFS_JMS_CalibSequence;

    ToolHandle<IJetCalibrationTool> m_jetCalibrationTool;
    ToolHandle<IJetCalibrationTool> m_jetCalibrationToolLargeR;

    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolLargeR;
    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolLargeRPseudoData;

    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesTool;
    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolPseudoData;
    ToolHandle<ICPJetCorrectionTool> m_FFJetSmearingTool;
    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolReducedNPScenario1;
    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolReducedNPScenario2;
    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolReducedNPScenario3;
    ToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesToolReducedNPScenario4;

    ToolHandle<IJetSelector> m_jetCleaningToolLooseBad;
    ToolHandle<IJetSelector> m_jetCleaningToolTightBad;

    // Implement event object cleaning tool
    ToolHandle<ECUtils::IEventCleaningTool> m_jetEventCleaningToolLooseBad;
    ToolHandle<ECUtils::IEventCleaningTool> m_jetEventCleaningToolTightBad;

    ToolHandle<IJetUpdateJvt> m_jetUpdateJvtTool;
    ToolHandle<IJetModifier> m_jetSelectfJvtTool;

    ToolHandle<CP::IJetJvtEfficiency> m_jetJvtTool;
    ToolHandle<CP::IJetJvtEfficiency> m_jetfJvtTool;

    ToolHandle<IMETMaker> m_met_maker;
    ToolHandle<IMETSystematicsTool> m_met_systematics;
    ToolHandle<IMETSignificance> m_metSignif;

    StatusCode setupJetsCalibration();
    StatusCode setupLargeRJetsCalibration();
    StatusCode setupJetsScaleFactors();
    StatusCode setupMET();


    ICPJetCorrectionTool * setupFFJetSmearingTool(const std::string& mass_def,const std::string& config);


    ICPJetUncertaintiesTool*
    setupJetUncertaintiesTool(const std::string &name,
                              const std::string &jet_def,
                              const std::string &mc_type,
                              bool isMC,
                              const std::string &config_file,
                              std::vector<std::string> *variables,
                              const std::string &analysis_file = "",
                              const std::string &calib_area = "None",
                              bool use_abs_gfrac_eta = true);

    IJetSelector* setupJetCleaningTool(const std::string& WP);
    ECUtils::IEventCleaningTool* setupJetEventCleaningTool(const std::string& WP,
                                                           ToolHandle<IJetSelector> JetCleaningToolHandle);
  };
}  // namespace top

#endif  // TOPCPTOOLS_TOPJETMETCPTOOLS_H_
