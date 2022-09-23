/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_MUHADPROCESSORTOOL_H
#define DITAUREC_MUHADPROCESSORTOOL_H

#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/ITauToolExecBase.h"
#include "AsgTools/ToolHandleArray.h"

#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

/**
 * @brief modified from tauRec/TauProcessorTool by Justin Griffiths.
 * 
 * @author Lianyou SHAN
 *  This ProcessorTool should be limited within AOD->dAOD derivation jobs only for R21.2 yet.
 *  There is no intension/plan to migrate this ProcessorTool into R22, 
 *  neither intension to use it in analysisi with Root standalone.                                                                             
 */

class MuHadProcessorTool : public asg::AsgTool, virtual public ITauToolExecBase {
 public:

  ASG_TOOL_CLASS1( MuHadProcessorTool, ITauToolExecBase )
    
  MuHadProcessorTool(const std::string& type);
  ~MuHadProcessorTool() override = default ;

  StatusCode initialize() override ;
  StatusCode execute() override ;
  StatusCode finalize() override ;

 private:

  std::string                 m_ConfigPath;
  std::string                 m_tauContainerName;
  std::string                 m_tauTrackName;
  std::string                 m_tauAuxContainerName; 
  bool                          m_configured;
  bool                          m_AODmode;
  bool                          m_deep_copy_SecVtxContainer;
  bool                          m_deep_copy_TauTrackContainer;
  bool                          m_saveCluster ;
  bool                          m_TrkClassifided ;
  xAOD::TauJetParameters::TauTrackFlag m_isoTrackType;

  TauEventData m_data;
  ToolHandleArray<ITauToolBase>  m_tools;
  ToolHandleArray<ITauToolBase>  m_RNN_tools;

  int m_MaxMuonIDWP ;
  ToolHandle<CP::IMuonCalibrationAndSmearingTool> m_thMuonCalibrationTool;
  ToolHandle<CP::IMuonSelectionTool> m_thMuonSelectionTool;
  float m_jetCone  ;

 private :

   struct overlapMuon {
     TLorentzVector IDtrkP4 ;
     TLorentzVector ClustP4 ;
     int Qlt ;
     overlapMuon () 
     {
       IDtrkP4 = { 0., 0., 0., 0. } ;
       ClustP4 = { 0., 0., 0., 0  } ;
       Qlt = 4 ;
     }
   } ;

};

#endif //TAUREC_TAUPROCESSORTOOL_H
