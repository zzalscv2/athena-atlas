/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PFlowByVertexPseudoJetGetter.cxx

#include "JetRecTools/PFlowByVertexPseudoJetGetter.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODTracking/VertexContainer.h" 
#include "AthContainers/ConstDataVector.h"
#include "JetEDM/VertexIndexedConstituentUserInfo.h"

PFlowByVertexPseudoJetGetter::PFlowByVertexPseudoJetGetter(const std::string &name)
  : PseudoJetGetter(name) {
  declareProperty("UseCharged", m_useCharged =true, "Whether or not to use charged PFO inputs");
  declareProperty("UseNeutral", m_useNeutral =true, "Whether or not to use neutral PFO inputs");
  declareProperty("UseChargedPV", m_useChargedPV =true, "Whether or not to require charged PFO inputs come from the PV of interest");
  declareProperty("UseChargedPUsideband", m_useChargedPUsideband =false, "Whether or not to use charged particles around, but not matched to, the PV of interest");
  declareProperty("UniqueChargedVertex", m_uniqueVertex =true, "Whether or not to define a unique charged-PFO-to-vertex association, currently only true is supported");
}

int PFlowByVertexPseudoJetGetter::appendTo(PseudoJetVector& psjs, const LabelIndex* pli) const { 
  const xAOD::PFOContainer* pfos(nullptr);
  ATH_MSG_DEBUG("Retrieving xAOD container " << m_incoll << ", ghost scale="
		<< m_ghostscale  <<  ", isGhost=" << bool(m_ghostscale));
  if( evtStore()->retrieve(pfos,m_incoll).isFailure() ) {
    ATH_MSG_ERROR("Unable to find input collection: " << m_incoll);
    return 1;
  }


  // Select only the PFOs we want to use for jet building
  // These could be from any vertex - the vertex specification happens as part of buildCUI (below)
  ConstDataVector<xAOD::PFOContainer> filteredpfos(SG::VIEW_ELEMENTS);
  const static SG::AuxElement::Accessor< std::vector<unsigned> > matchedPVs("MatchingPVs");       // For charged PFOs
  const static SG::AuxElement::Accessor< std::vector<unsigned> > matchedPUSBs("MatchingPUsidebands"); // For charged PFOs
  for(const auto& pfo : *pfos) {
    // Technically this also skips 0 energy, which has no effect other than
    // on memory size, but is used in some workflows for pileup rejection
    bool reject = (m_skipNegativeEnergy && pfo->e()<FLT_MIN);
    // Although it may seem pointless, we need to keep charged PFOs with energy==0
    // because for MET TST with PFlow, there may be high pt charged PFOs that receive
    // a weight of 0 due to being in dense showers, but need to be present for
    // overlap removal, because they don't retain these weights when added to the TST
    if( std::abs(pfo->charge())>FLT_MIN) {
      if (m_useCharged) reject = false; // put back in 0 energy tracks for MET 
      if (m_useChargedPV && !matchedPVs(*pfo).size()) reject = true;
      if (m_useChargedPUsideband && !matchedPUSBs(*pfo).size()) reject = true;
      if (!m_useCharged) reject = true;
    }else{
      if (!m_useNeutral) reject = true;
    }
    if(!reject) filteredpfos.push_back(pfo);
  }

  return append(filteredpfos, psjs, pli);
}

jet::IConstituentUserInfo* PFlowByVertexPseudoJetGetter::buildCUI(const xAOD::IParticle* ppar, jet::IConstituentUserInfo::Index idx, const LabelIndex* pli) const
{
  const static SG::AuxElement::Accessor<             unsigned  > copyIndex("ConstituentCopyIndex");     // For neutral PFOs
  const static SG::AuxElement::Accessor< std::vector<unsigned> > matchedPVs("MatchingPVs");             // For charged PFOs
  const static SG::AuxElement::Accessor< std::vector<unsigned> > matchedPUSBs("MatchingPUsidebands");   // For charged PFOs
  
  const xAOD::PFO* pfo = dynamic_cast<const xAOD::PFO*>(ppar);
  if (!pfo)
  {
    ATH_MSG_ERROR("Something has gone terribly wrong: the low-level pseudojet code was given PFOs but has returned non-PFOs for the CUI - this should never happen");
    return nullptr;
  }

  unsigned vertexIndex {0};
  if (std::abs(pfo->charge())>FLT_MIN)
  {
    // Charged PFOs - use the vertex matched to the track
    if (matchedPVs.isAvailable(*pfo) && matchedPVs(*pfo).size())
    {
      // A charged PFO can potentially match multiple vertices, depending on the matching criteria used
      if (m_uniqueVertex)
        // For now, just use the first match, to be further optimised in future releases, especially in the context of AMVF
        // Also can add the part for PU sidebands if this is determined to be useful in the by-vertex case, for future releases
        vertexIndex = matchedPVs(*pfo).at(0);
      else
      {
        ATH_MSG_ERROR("Non-unique vertex matching in vertex-by-vertex jet reco is not currently supported");
        return nullptr;
      }
    }
  }
  else
  {
    // Neutral PFOs - there is one neutral PFO corrected to point to each vertex of interest
    // As such, just get the vertex index that this neutral PFO corresponds to
    if (copyIndex.isAvailable(*pfo))
      vertexIndex = copyIndex(*pfo);
  }

  // Retrieve Primary Vertices
  const xAOD::VertexContainer* pvtxs = nullptr;
  if(evtStore()->retrieve(pvtxs, "PrimaryVertices").isFailure() // This should probably be made to be configurable in future releases
     || pvtxs->empty()){
      ATH_MSG_WARNING("Failed to find the primary vertex container when building VertexIndexedConstituentUserInfo" );
      return nullptr;
  }

  // Get the specified vertex and build the VertexIndexedConstituentUserInfo
  for (const xAOD::Vertex* vertex : *pvtxs)
    if (vertex->index() == vertexIndex)
      return new jet::VertexIndexedConstituentUserInfo(*ppar, idx, pli, vertex);

  // If we get here, we failed to find the vertex
  ATH_MSG_ERROR("Failed to find the specified vertex used to build the PFO");
  return nullptr;
}
