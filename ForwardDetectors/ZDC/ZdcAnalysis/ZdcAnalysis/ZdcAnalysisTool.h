/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_ZDCANALYSISTOOL_H
#define ZDCANALYSIS_ZDCANALYSISTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"

#include "xAODForward/ZdcModuleContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"


#include "ZdcAnalysis/ZDCDataAnalyzer.h"
#include "ZdcAnalysis/ZDCTriggerEfficiency.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "ZdcAnalysis/ZDCMsg.h"

#include "TF1.h"
#include "TMath.h"

#include "xAODEventInfo/EventInfo.h"


namespace ZDC
{

class ZdcAnalysisTool : public virtual IZdcAnalysisTool, public asg::AsgTool
{

  ASG_TOOL_CLASS(ZdcAnalysisTool, ZDC::IZdcAnalysisTool)

public:
  ZdcAnalysisTool(const std::string& name);
  virtual ~ZdcAnalysisTool() override;

  //interface from AsgTool
  StatusCode initialize() override;
  void initialize80MHz();
  void initialize40MHz();
  void initializeTriggerEffs(unsigned int runNumber);

  StatusCode recoZdcModule(const xAOD::ZdcModule& module) override;
  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer) override;
  StatusCode reprocessZdc() override;

  // methods for processing, used for decoration
  bool sigprocMaxFinder(const std::vector<unsigned short>& adc, float deltaT, float& amp, float& time, float& qual);
  bool sigprocSincInterp(const std::vector<unsigned short>& adc, float deltaT, float& amp, float& time, float& qual);

  void setEnergyCalibrations(unsigned int runNumber);
  void setTimeCalibrations(unsigned int runNumber);

  float getModuleSum(int side);

  //float getModuleFitAmplitude(int side, int imod);
  //float getModuleFitT0(int side, int imod);
  //float getModuleAmplitude(int side, int imod);
  //float getModuleTime(int side, int imod);
  //float getModuleChisq(int side, int imod);
  //float getModuleStatus(int side, int imod);

  //float getModuleCalibAmplitude(int side, int imod);
  //float getModuleCalibTime(int side, int imod);

  float getCalibModuleSum(int side);
  float getCalibModuleSumErr(int side);

  float getUncalibModuleSum(int side);
  float getUncalibModuleSumErr(int side);

  float getAverageTime(int side);
  bool  sideFailed(int side);
  unsigned int getModuleMask();

  double getTriggerEfficiency(int side);
  double getTriggerEfficiencyUncertainty(int side);
  bool m_doTimeCalib;

  const ZDCDataAnalyzer* getDataAnalyzer() {return m_zdcDataAnalyzer.get();}

  static void SetDebugLevel(int debugLevel = 0)
  {
    s_debugLevel = debugLevel;
  }

  ZDCMsg::MessageFunctionPtr MakeMessageFunction()
  {
    std::function<bool(int, std::string)> msgFunction = [this](int level, std::string message)-> bool
    {
      MSG::Level theLevel = static_cast<MSG::Level>(level);
      bool test = theLevel >= this->msg().level();
      if (test) {
        this->msg() << message << endmsg;
      }
      return test;
    };

    return ZDCMsg::MessageFunctionPtr(new ZDCMsg::MessageFunction(msgFunction));
  }

  void Dump_setting() {
    if (s_debugLevel > 2) {
      ATH_MSG_INFO("========================================================================================================================");
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
          ATH_MSG_INFO("-------------------------------------------------------------------------------------------------------------------");
          ATH_MSG_INFO("Side: " << i << ", Module: " << j);
          m_zdcDataAnalyzer->GetPulseAnalyzer(i, j)->Dump_setting();
        }
      }
      ATH_MSG_INFO("========================================================================================================================");
    }
  }

private:
  // Private methods
  //
  std::unique_ptr<ZDCDataAnalyzer> initializeDefault();
  std::unique_ptr<ZDCDataAnalyzer> initializePbPb2015G4();
  std::unique_ptr<ZDCDataAnalyzer> initializepPb2016();
  std::unique_ptr<ZDCDataAnalyzer> initializePbPb2018();

  StatusCode configureNewRun(unsigned int runNumber);

  // Data members
  //
  std::string m_name;
  bool m_init;
  std::string m_configuration;
  std::string m_zdcAnalysisConfigPath;
  std::string m_zdcEnergyCalibFileName;
  std::string m_zdcTimeCalibFileName;
  std::string m_zdcTriggerEffParamsFileName;

  bool m_writeAux;
  std::string m_auxSuffix;

  bool m_eventReady;
  unsigned int m_runNumber;
  unsigned int m_lumiBlock;

  // internal functions
  std::unique_ptr<TF1> m_tf1SincInterp;

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo",
      "Location of the event info."};
  SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleWriteKey {
    this, "ZdcModuleWriteKey", "ZdcSums",
      "Output location of ZDC reprocessed data"};

  std::string m_zdcModuleContainerName;
  const xAOD::ZdcModuleContainer* m_zdcModules {nullptr};
  bool m_flipEMDelay;
  bool m_lowGainOnly;
  bool m_combineDelay;
  bool m_doCalib;
  bool m_doTrigEff;
  int m_forceCalibRun;
  int m_forceCalibLB;

  //  Parameters that control the pulse fitting analysis
  //
  unsigned int m_numSample;
  float m_deltaTSample;
  unsigned int m_presample;
  unsigned int m_peakSample;
  float m_Peak2ndDerivThresh;
  float m_t0;
  float m_delayDeltaT;
  float m_tau1;
  float m_tau2;
  bool m_fixTau1;
  bool m_fixTau2;
  float m_deltaTCut;
  float m_ChisqRatioCut;

  std::shared_ptr<ZDCDataAnalyzer> m_zdcDataAnalyzer;
  std::shared_ptr<ZDCDataAnalyzer> m_zdcDataAnalyzer_40MHz;
  std::shared_ptr<ZDCDataAnalyzer> m_zdcDataAnalyzer_80MHz;
  ZDCDataAnalyzer::ZDCModuleFloatArray m_peak2ndDerivMinSamples;
  ZDCDataAnalyzer::ZDCModuleFloatArray m_peak2ndDerivMinThresholdsHG;
  ZDCDataAnalyzer::ZDCModuleFloatArray m_peak2ndDerivMinThresholdsLG;


  std::shared_ptr<ZDCTriggerEfficiency> m_zdcTriggerEfficiency;

  static int s_debugLevel;

};

} // namespace ZDC

#endif



