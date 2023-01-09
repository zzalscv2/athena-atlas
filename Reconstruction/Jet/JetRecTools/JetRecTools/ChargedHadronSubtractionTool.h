/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETRECTOOLS_CHARGEDHADRONSUBTRACTIONTOOL_H
#define JETRECTOOLS_CHARGEDHADRONSUBTRACTIONTOOL_H

////////////////////////////////////////////
/// \class ChargedHadronSubtractionTool
///
/// Removes charged PFO not associated to the PV
///
/// \author John Stupak, Jennifer Roloff, and Steven Schramm
//////////////////////////////////////////////////

#include <string>
#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODBase/IParticleContainer.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "JetEDM/TrackVertexAssociation.h"
#include "xAODTracking/VertexContainer.h" 
#include "xAODPFlow/PFOContainer.h"

class ChargedHadronSubtractionTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(ChargedHadronSubtractionTool, IJetConstituentModifier)

  public:
  
  ChargedHadronSubtractionTool(const std::string& name);

  // Check that the configuration is reasonable
  StatusCode initialize();

  private:
  // Implement the correction
  StatusCode process_impl(xAOD::IParticleContainer* cont) const; 
  // Type-specific operation
  StatusCode matchToPrimaryVertex(xAOD::PFOContainer& cont) const;
  StatusCode matchByPrimaryVertex(xAOD::PFOContainer& cont) const;

  const xAOD::Vertex* getPrimaryVertex() const;
  static double calcAbsZ0SinTheta(const xAOD::TrackParticle& trk, const xAOD::Vertex& vtx);
  bool m_useTrackToVertexTool;
  float m_z0sinThetaCutValue;
  bool m_byVertex;
	
  std::string m_vertexContainer_key;
  std::string m_trkVtxAssoc_key;
};

#endif
