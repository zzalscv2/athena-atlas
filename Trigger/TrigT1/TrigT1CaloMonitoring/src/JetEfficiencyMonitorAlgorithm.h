/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_JETEFFICIENCYMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_JETEFFICIENCYMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthenaKernel/Units.h"
#include "FourMomUtils/P4Helpers.h"


#include "xAODTrigger/gFexJetRoI.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoI.h" 
#include "xAODTrigger/gFexGlobalRoIContainer.h"

using Athena::Units::GeV;
 
// #include "TrigDecisionTool/TrigDecisionTool.h"
//#include "TrigT1Interfaces/TrigT1CaloDefs.h" 

class JetEfficiencyMonitorAlgorithm : public AthMonitorAlgorithm {
public:JetEfficiencyMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~JetEfficiencyMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
private:
  StringProperty m_packageName{this,"PackageName","JetEfficiencyMonitor","group name for histograming"};
  StringProperty m_bootstrap_trigger{this,"BootstrapTrigger","L1_J100","the bootstrapping trigger"};
  Gaudi::Property<bool> m_emulated{this,"Emulated",0,"boolean of if we want to emulate the gfex triggers"};

  Gaudi::Property<std::vector<std::string>> m_L1TriggerList{this,"L1TriggerList",{},"Vector of L1 offline jet triggers"};
  Gaudi::Property<std::vector<std::string>> m_SRgfexTriggerList{this,"SRgfexTriggerList",{},"Vector of SR gfex jet triggers"};
  Gaudi::Property<std::vector<std::string>> m_LRgfexTriggerList{this,"LRgfexTriggerList",{},"Vector of LR gfex jet triggers"};
  Gaudi::Property<std::vector<std::string>> m_TriggerList{this,"TriggerList",{},"Vector of triggers firing for jets over 100GeV"};

  
  // container keys including steering parameter and description
  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{ this, "JetKey" , "AntiKt4EMPFlowJets", ""}; //offline jets git 
  SG::ReadHandleKey<xAOD::JetContainer> m_LRjetKey{ this, "LRJetKey" , "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets", ""}; //LR offline jets 
  // SG::ReadHandleKey<xAOD::JetContainer> m_LRjetKey{ this, "LRJetKey" , "HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_nojcalib", ""}; //LR offline jets for Run3
  SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_gFexSRJetContainerKey{ this, "mygFexSRJetRoIContainer" , "L1_gFexSRJetRoI" , ""}; //gfex SR jets
  SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_gFexLRJetContainerKey{ this, "mygFexLRJetRoIContainer" , "L1_gFexLRJetRoI" , ""}; //gfex LR jets

};
#endif
