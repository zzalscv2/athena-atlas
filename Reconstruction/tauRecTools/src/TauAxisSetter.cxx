/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TauAxisSetter.h"
#include "tauRecTools/HelperFunctions.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"



TauAxisSetter::TauAxisSetter(const std::string& name) :
TauRecToolBase(name) {
}



StatusCode TauAxisSetter::execute(xAOD::TauJet& tau) const {
  if (! tau.jetLink().isValid()) {
    ATH_MSG_ERROR("Tau jet link is invalid.");
    return StatusCode::FAILURE;
  }
  const xAOD::Jet* jetSeed = tau.jet();

  // Barycenter is the sum of cluster p4 in the seed jet
  TLorentzVector baryCenter;  
  
  xAOD::JetConstituentVector constituents = jetSeed->getConstituents();
  for (const xAOD::JetConstituent* constituent : constituents) {
    TLorentzVector constituentP4 = tauRecTools::GetConstituentP4(*constituent);
    baryCenter += constituentP4;
  }
  
  ATH_MSG_DEBUG("barycenter (eta, phi): "  << baryCenter.Eta() << " " << baryCenter.Phi());

  // Detector axis is the total p4 of clusterswithin m_clusterCone core of the barycenter 
  TLorentzVector tauDetectorAxis;
  int nConstituents = 0;

  for (const xAOD::JetConstituent* constituent : constituents) {
    TLorentzVector constituentP4 = tauRecTools::GetConstituentP4(*constituent);
    
    double dR = baryCenter.DeltaR(constituentP4);
    if (dR > m_clusterCone) continue;

    tauDetectorAxis += constituentP4;
    ++nConstituents;
  }

  if (0 == nConstituents) {
    ATH_MSG_DEBUG("this tau candidate does not have any constituent clusters!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("detector axis:" << tauDetectorAxis.Pt()<< " " << tauDetectorAxis.Eta() << " " << tauDetectorAxis.Phi()  << " " << tauDetectorAxis.E());
  tau.setP4(tauDetectorAxis.Pt(), tauDetectorAxis.Eta(), tauDetectorAxis.Phi(), tau.m());
  tau.setP4(xAOD::TauJetParameters::DetectorAxis, tauDetectorAxis.Pt(), tauDetectorAxis.Eta(), tauDetectorAxis.Phi(), tauDetectorAxis.M());


  if (m_doVertexCorrection) {
    // Tau intermediate axis (corrected for tau vertex)
    TLorentzVector tauInterAxis;

    const xAOD::Vertex* jetVertex = getJetVertex(*jetSeed);
    
    const xAOD::Vertex* tauVertex = nullptr;
    if (tau.vertexLink().isValid()) tauVertex = tau.vertex();

    // Redo the vertex correction when tau vertex is different from jet vertex
    if (jetVertex != tauVertex) {
      // If seed jet has a vertex, then tau must have one
      if (tauVertex == nullptr) {
        ATH_MSG_WARNING("The seed jet has a vertex, while the tau candidate does not. It should not happen.");
        return StatusCode::FAILURE;
      }
    
      // Relative position of the tau vertex and jet vertex
      Amg::Vector3D position = tauVertex->position();
      if (jetVertex != nullptr) {
        position -= jetVertex->position();
      }

      // Barycenter at the tau vertex
      TLorentzVector baryCenterTauVertex; 
 
      // Loop over the jet constituents, and calculate the barycenter using the four momentum 
      // correctd to point at tau vertex 
      for (const xAOD::JetConstituent* constituent : constituents) {
        TLorentzVector constituentP4 = getVertexCorrectedP4(*constituent, position);
        baryCenterTauVertex += constituentP4;
      }
      ATH_MSG_DEBUG("barycenter (eta, phi) at tau vertex: "  << baryCenterTauVertex.Eta() << " " << baryCenterTauVertex.Phi());

      // Tau intrmediate axis is the four momentum (corrected to point at tau verteex) of clusters 
      // within m_clusterCone of the baryon center
      for (const xAOD::JetConstituent* constituent : constituents) {
        TLorentzVector constituentP4 = getVertexCorrectedP4(*constituent, position);
        double dR = baryCenterTauVertex.DeltaR(constituentP4);
        if (dR > m_clusterCone) continue;
        
        tauInterAxis += constituentP4;
      }
    }
    else {  
      tauInterAxis = tauDetectorAxis;
    }

    ATH_MSG_DEBUG("tau axis:" << tauInterAxis.Pt()<< " " << tauInterAxis.Eta() << " " << tauInterAxis.Phi()  << " " << tauInterAxis.E() );
    tau.setP4(tauInterAxis.Pt(), tauInterAxis.Eta(), tauInterAxis.Phi(), tau.m());
    tau.setP4(xAOD::TauJetParameters::IntermediateAxis, tauInterAxis.Pt(), tauInterAxis.Eta(), tauInterAxis.Phi(), tauInterAxis.M());
  } // End of m_doVertexCorrection

  return StatusCode::SUCCESS;
}



const xAOD::Vertex* TauAxisSetter::getJetVertex(const xAOD::Jet& jet) const {
  
  const xAOD::Vertex* jetVertex = nullptr;
  
  if (m_doJetVertexCorrection && !inTrigger()) {
    bool isAvailable = jet.getAssociatedObject("OriginVertex", jetVertex);
    if (!isAvailable) {
      ATH_MSG_WARNING("OriginVertex not available !");
      jetVertex = nullptr;
    }
  }

  return jetVertex;
}



TLorentzVector TauAxisSetter::getVertexCorrectedP4(const xAOD::JetConstituent& constituent,
                                                   const Amg::Vector3D& position) const {
  TLorentzVector vertexCorrectedP4;
  
  if (constituent.type() == xAOD::Type::CaloCluster) {
	const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>(constituent.rawConstituent());
    vertexCorrectedP4 = xAOD::CaloVertexedTopoCluster(*cluster, position).p4();;
  }
  else if (constituent.type() == xAOD::Type::ParticleFlow) {
	const xAOD::PFO* pfo = static_cast<const xAOD::PFO*>(constituent.rawConstituent());
    vertexCorrectedP4 = getVertexCorrectedP4(*pfo, position); 
  }
  else {
	ATH_MSG_WARNING("Seed jet constituent type not supported, will not do vertex correction !");
    vertexCorrectedP4 = tauRecTools::GetConstituentP4(constituent);
  }
 
  return vertexCorrectedP4; 
}



TLorentzVector TauAxisSetter::getVertexCorrectedP4(const xAOD::PFO& pfo,
                                                   const Amg::Vector3D& position) const {
  TLorentzVector vertexCorrectedP4;

  // Only perfrom vertex corretion for neutral PFO
  if (!pfo.isCharged()) {
    // Convert the type to TVector3 for PFO
    TVector3 pos(position.x(), position.y(), position.z());

    // If there is a vertex correction in jet reconstruction, then pfo.p4() is the four momentum 
    // at EM scale. Otherwise, pfo.p4() is at LC scale (not clear), and pfo.p4EM() is the four 
    // momentum at EM scale. 
    // TODO: May need further modifications, depending on how the jet reconstruction fix ATLJETMET-1280
    // The strategy only works for PFlow at EM scale.
    if (m_doJetVertexCorrection && !inTrigger()) {
      vertexCorrectedP4 = pfo.GetVertexCorrectedFourVec(pos);
    }
    else {
      vertexCorrectedP4 = pfo.GetVertexCorrectedEMFourVec(pos);
    }
  }
  else {
    vertexCorrectedP4 = pfo.p4();
  }

  ATH_MSG_DEBUG("Original pfo four momentum, pt: " << pfo.pt() << 
                  " eta: " << pfo.eta() << " phi: " << pfo.phi() << " e: " << pfo.e());
  ATH_MSG_DEBUG("Vertex corrected four momentum, pt: " << vertexCorrectedP4.Pt() << 
                " eta: " << vertexCorrectedP4.Eta() << " phi: " << vertexCorrectedP4.Phi() << " e: " << vertexCorrectedP4.E());
      
  return vertexCorrectedP4;
}

#endif
