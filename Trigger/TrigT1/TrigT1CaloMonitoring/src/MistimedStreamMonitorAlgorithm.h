/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRIGT1CALOMONITORING_MISTIMEDSTREAMMONITORALGORITHM
#define TRIGT1CALOMONITORING_MISTIMEDSTREAMMONITORALGORITHM

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "TrigT1CaloToolInterfaces/IL1TriggerTowerToolRun3.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "TrigT1CaloCalibConditions/L1CaloRunParametersContainer.h"  
#include "TrigT1CaloCalibConditions/L1CaloReadoutConfigContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/CPMTowerContainer.h" 
#include "xAODTrigL1Calo/JetElementContainer.h"
// Trigger include(s):
#include "TrigDecisionTool/TrigDecisionTool.h"


class  MistimedStreamMonitorAlgorithm : public AthMonitorAlgorithm {
public: MistimedStreamMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~MistimedStreamMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
  
  /// Struct to contain PPM trigger tower info
  struct MonitorTT { 
    const xAOD::TriggerTower* tower;
    double phiScaled; /// phi for 2d maps with integer bins (taking into account granularity in eta)
    double phi1d;     /// phi for 1d phi distributions (taking into account granularity in eta) 
  };

  /// Struct to contain CPM tower info
  struct MonitorCPM { 
    const xAOD::CPMTower* tower = nullptr;
    std::vector<double>  phiScaled;
    double etaScaled = 0; /// phi for 2d maps with integer bins (taking into account granularity in eta)
    double phi1d = 0;     /// phi for 1d phi distributions (taking into account granularity in eta) 
  };

  /// Struct to contain JE info
  struct MonitorJE { 
    const xAOD::JetElement* element = nullptr;
    std::vector<double>  phiScaled;
    std::vector<double> etaScaled;
    double phi1d = 0;     /// phi for 1d phi distributions (taking into account granularity in eta) 
  };

  

private:
  
  StringProperty m_packageName{this,"PackageName","MistimedStreamMonitor","group name for histogramming"};

  bool pulseQuality(const std::vector<uint16_t> ttPulse, int peakSlice) const;
 

  // TrigDecisionTool
  PublicToolHandle< Trig::TrigDecisionTool > m_trigDec{this, "TriggerDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", ""};
  
  /// container keys including steering parameter and description
  SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_xAODTriggerTowerContainerName{this, "BS_xAODTriggerTowerContainer",LVL1::TrigT1CaloDefs::xAODTriggerTowerLocation,"Trigger Tower Container"};

  SG::ReadHandleKey<xAOD::CPMTowerContainer> m_cpmTowerLocation{this, "CPMTowerLocation", LVL1::TrigT1CaloDefs::CPMTowerLocation, "CPM container"};
  
  SG::ReadHandleKey<xAOD::JetElementContainer> m_jetElementLocation{this, "JetElementLocation", LVL1::TrigT1CaloDefs::JetElementLocation, "Jet Element Container"};



  ToolHandle<LVL1::IL1TriggerTowerToolRun3> m_ttTool{this,"L1TriggerTowerToolRun3", "LVL1::L1TriggerTowerToolRun3/L1TriggerTowerToolRun3", "L1TriggerTowerToolRun3"};

    /// Properties
  Gaudi::Property<double> m_phiScaleTT{this, "phiScaleTT", 32./M_PI, "Scale factor to convert trigger tower phi to integer binning"};


  
  
  // L1Calo Conditions 
  SG::ReadCondHandleKey<L1CaloRunParametersContainer>  m_runParametersContainer{ this, "InputKeyRunParameters", "L1CaloRunParametersContainer"};
  SG::ReadCondHandleKey<L1CaloReadoutConfigContainer>  m_readoutConfigContainer{ this, "InputKeyReadoutConfig", "L1CaloReadoutConfigContainer"};



  /// Helper functions
  StatusCode makeTowerPPM( const xAOD::TriggerTower* tt, 
                               std::vector<MonitorTT> &vecMonTT) const;

 
  StatusCode fillPPMEtaPhi( MonitorTT &monTT, 
                              const std::string& groupName, 
                              const std::string& weightName,
                              double weight) const;


  
  StatusCode makeTowerCPM( const xAOD::CPMTower* cpm, 
			   std::vector<MonitorCPM> &vecMonCPM) const;

  
 
  
  StatusCode makeTowerJE( const xAOD::JetElement* je, 
			   std::vector<MonitorJE> &vecMonJE) const;

 
  
  //Control maximum number of histograms per job
  Gaudi::Property<int> m_maxEvents{this,"MaxEvents",15};  
  
  // Event number counter
  mutable std::atomic<int>  m_eventCounter{0};
  // count number of error events per lumiblock across threads for each type of error
  mutable std::mutex m_mutex{};
  mutable std::map<uint32_t, int> m_event_counter ATLAS_THREAD_SAFE; 



  //  Bin Names for CutFlow
  enum cutFlow {
    All,
    UnsuitableReadout,
    HLT_mistimemonj400,
    L1_J100,
    badpeakTT,
    badCentralTT,
    badLateTT,
    lateTT,
    InTime,
    TTEMLayer
  };



   
};
#endif

