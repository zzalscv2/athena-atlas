/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// PseudoJetAlgorithm.cxx 

#include "JetRec/PseudoJetAlgorithm.h"
#include "JetRec/PseudoJetGetter.h"
#include "JetRec/IParticleExtractor.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"

// Fixed value by which to scale ghost kinematics
constexpr float ghostscale = 1e-40;

//**********************************************************************

StatusCode PseudoJetAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  // This is horrible, but still necessary for the time being
  // PJG needs to know if this is the basic EMTopo cluster collection
  // in order to change the cluster signal state.
  if ( m_label == "EMTopo") m_emtopo = true;
  // PFlow containers need to have PV matching applied
  if ( std::string(m_label).find("PFlow") != std::string::npos) m_pflow = true;
  if ( std::string(m_label).find("UFO") != std::string::npos) m_ufo = true;

  // "Ghost" in output collection name? If so is a ghost collection.
  m_isGhost = (m_outcoll.key()).find("Ghost") != std::string::npos;

  print();

  if(m_incoll.key().empty() || m_outcoll.key().empty()) {
    ATH_MSG_ERROR("Either input or output collection is empty!");
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_incoll.initialize() );
  ATH_CHECK( m_outcoll.initialize() );

  ATH_CHECK( m_vertexContainer_key.initialize(m_byVertex) );

  return StatusCode::SUCCESS;
}


//**********************************************************************

StatusCode PseudoJetAlgorithm::execute(const EventContext& ctx) const {
  ATH_MSG_VERBOSE("Executing " << name() << "...");

  // build and record the container
  auto incoll = SG::makeHandle(m_incoll, ctx);
  if( !incoll.isValid() ) {
    // Return SUCCESS to avoid crashing T0 jobs
    ATH_MSG_WARNING("Failed to retrieve " << m_incoll.key() << " for PseudoJet creation!" );
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Retrieved xAOD container " << m_incoll.key() << " of size " << incoll->size()
		<<  ", isGhost=" << m_isGhost);

  ATH_MSG_DEBUG("Creating PseudoJetContainer...");
  std::unique_ptr<PseudoJetContainer> pjcont( createPJContainer(*incoll, ctx) );

  auto outcoll = SG::makeHandle(m_outcoll, ctx);
  ATH_MSG_DEBUG("Created new PseudoJetContainer \"" << m_outcoll.key() << "\" with size " << pjcont->size());

  ATH_CHECK( outcoll.record(std::move(pjcont)) );

  return StatusCode::SUCCESS;
}


std::unique_ptr<PseudoJetContainer> PseudoJetAlgorithm::createPJContainer(const xAOD::IParticleContainer& cont, const EventContext& ctx) const {
  // create PseudoJets
  std::vector<fastjet::PseudoJet> vpj;

  #ifndef GENERATIONBASE
  if (m_byVertex){
    auto pvs = SG::makeHandle(m_vertexContainer_key, ctx);
    vpj = createPseudoJets(cont, pvs.cptr());
  }
  else{
    vpj = createPseudoJets(cont);
  }
  #else
  [[maybe_unused]] const EventContext& unused_ctx = ctx;
  vpj = createPseudoJets(cont);
  #endif

  // create an extractor to attach to the PJContainer -- this will be used by clients
  auto extractor = std::make_unique<IParticleExtractor>(&cont, m_label, m_isGhost);
  ATH_MSG_DEBUG("Created extractor: "  << extractor->toString(0));
  
  // ghostify the pseudojets if necessary
  if(m_isGhost){
    for(fastjet::PseudoJet& pj : vpj) {pj *= ghostscale;}
  }
  
  // Put the PseudoJetContainer together
  auto pjcont = std::make_unique<PseudoJetContainer>(std::move(extractor), vpj);
  ATH_MSG_DEBUG("New PseudoJetContainer size " << pjcont->size());

  return pjcont;
}


std::vector<fastjet::PseudoJet> 
PseudoJetAlgorithm::createPseudoJets(const xAOD::IParticleContainer& ips) const{
#ifndef GENERATIONBASE
  if (m_pflow || m_ufo) {return PseudoJetGetter::PFlowsToPJs(ips,m_skipNegativeEnergy.value(),m_useCharged.value(),m_useNeutral.value(),m_useChargedPV.value(),m_useChargedPUsideband.value(),m_ufo);}
  if (m_emtopo) {return PseudoJetGetter::EMToposToPJs(ips,m_skipNegativeEnergy.value());}
#endif
  return PseudoJetGetter::IParticlesToPJs(ips,m_skipNegativeEnergy.value());
}

#ifndef GENERATIONBASE
std::vector<fastjet::PseudoJet> 
PseudoJetAlgorithm::createPseudoJets(const xAOD::IParticleContainer& ips, const xAOD::VertexContainer* pvs) const{
  return PseudoJetGetter::ByVertexPFlowsToPJs(ips, pvs, m_skipNegativeEnergy.value(),m_useCharged.value(),m_useNeutral.value(),m_ufo);
}
#endif

//**********************************************************************

void PseudoJetAlgorithm::print() const {
  std::string sskip = m_skipNegativeEnergy ? "true" : "false";
  // May want to change to DEBUG
  ATH_MSG_INFO("Properties for PseudoJetGetter " << name());
  ATH_MSG_INFO("             Label: " << m_label);
  ATH_MSG_INFO("   Input container: " << m_incoll.key());
  ATH_MSG_INFO("  Output container: " << m_outcoll.key());
  ATH_MSG_INFO("   Skip negative E: " << sskip);
  ATH_MSG_INFO("         Is EMTopo: " << m_emtopo);
  ATH_MSG_INFO("          Is PFlow: " << m_pflow);
  ATH_MSG_INFO("            Is UFO: " << m_ufo);
  ATH_MSG_INFO("          Is ghost: " << m_isGhost);
  ATH_MSG_INFO(" Treat negative E as ghost: " << m_negEnergyAsGhosts.value());
  ATH_MSG_INFO(" Running by-vertex reco: " << m_byVertex.value());
  ATH_MSG_INFO(" Vertex input container: " << m_vertexContainer_key.key());

  if(m_pflow){
    ATH_MSG_INFO("   Use charged FEs: " << m_useCharged.value());
    ATH_MSG_INFO("   Use neutral FEs: " << m_useNeutral.value());
    ATH_MSG_INFO("Use charged PV FEs: " << m_useChargedPV.value());
    ATH_MSG_INFO("   PU sideband def: " << m_useChargedPUsideband.value());
  }
  if(m_ufo){
    ATH_MSG_INFO("   Use charged UFOs: " << m_useCharged.value());
    ATH_MSG_INFO("   Use neutral UFOs: " << m_useNeutral.value());
  }
}

//**********************************************************************
