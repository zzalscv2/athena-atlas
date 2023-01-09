/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// CorrectPFOTool.h

#ifndef JETRECTOOLS_CORRECTPFOTOOL_H
#define JETRECTOOLS_CORRECTPFOTOOL_H

////////////////////////////////////////////
/// \class CorrectPFOTool
///
/// Applies various corrections to PFO
///
/// Properties:
///  InputIsEM - If true, EM-scale is used for the neutral pflow
///  CalibratePFO - If true the EM-scale pflow is calibrated
///
/// \author Jennifer Roloff, John Stupak, and Steven Schramm - based on PFlowPseudoJetGetter by P-A Delsart, D. Adams
//////////////////////////////////////////////////

#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODTracking/VertexContainer.h" 
#include "xAODPFlow/PFOContainer.h"
#include "AsgTools/ToolHandle.h"
#include "PFlowUtils/IWeightPFOTool.h"

class CorrectPFOTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(CorrectPFOTool, IJetConstituentModifier)

  public:

  CorrectPFOTool(const std::string& name);
  
  // Check that the configuration is reasonable
  StatusCode initialize() override final;

 private:

  // Implement the correction
  StatusCode process_impl(xAOD::IParticleContainer* cont) const override final;
  // Type-specific operation
  StatusCode correctPFO(xAOD::PFOContainer& cont) const;
  StatusCode correctPFOByVertex(xAOD::PFOContainer& cont) const;

  const xAOD::Vertex* getPrimaryVertex() const;
  StatusCode applyNeutralCorrection(xAOD::PFO& pfo, const xAOD::Vertex& vtx) const;
  StatusCode applyChargedCorrection(xAOD::PFO& pfo) const;

  bool m_inputIsEM;   /// If true EM clusters are used for neutral PFOs.
  bool m_calibrate;   /// If true, EM PFOs are calibrated to LC.
  bool m_correctneutral;   //If true, correct neutral PFO
  bool m_correctcharged;   //If true, correct charged PFO
  bool m_useChargedWeights; //If true, them we make use of the charged PFO weighting scheme
  bool m_doByVertex;  // If true, then we add vertex-by-vertex corrections as auxdata for neutral PFOs
  ToolHandle<CP::IWeightPFOTool> m_weightPFOTool;

  std::string m_vertexContainer_key;
  
};

#endif
