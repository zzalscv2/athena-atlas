/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ***************************************************************************
//
// NAME:    TagProbeEfficiencyMon.h
// PACKAGE: TrigT1CaloMonitoring
// AUTHOR:  Jason Barkeloo & Kate Whalen
//
// **************************************************************************

#ifndef TRIGT1CALOMONITORING_TAGPROBEEFFICIENCYMON_H
#define TRIGT1CALOMONITORING_TAGPROBEEFFICIENCYMON_H

#include <map>
#include <string>
#include <vector>

// Athena includes
#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"

// xAOD includes
#include "xAODTrigL1Calo/TriggerTower.h"

// Root Includes
#include <TLorentzVector.h>

class TH1F_LW;
class TH2F_LW;
class TH2I_LW;
class StatusCode;

namespace TrigConf { class ILVL1ConfigSvc; }

namespace LVL1 {

  // Forward declarations:
  class ITrigT1CaloMonErrorTool;
  class TrigT1CaloLWHistogramTool;

  class TagProbeEfficiencyMon: public ManagedMonitorToolBase { 
 
  public: 
  
  TagProbeEfficiencyMon(const std::string & type, const std::string& name, 
                                                  const IInterface* parent);
  virtual ~TagProbeEfficiencyMon(); 

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual StatusCode bookHistogramsRecurrent();
  virtual StatusCode fillHistograms();
  virtual StatusCode procHistograms();
 
  private: 
 
  //=============================================
  // Monitoring tools 
  // ============================================
 
  // Trigger configuration service
  ServiceHandle<TrigConf::ILVL1ConfigSvc> m_configSvc;
  
  /// Tool to retrieve bytestream errors
  ToolHandle<ITrigT1CaloMonErrorTool> m_errorTool;
  /// Histogram helper tool
  ToolHandle<TrigT1CaloLWHistogramTool> m_histTool;

  //============================================
  // StoreGate keys
  // ===========================================

  /// Trigger tower container StoreGate key
  std::string m_xAODTriggerTowerContainerName;
  /// CPM TOB RoI container StoreGate key
  std::string m_cpmTobRoiLocation;
  /// Error vector StoreGate key
  std::string m_errorLocation;

  //===========================================
  // Helper variables 
  // ==========================================
 
  /// Root directory
  std::string m_rootDir;
  /// Maximum energy plotted
  int m_maxEnergyRange{};
  /// Histograms booked flag
  bool m_histBooked{};
  float m_lowcutTag{};
  float m_lowcutProbe{};
  float m_inv_m_range{};
  float m_m_z{};
  int m_probe_et_bins{};
  int m_probe_eta_bins{};
  int m_probe_phi_bins{};
  double m_probe_et_min{};
  double m_probe_et_max{};
  double m_probe_eta_min{};
  double m_probe_eta_max{};
  double m_probe_phi_min{};
  double m_probe_phi_max{};
  int m_tag_charge{};
  std::vector<int> m_probelist{};
  int m_probe_check{};
  //TLorentzVector tagLV{}; 
  TLorentzVector m_probeLV;
  TLorentzVector m_combLV; 
  //TLorentzVector probeLV1{};
  TLorentzVector m_combLV1; 
  TLorentzVector m_tmpProbe; 

  //===========================================
  // Histograms
  // ==========================================
  
  // Histograms used to make efficiency plots
  static const int s_thresh_num=16; //16 EM Thresholds
  TH1F_LW* m_h_probe_eta_pass[s_thresh_num]{};
  TH1F_LW* m_h_probe_phi_pass[s_thresh_num]{};
  TH1F_LW* m_h_probe_Et_pass[s_thresh_num]{};
  TH1F_LW* m_h_lumib_int_Et_pass[s_thresh_num]{};
  TH1F_LW* m_h_lumib_int_phi_pass[s_thresh_num]{};
  TH1F_LW* m_h_lumib_int_eta_pass[s_thresh_num]{};
  TH1F_LW* m_h_lumib_int_Et_tot{};
  TH1F_LW* m_h_lumib_int_phi_tot{};
  TH1F_LW* m_h_lumib_int_eta_tot{};
  TH1F_LW* m_h_eta_eff[s_thresh_num]{};
  TH1F_LW* m_h_phi_eff[s_thresh_num]{};
  TH1F_LW* m_h_Et_eff[s_thresh_num]{};
  TH1F_LW* m_h_probe_eta_tot{};
  TH1F_LW* m_h_probe_phi_tot{};
  TH1F_LW* m_h_probe_Et_tot{};
  TH1F_LW* m_h_z_inv_mass{};


  //==========================================
  // Helper Functions
  //==========================================
  bool inGoodEtaRange(double eta);//Not working currently hardcoded
  double calcDeltaR(double eta1, double phi1, double eta2, double phi2);
  double correctDeltaPhi(double dPhi);



  }; 
//==============================================================================
} // end namespace
//==============================================================================
#endif
