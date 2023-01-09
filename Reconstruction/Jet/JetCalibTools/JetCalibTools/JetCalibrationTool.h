///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// JetCalibrationTool.h 
// Header file for class JetCalibrationTool
// Author: Joe Taenzer <joseph.taenzer@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef JETCALIBTOOLS_APPLYJETCALIBRATION_H
#define JETCALIBTOOLS_APPLYJETCALIBRATION_H 1

#include <string.h>

#include <TString.h>
#include <TEnv.h>

#include "AsgTools/AsgTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEventShape/EventShape.h"

// JetCalibTools includes
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCalibTools/JetCalibrationToolBase.h"
#include "JetCalibTools/CalibrationMethods/JetPileupCorrection.h"
#include "JetCalibTools/CalibrationMethods/ResidualOffsetCorrection.h"
#include "JetCalibTools/CalibrationMethods/BcidOffsetCorrection.h"
#include "JetCalibTools/CalibrationMethods/EtaJESCorrection.h"
#include "JetCalibTools/CalibrationMethods/GlobalSequentialCorrection.h"
#include "JetCalibTools/CalibrationMethods/GlobalNNCalibration.h"
#include "JetCalibTools/CalibrationMethods/InsituDataCorrection.h"
#include "JetCalibTools/CalibrationMethods/JMSCorrection.h"
#include "JetCalibTools/CalibrationMethods/JetSmearingCorrection.h"
#include "JetCalibTools/CalibrationMethods/MC2MCCorrection.h"
#include "JetCalibTools/CalibrationMethods/GenericHistScaleCorrection.h"

class JetPileupCorrection;
class ResidualOffsetCorrection;
class BcidOffsetCorrection;
class EtaJESCorrection;
class GlobalSequentialCorrection;
class GlobalNNCalibration;
class InsituDataCorrection;
class JMSCorrection;
class JetSmearingCorrection;
class MC2MCCorrection;
class GenericHistScaleCorrection;

class JetCalibrationTool
  : public ::JetCalibrationToolBase {

  ASG_TOOL_CLASS2(JetCalibrationTool, IJetCalibrationTool, IJetModifier)

public:
  /// Constructor with parameters: 
  JetCalibrationTool(const std::string& name = "JetCalibrationTool");

  /// Destructor: 
  virtual ~JetCalibrationTool(); 

  enum jetScale { EM, LC, PFLOW };

  // Initialize the tool (default, assumes private members were set in the constructor)
  virtual StatusCode initializeTool(const std::string& name);

  // PATInterfaces inherited method
  StatusCode initialize();
 
  //PATInterfaces inherited method
  StatusCode finalize();

  ///Implement IJetCalibrationTool interface :
  virtual StatusCode applyCalibration(xAOD::Jet& jet) const;
  //PATInterfaces version of applyCalibration
  virtual CP::CorrectionCode applyCorrection(xAOD::Jet& jet);

  virtual int modify(xAOD::JetContainer&) const;
  virtual int modifyJet(xAOD::Jet&) const;

  // Retrieve pTmax from in situ corrections
  virtual VecD retrieveEtaIntercalPtMax(){return m_relInsituPtMax;}
  virtual VecD retrieveAbsoluteInsituPtMax(){return m_absInsituPtMax;}

  // Get the nominal resolution
  virtual StatusCode getNominalResolutionData(const xAOD::Jet& jet, double& resolution) const;
  virtual StatusCode getNominalResolutionMC(  const xAOD::Jet& jet, double& resolution) const;
  
protected:
  /// This is where the actual calibration code goes.
  virtual StatusCode calibrateImpl(xAOD::Jet& jet, JetEventInfo& jetEventInfo) const;

//Private methods
private:
  //Set event by event info like rho, mu, NPV
  virtual StatusCode initializeEvent(JetEventInfo& jetEventInfo) const;
  //Create the vector of calibration classes
  StatusCode getCalibClass(const std::string& name, TString calibration);

//Private members
private:
  //Variables for configuration
  std::string m_jetAlgo;
  std::string m_config;
  std::string m_calibSeq;
  std::string m_calibAreaTag;
  std::string m_originScale;
  std::string m_shower;
  bool m_devMode;
  bool m_isData;
  bool m_timeDependentCalib;
  bool m_originCorrectedClusters;
  std::string m_rhoKey;
  bool m_useNjetInResidual;
  bool m_doMuOnlyInResidual;
  bool m_saveAreaCorrectedScaleMomentum;
  float m_nJetPtThreshold;
  std::string m_nJetContainerName;
  std::string m_dir;
  std::string m_eInfoName;
  std::vector<TString> m_timeDependentInsituConfigs;
  std::vector<double>  m_runBins;
  bool m_doSetDetectorEta;
  // this member is unused, please remove if not needed:
  //   bool m_doOnlyResidual;
  std::string m_vertexContainerName;
  bool m_insituCombMassCalib;
  std::vector<TString> m_insituCombMassConfig;
  std::string m_rhoKey_config;

  //TEnv to hold the global text config
  TEnv * m_globalConfig;
  std::vector<TEnv*> m_globalTimeDependentConfigs;
  std::vector<TEnv*> m_globalInsituCombMassConfig;

  //Bools/enums to avoid string comparisons at run time
  jetScale m_jetScale;
  bool m_doBcid;
  bool m_doJetArea;
  bool m_doResidual;
  bool m_doOrigin;
  bool m_doGSC;
  bool m_doMC2MC;

  // vector with pTmax of each in situ correction
  VecD m_relInsituPtMax, m_absInsituPtMax;

  //Class objects for each calibration step
  std::vector<JetCalibrationToolBase*> m_calibClasses;
  BcidOffsetCorrection * m_bcidCorr;
  JetPileupCorrection * m_jetPileupCorr;
  EtaJESCorrection * m_etaJESCorr;
  GlobalSequentialCorrection * m_globalSequentialCorr;
  GlobalNNCalibration * m_globalNNCorr;
  InsituDataCorrection * m_insituDataCorr;
  std::vector<JetCalibrationToolBase*> m_insituTimeDependentCorr;
  JMSCorrection * m_jetMassCorr;
  JetSmearingCorrection* m_jetSmearCorr;
  MC2MCCorrection* m_jetMC2MCCorr;
  JMSCorrection *InsituCombMassCorr;
  std::vector<JetCalibrationToolBase*> m_insituCombMassCorr;
  GenericHistScaleCorrection* m_genericScaleCorr;

  // Try to use jet-attribute-specified origin vertex for calibration
  bool m_useOriginVertex;
}; 

#endif //> !JETCALIBTOOLS_APPLYJETCALIBRATION_H
