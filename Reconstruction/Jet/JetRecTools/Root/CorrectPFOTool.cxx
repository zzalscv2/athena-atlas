/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// CorrectPFOTool.cxx

#include "JetRecTools/CorrectPFOTool.h"
#include <cmath>

CorrectPFOTool::CorrectPFOTool(const std::string &name):
  JetConstituentModifierBase(name), 
  m_weightPFOTool("") {


  // Configuration
  declareProperty("WeightPFOTool",   m_weightPFOTool,     "Name of tool that extracts the cPFO weights.");
  declareProperty("InputIsEM",       m_inputIsEM = true,  "True if neutral PFOs are EM scale clusters.");
  declareProperty("CalibratePFO",    m_calibrate = false, "True if LC calibration should be applied to EM PFOs.");
  declareProperty("CorrectNeutral",      m_correctneutral = true, "True to use the neutral component of PFlow.");
  declareProperty("CorrectCharged",      m_correctcharged = true, "True if use the charged component of PFlow.");
  declareProperty("UseChargedWeights",m_useChargedWeights = true, "True if we make use of weighting scheme for charged PFO");
  declareProperty("DoByVertex", m_doByVertex = false, "True to add vertex-by-vertex corrections for neutral PFOs");

  // Input properties
  declareProperty("VertexContainerKey", 
                  m_vertexContainer_key="PrimaryVertices",
                  "StoreGate key for the primary vertex container");
}

StatusCode CorrectPFOTool::initialize() {
  if(!m_correctneutral && !m_correctcharged) {
    ATH_MSG_ERROR("This tool is configured to do nothing!");
    return StatusCode::FAILURE;
  }
  if(m_inputType!=xAOD::Type::ParticleFlow) {
    ATH_MSG_ERROR("ChargedHadronSubtractionTool requires PFO inputs. It cannot operate on objects of type "
		  << m_inputType);
    return StatusCode::FAILURE;
  }
  if(m_useChargedWeights) {
    ATH_CHECK( m_weightPFOTool.retrieve() );
  }
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::process_impl(xAOD::IParticleContainer* cont) const {
  // Type-checking happens in the JetConstituentModifierBase class
  // so it is safe just to static_cast
  xAOD::PFOContainer* pfoCont = static_cast<xAOD::PFOContainer*> (cont);
  return !m_doByVertex ? correctPFO(*pfoCont) : correctPFOByVertex(*pfoCont);
}

const xAOD::Vertex* CorrectPFOTool::getPrimaryVertex() const {
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

StatusCode CorrectPFOTool::correctPFO(xAOD::PFOContainer& cont) const { 

  const xAOD::Vertex* vtx = nullptr;
  bool goodVtx = true;
  if(m_correctneutral) {
    vtx = getPrimaryVertex();
    if(vtx==nullptr) {
      ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
      return StatusCode::FAILURE;
    }
    // Don't correct to vertices outside the inner tracker, this can lead to pathologies
    // Numbers below are in mm and correspond to inner detector volume (also valid for ITk)
    float vtxRadius = TMath::Sqrt((vtx->x())*(vtx->x()) + (vtx->y())*(vtx->y()));
    goodVtx = ((vtxRadius < 1100) && (fabs(vtx->z()) < 3500));
    if (vtx->vertexType()==xAOD::VxType::NoVtx) {
      ATH_MSG_VERBOSE("No genuine primary vertex found. Will not apply origin correction");
      goodVtx = false;
    }
    else if (!goodVtx)
      ATH_MSG_VERBOSE("Primary vertex is outside of the inner detector volume. Will not apply origin correction");
  }

  //CP::PFO_JetMETConfig_inputScale inscale = m_inputIsEM ? CP::EM : CP::LC;
  const static SG::AuxElement::Accessor<char> PVMatchedAcc("matchedToPV");
  const static SG::AuxElement::Accessor<float> z0SinThetaPVAcc("z0SinThetaPV");
  const static SG::AuxElement::Accessor<float> z0SinThetaPUMinAcc("z0SinThetaPUMin");

  for ( xAOD::PFO* ppfo : cont ) {

    if ( fabs(ppfo->charge())<FLT_MIN) { // Neutral PFOs
      if(m_correctneutral) {
        ATH_CHECK( applyNeutralCorrection(*ppfo, *vtx, goodVtx) );
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop

  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::correctPFOByVertex(xAOD::PFOContainer& cont) const {

  //static const SG::AuxElement::Accessor< std::vector<unsigned> > matchedPVs("MatchingPVs");
  static const SG::AuxElement::Accessor<unsigned> copyIndex("ConstituentCopyIndex");
  
  // Retrieve Primary Vertices
  const xAOD::VertexContainer* vertices = nullptr;
  if(evtStore()->retrieve(vertices, m_vertexContainer_key).isFailure()
     || vertices->empty()){
      ATH_MSG_WARNING(" This event has no primary vertices " );
      return StatusCode::FAILURE;
  } 
  
  // Store the size in advance, as we will be extending it with duplicate neutrals adjusted to the vertex of interest
  const size_t numPFO {cont.size()};
  for (size_t iPFO {0}; iPFO < numPFO; ++iPFO)
  {
    xAOD::PFO* ppfo = cont.at(iPFO);
    if (ppfo == nullptr)
      ATH_MSG_WARNING("Got a nullptr PFO when trying to correct PFOs");
    else if (std::abs(ppfo->charge())<FLT_MIN)
    {
      if (m_correctneutral)
      {
        // Neutral PFOs - there are copies, one per vertex, already created
        // We need to now need to correct each copy to point to the corresponding vertex
        if (!copyIndex.isAvailable(*ppfo))
        {
          ATH_MSG_WARNING("Encountered a neutral per-vertex PFO object without the corresponding vertex index attribute");
          continue;
        }
        
        const unsigned iVtx = copyIndex(*ppfo);
        if (iVtx >= vertices->size())
        {
          ATH_MSG_WARNING("Encountered a neutral per-vertex PFO object with an index beyond the size of the vertex container");
          continue;
        }
        const xAOD::Vertex* vtx = vertices->at(iVtx);
          
        // Determine the correct four-vector interpretation for a neutral PFO associated with this vertex
        // Cannot use applyNeutralCorrection as that only used VxType::PriVtx, but we want to apply to all primary vertices
        // As such, extract the relevant parts and modify appropriately here
        if (!m_inputIsEM || m_calibrate) { // Use LC four-vector
          ppfo->setP4(ppfo->GetVertexCorrectedFourVec(*vtx));
        } else { // Use EM four-vector
          ppfo->setP4(ppfo->GetVertexCorrectedEMFourVec(*vtx));
        }

      }
    }
    else
    {
      // Charged PFOs - correct them in-place, as they already have a vertex interpretation
      if (m_correctcharged) ATH_CHECK(applyChargedCorrection(*ppfo));
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyNeutralCorrection(xAOD::PFO& pfo, const xAOD::Vertex& vtx, bool goodVtx) const {
  if (pfo.e() < FLT_MIN) { //This is necessary to avoid changing sign of pT for pT<0 PFO
    pfo.setP4(0,0,0,0);
    return StatusCode::SUCCESS;
  }
  if ( !m_inputIsEM || m_calibrate ) { // Use LC four-vector
    // Only correct if we really reconstructed a vertex
    if(goodVtx && vtx.vertexType()==xAOD::VxType::PriVtx)
      pfo.setP4(pfo.GetVertexCorrectedFourVec(vtx));
  }
  else { // Use EM four-vector
    // Only apply origin correction if we really reconstructed a vertex
    if(goodVtx && vtx.vertexType()==xAOD::VxType::PriVtx)
      pfo.setP4(pfo.GetVertexCorrectedEMFourVec(vtx));
    else
      pfo.setP4(pfo.p4EM()); // Just set EM 4-vec
  }
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyChargedCorrection(xAOD::PFO& pfo) const {
  if (m_useChargedWeights) {
    float weight = 0.0;
    ATH_CHECK( m_weightPFOTool->fillWeight( pfo, weight ) );
    ATH_MSG_VERBOSE("Fill pseudojet for CPFO with weighted pt: " << pfo.pt()*weight);
    pfo.setP4(pfo.p4()*weight);
  }//if should use charged PFO weighting scheme
  return StatusCode::SUCCESS;
}

