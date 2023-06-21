// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#ifndef EFTRACKING_SMEARINGALG_H
#define EFTRACKING_SMEARINGALG_H 

#include "FakeTrackSmearer.h"
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h" 
#include "xAODTracking/TrackParticleAuxContainer.h" 
#include "xAODTracking/TrackParticle.h"



class EFTrackingSmearingAlg: public ::AthHistogramAlgorithm { 
 public: 
  EFTrackingSmearingAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EFTrackingSmearingAlg(); 

  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

 private: 

  std::string m_inputTrackContainerName;
  std::string m_outputTrackContainerName;

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticleKey { this, "InputTrackParticleContainer", "InDetTrackParticles_tosmear",
                                                                          "key for retrieval of input TrackParticles" };

  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outputTrackParticleKey { this, "OutputTrackParticleContainer", "InDetTrackParticles_smeared",
                                                                          "key for retrieval of output TrackParticles" };


  IntegerProperty  m_SigmaScaleFactor      {this, "SmearingScaleFactor", 1, "Set the smearing SF value for the sigmas"};
  BooleanProperty  m_UseResolutionPtCutOff {this, "UseResolutionPtCutOff", false, "Apply ptCutoff on smearing"};
  DoubleProperty   m_SetResolutionPtCutOff {this, "SetResolutionPtCutOff", 0., "Set ptCutoff off for semaring"};
  DoubleProperty   m_inputTracksPtCut      {this, "InputTracksPtCutGeV", 4.0, "Set input track pT cut"};
  DoubleProperty   m_outputTracksPtCut     {this, "OutputTracksPtCutGeV", 4.0, "Set the output track pT cut"};
  DoubleProperty   m_smearedTrackEfficiency{this, "SmearedTrackEfficiency", 1.0, "Set track efficiency for smearing"};
  BooleanProperty  m_parameterizedTrackEfficiency{this, "ParameterizedTrackEfficiency", false, "Enable parameterized efficiency for smearing"};
  BooleanProperty  m_enableMonitoring      {this, "EnableMonitoring", false, "Enable debugging monitoring of the algorithm"};
  // these are for fake emulations
  BooleanProperty  m_EnableFakes      {this,"IncludeDuplicatesAndFakes",false,"Enable fake track production"};
  BooleanProperty  m_FakeKillerEnable {this,"FakeKillerEnable",false, "disable inclusion of broad fakes"};
  BooleanProperty  m_IncludeFakesInResolutionCalculation{this,"IncludeFakesInResolutionCalculation",false, 
                              "modify calculation of covariance parameters to include weighted combination of core and broad RMS"};
  BooleanProperty  m_UseCoinToss       {this,"UseCoinToss",false, 
                              "if True, fakes are generated with flat inefficiency, else use poissonian criteria"};
  
  LongLongProperty m_RandomSeed{this,"RandomSeed",0};
  
  
  void *m_mySmearer;
  StatusCode book_histograms();
}; 

#endif //> !FTRACKING_SMEARINGALG_H
