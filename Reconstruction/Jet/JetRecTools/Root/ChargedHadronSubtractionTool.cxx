/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetRecTools/ChargedHadronSubtractionTool.h"

using namespace std;

ChargedHadronSubtractionTool::ChargedHadronSubtractionTool(const std::string& name) : JetConstituentModifierBase(name)
{

  declareProperty("UseTrackToVertexTool", m_useTrackToVertexTool=false, "True if we will use the track to vertex tool");

  declareProperty("TrackVertexAssociation", 
                  m_trkVtxAssoc_key="JetTrackVtxAssoc",
                  "StoreGate key for the TrackVertexAssociation object");
  declareProperty("VertexContainerKey", 
                  m_vertexContainer_key="PrimaryVertices",
                  "StoreGate key for the primary vertex container");

  declareProperty("Z0sinThetaCutValue", m_z0sinThetaCutValue=2.0, "True if we will use the track to vertex tool");

  declareProperty("DoByVertex", m_byVertex=false, "True if we should match to each primary vertex, not just PV0");

}

StatusCode ChargedHadronSubtractionTool::initialize() {
  if(m_inputType!=xAOD::Type::ParticleFlow) {
    ATH_MSG_ERROR("ChargedHadronSubtractionTool requires PFO inputs. It cannot operate on objects of type "
		  << m_inputType);
  return StatusCode::FAILURE;
  }
  if (m_byVertex && m_useTrackToVertexTool) {
    ATH_MSG_ERROR("ChargedHadronSubtractionTool does not support both TrackVertexAssociation and DoByVertex");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode ChargedHadronSubtractionTool::process_impl(xAOD::IParticleContainer* cont) const {
  // Type-checking happens in the JetConstituentModifierBase class
  // so it is safe just to static_cast
  xAOD::PFOContainer* pfoCont = static_cast<xAOD::PFOContainer*> (cont);
  return !m_byVertex ? matchToPrimaryVertex(*pfoCont) : matchByPrimaryVertex(*pfoCont);
}

const xAOD::Vertex* ChargedHadronSubtractionTool::getPrimaryVertex() const {
  // Retrieve Primary Vertices
  const xAOD::VertexContainer* pvtxs = nullptr;
  if(evtStore()->retrieve(pvtxs, m_vertexContainer_key).isFailure()
     || pvtxs->empty()){
      ATH_MSG_WARNING(" This event has no primary vertices " );
      return nullptr;
  } 

  //Usually the 0th vertex is the primary one, but this is not always 
  // the case. So we will choose the first vertex of type PriVtx
  for (auto theVertex : *pvtxs) {
    if (theVertex->vertexType()==xAOD::VxType::PriVtx) {
      return theVertex;
    }//If we have a vertex of type primary vertex
  }//iterate over the vertices and check their type

  // If we failed to find an appropriate vertex, return the dummy vertex
  ATH_MSG_VERBOSE("Could not find a primary vertex in this event " );
  for (auto theVertex : *pvtxs) {
    if (theVertex->vertexType()==xAOD::VxType::NoVtx) {
      return theVertex;
    }
  }

  // If there is no primary vertex, then we cannot do PV matching.
  return nullptr;
}

double ChargedHadronSubtractionTool::calcAbsZ0SinTheta(const xAOD::TrackParticle& trk, const xAOD::Vertex& vtx)
{
  // vtz.z() provides z of that vertex w.r.t the center of the beamspot (z = 0).
  // Thus we correct the track z0 to be w.r.t z = 0
  const float z0 = trk.z0() + trk.vz() - vtx.z();
  const float theta = trk.theta();
  return std::abs(z0*std::sin(theta));
}


StatusCode ChargedHadronSubtractionTool::matchToPrimaryVertex(xAOD::PFOContainer& cont) const {
  const static SG::AuxElement::Accessor<char> PVMatchedAcc("matchedToPV");
  const static SG::AuxElement::Accessor<char> PUsidebandMatchedAcc("matchedToPUsideband");

  // Use only one of TVA or PV
  const jet::TrackVertexAssociation* trkVtxAssoc = nullptr;
  const xAOD::Vertex* vtx = nullptr;
  if(m_useTrackToVertexTool) {
    if(evtStore()->retrieve(trkVtxAssoc,m_trkVtxAssoc_key).isFailure()){
      ATH_MSG_ERROR("Can't retrieve TrackVertexAssociation : "<< m_trkVtxAssoc_key); 
      return StatusCode::FAILURE;
    }
  } else {
    vtx = getPrimaryVertex();
    if(vtx==nullptr) {
      ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
      return StatusCode::FAILURE;
    } else if (vtx->vertexType()==xAOD::VxType::NoVtx) {
      ATH_MSG_VERBOSE("No genuine primary vertex found. Will consider all PFOs matched.");
    }
  }

  for ( xAOD::PFO* ppfo : cont ) {
    // Ignore neutral PFOs
    if(std::abs(ppfo->charge()) < FLT_MIN) continue;

    bool matchedToPrimaryVertex = false;
    bool matchedToPileupSideband = false;
    const xAOD::TrackParticle* ptrk = ppfo->track(0);
    if(ptrk==nullptr) {
      ATH_MSG_WARNING("Charged PFO with index " << ppfo->index() << " has no ID track!");
      continue;
    }
    if(trkVtxAssoc) { // Use TrackVertexAssociation
      const xAOD::Vertex* thisTracksVertex = trkVtxAssoc->associatedVertex(ptrk);
      matchedToPrimaryVertex = (xAOD::VxType::PriVtx == thisTracksVertex->vertexType());
    } else { // Use Primary Vertex
      if(vtx->vertexType()==xAOD::VxType::NoVtx) { // No reconstructed vertices
	matchedToPrimaryVertex = true; // simply match all cPFOs in this case
      } else { // Had a good reconstructed vertex.
        const double absZ0sinTheta = calcAbsZ0SinTheta(*ptrk,*vtx);
	matchedToPrimaryVertex = ( absZ0sinTheta < m_z0sinThetaCutValue );
        if (absZ0sinTheta < 2.0*m_z0sinThetaCutValue && absZ0sinTheta >= m_z0sinThetaCutValue ) matchedToPileupSideband = true;
      }
    } // TVA vs PV decision
    PVMatchedAcc(*ppfo) = matchedToPrimaryVertex;
    PUsidebandMatchedAcc(*ppfo) = matchedToPileupSideband;
  }

  return StatusCode::SUCCESS;
}

StatusCode ChargedHadronSubtractionTool::matchByPrimaryVertex(xAOD::PFOContainer& cont) const
{
  const static SG::AuxElement::Accessor< std::vector<unsigned> > matchingPVs("MatchingPVs");
  const static SG::AuxElement::Accessor< std::vector<unsigned> > matchingPUSBs("MatchingPUsidebands");

  // Retrieve Primary Vertices
  const xAOD::VertexContainer* pvtxs = nullptr;
  if(evtStore()->retrieve(pvtxs, m_vertexContainer_key).isFailure()
     || pvtxs->empty()){
      ATH_MSG_WARNING(" This event has no primary vertices to match to PFOs" );
      return StatusCode::FAILURE;
  } 

  for (xAOD::PFO* ppfo : cont) {
    // Ignore neutral PFOs
    if(std::abs(ppfo->charge()) < FLT_MIN) continue;

    // Get the track for this charged PFO
    const xAOD::TrackParticle* ptrk = ppfo->track(0);
    if(ptrk==nullptr) {
      ATH_MSG_WARNING("Charged PFO with index " << ppfo->index() << " has no ID track!");
      continue;
    }

    std::vector<unsigned> matchingVertexList;
    std::vector<unsigned> matchingPUSBList;

    // Loop over the primary vertices to determine which ones potentially match
    for (const xAOD::Vertex* vtx : *pvtxs) {
      bool matchedToVertex = false;
      bool matchedToPUsideband = false;

      if (vtx == nullptr) {
        ATH_MSG_WARNING("Encountered a nullptr vertex when trying to match charged PFOs to vertices");
        continue;
      } else if(vtx->vertexType()==xAOD::VxType::NoVtx) { // No reconstructed vertices
	matchedToVertex = true; // simply match all cPFOs in this case
      } else { // Had a good reconstructed vertex
        const double absZ0sinTheta = calcAbsZ0SinTheta(*ptrk,*vtx);
	matchedToVertex = ( absZ0sinTheta < m_z0sinThetaCutValue );
        if (absZ0sinTheta < 2.0*m_z0sinThetaCutValue && absZ0sinTheta >= m_z0sinThetaCutValue ) matchedToPUsideband = true;
      }

      if (matchedToVertex) matchingVertexList.push_back(vtx->index());
      if (matchedToPUsideband) matchingPUSBList.push_back(vtx->index());
    }

    matchingPVs(*ppfo) = std::move(matchingVertexList);
    matchingPUSBs(*ppfo) = std::move(matchingPUSBList);
  }

  return StatusCode::SUCCESS;
}

