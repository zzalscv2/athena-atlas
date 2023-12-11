/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Source code for the CaloClusterConstituentsOrigin implementation class
// Michael Nelson, CERN & University of Oxford
//
//
#include "AsgDataHandles/ReadHandle.h"
#include "JetRecTools/CaloClusterConstituentsOrigin.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

CaloClusterConstituentsOrigin::CaloClusterConstituentsOrigin(const std::string & name): JetConstituentModifierBase(name) {
  declareProperty("VertexContainer",m_readVertexContainer_key="PrimaryVertices");
}

StatusCode CaloClusterConstituentsOrigin::initialize() {
  if(m_inputType!=xAOD::Type::CaloCluster) {
    ATH_MSG_ERROR("As the name suggests, CaloClusterConstituentsOrigin cannot operate on objects of type "
		  << m_inputType);
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_readVertexContainer_key.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode CaloClusterConstituentsOrigin::process_impl(xAOD::IParticleContainer* cont) const {
   xAOD::CaloClusterContainer* clust = static_cast<xAOD::CaloClusterContainer*> (cont); // Get CaloCluster container

   auto handle = SG::makeHandle(m_readVertexContainer_key);
   ATH_CHECK(handle.isValid());
   const auto *vertexContainer = handle.cptr();
       
   for(const xAOD::Vertex* pv : *vertexContainer) {
     // Apply the origin correction iff a PV is identified
     if(pv->vertexType()==xAOD::VxType::PriVtx) {return correctToOriginVtx(*clust, *pv);}
   }
   // Exit silently if we did not find a designated primary vertex
   // This leaves the clusters uncorrected, which is fine
   return StatusCode::SUCCESS;
}
// Apply PV origin correction and decorate the CaloCluster container appropriately.
// We depend on the ClustersAtEMScaleTool to change the calibration state if needed
// so we can just correct the position based on the default four-momentum
StatusCode CaloClusterConstituentsOrigin::correctToOriginVtx(xAOD::CaloClusterContainer& cont, const xAOD::Vertex& vert) const {
  for(xAOD::CaloCluster* cl : cont) {
    if(cl->calE()>1e-9) {
      xAOD::CaloVertexedTopoCluster corrCL( *cl,vert.position());
      cl->setEta(corrCL.eta());
      cl->setPhi(corrCL.phi());
    }
  }
  return StatusCode::SUCCESS;
}
