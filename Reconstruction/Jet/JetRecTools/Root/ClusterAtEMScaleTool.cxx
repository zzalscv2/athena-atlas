/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <vector>
#include "JetRecTools/ClusterAtEMScaleTool.h"

using namespace std;


ClusterAtEMScaleTool::ClusterAtEMScaleTool(const std::string& name) : JetConstituentModifierBase(name)
{
}

StatusCode ClusterAtEMScaleTool::initialize() {
  if(m_inputType!=xAOD::Type::CaloCluster) {
    ATH_MSG_ERROR("As the name suggests, ClusterAtEMScaleTool cannot operate on objects of type "
		  << m_inputType);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode ClusterAtEMScaleTool::setClustersToEMScale(xAOD::CaloClusterContainer& cont) const {
  for(xAOD::CaloCluster* cl : cont ) {
		cl->setCalE( cl->rawE() );
		cl->setCalPhi( cl->rawPhi() );
		cl->setCalEta( cl->rawEta() );
  }

  return StatusCode::SUCCESS;
}

StatusCode ClusterAtEMScaleTool::process_impl(xAOD::IParticleContainer* cont) const {
  xAOD::CaloClusterContainer* clust = dynamic_cast<xAOD::CaloClusterContainer*> (cont); // Get CaloCluster container
  if(clust) return setClustersToEMScale(*clust);
  return StatusCode::FAILURE;
}


ClusterAtEMScaleTool::~ClusterAtEMScaleTool()= default;
