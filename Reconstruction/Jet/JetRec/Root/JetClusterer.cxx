/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include "AsgDataHandles/ReadHandle.h"
#include "JetRec/JetClusterer.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/config.h"
#include "fastjet/contrib/VariableRPlugin.hh"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "JetEDM/FastJetUtils.h"

#include "JetRec/PseudoJetTranslator.h"
#include "JetEDM/VertexIndexedConstituentUserInfo.h"
#include "xAODTracking/VertexContainer.h"

namespace JetClustererHelper
{

  void seedsFromEventInfo(const xAOD::EventInfo *ei, std::vector<int> &seeds)
  {
    // const xAOD::EventInfo* pevinfo = handle.cptr();
    auto ievt = ei->eventNumber();
    auto irun = ei->runNumber();

    if (ei->eventType(xAOD::EventInfo::IS_SIMULATION))
    {
      // For MC, use the channel and MC event number
      ievt = ei->mcEventNumber();
      irun = ei->mcChannelNumber();
    }
    seeds.push_back(ievt);
    seeds.push_back(irun);
  }
}

StatusCode JetClusterer::initialize()
{

  ATH_MSG_DEBUG("Initializing...");
  xAOD::JetAlgorithmType::ID ialg = xAOD::JetAlgorithmType::algId(m_jetalg);
  m_fjalg = xAOD::JetAlgorithmType::fastJetDef(ialg);
  if (m_fjalg == fastjet::undefined_jet_algorithm)
  {
    ATH_MSG_ERROR("Invalid jet algorithm name: " << m_jetalg);
    ATH_MSG_ERROR("Allowed values are Kt, CamKt, AntiKt, etc.");
    return StatusCode::FAILURE;
  }
  if (m_jetrad <= 0)
  {
    ATH_MSG_ERROR("Invalid jet size parameter: " << m_jetrad);
    return StatusCode::FAILURE;
  }

  // buld an empty ClusterSequence, just for the fastjet splash screen to appear during initialization (?)
  fastjet::JetDefinition jetdef(m_fjalg, m_jetrad);
  m_useArea = m_ghostarea > 0;

  PseudoJetVector empty;
  fastjet::ClusterSequence cs(empty, jetdef);
  cs.inclusive_jets(m_ptmin);
  m_isVariableR = m_minrad >= 0.0 && m_massscale >= 0.0;

  // Input DataHandles
  if (!m_finalPseudoJets.empty())
  {
    ATH_MSG_WARNING("A non-empty value was found for the FinalPseudoJets WriteHandleKey -- this will be ignored!");
  }

  ATH_CHECK(m_eventinfokey.initialize());
  ATH_CHECK(m_inputPseudoJets.initialize());
  m_finalPseudoJets = name() + "FinalPJ";
  ATH_CHECK(m_finalPseudoJets.initialize());
  m_clusterSequence = name() + "ClusterSequence";
  ATH_CHECK(m_clusterSequence.initialize());

  return StatusCode::SUCCESS;
}

// -----------------------
// Build the cluster sequence
std::unique_ptr<fastjet::ClusterSequence> JetClusterer::buildClusterSequence(const PseudoJetVector *pseudoJetVector) const
{

  fastjet::JetDefinition jetdef(m_fjalg, m_jetrad);
  using fastjet::contrib::VariableRPlugin;
  std::unique_ptr<VariableRPlugin> VRJetPlugin(nullptr);

  if (m_isVariableR)
  {
    /* clustering algorithm
     * They correspond to p parameter of Sequential recombination algs
     * AKTLIKE = -1, CALIKE = 0, KTLIKE = 1
     */
    VariableRPlugin::ClusterType VRClusterType = VariableRPlugin::AKTLIKE;
    switch (m_fjalg)
    {
    case fastjet::kt_algorithm:
      VRClusterType = VariableRPlugin::KTLIKE;
      break;
    case fastjet::antikt_algorithm:
      VRClusterType = VariableRPlugin::AKTLIKE;
      break;
    case fastjet::cambridge_algorithm:
      VRClusterType = VariableRPlugin::CALIKE;
      break;
    default:
      ATH_MSG_ERROR("Unsupported clustering algorithm for Variable-R jet finding.");
      return nullptr;
    }
    VRJetPlugin = std::make_unique<VariableRPlugin>(m_massscale, m_minrad, m_jetrad, VRClusterType, false);
    jetdef = fastjet::JetDefinition(VRJetPlugin.get());
  }

  std::unique_ptr<fastjet::ClusterSequence> clSequence(nullptr);

  if (m_useArea)
  {
    // Prepare ghost area specifications -------------
    ATH_MSG_DEBUG("Creating input area cluster sequence");
    bool seedsok = true;
    fastjet::AreaDefinition adef = buildAreaDefinition(seedsok);

    if (seedsok)
    {
      clSequence = std::make_unique<fastjet::ClusterSequenceArea>(*pseudoJetVector, jetdef, adef);
    }
    else
    {
      return nullptr;
    }
  }
  else
  {
    ATH_MSG_DEBUG("Creating input cluster sequence");
    clSequence = std::make_unique<fastjet::ClusterSequence>(*pseudoJetVector, jetdef);
  }

  return clSequence;
}

void JetClusterer::processPseudoJet(const fastjet::PseudoJet &pj, const PseudoJetContainer &pjCont, xAOD::JetContainer *jets, const xAOD::Vertex *originVertex) const
{

  // -------------------------------------
  // translate to xAOD::Jet
  ATH_MSG_DEBUG("Converting pseudojets to xAOD::Jet");
  static const SG::AuxElement::Accessor<const fastjet::PseudoJet *> pjAccessor("PseudoJet");
  PseudoJetTranslator pjTranslator(m_useArea, m_useArea);

  // create the xAOD::Jet from the PseudoJet, doing the signal &  constituents extraction
  xAOD::Jet &jet = pjTranslator.translate(pj, pjCont, *jets, originVertex);

  // Add the PseudoJet onto the xAOD jet. Maybe we should do it in the above JetFromPseudojet call ??
  pjAccessor(jet) = &pj;

  jet.setInputType(xAOD::JetInput::Type(m_inputType.value()));
  xAOD::JetAlgorithmType::ID ialg = xAOD::JetAlgorithmType::algId(m_fjalg);
  jet.setAlgorithmType(ialg);
  jet.setSizeParameter(m_jetrad.value());
  if (m_isVariableR)
  {
    jet.setAttribute(xAOD::JetAttribute::VariableRMinRadius, m_minrad.value());
    jet.setAttribute(xAOD::JetAttribute::VariableRMassScale, m_massscale.value());
  }
  if (m_useArea)
    jet.setAttribute(xAOD::JetAttribute::JetGhostArea, m_ghostarea.value());

  ATH_MSG_VERBOSE("  xAOD::Jet with pt " << std::setprecision(4) << jet.pt() * 1e-3 << " has " << jet.getConstituents().size() << " constituents");
  ATH_MSG_VERBOSE("  Leading constituent is of type " << jet.getConstituents()[0].rawConstituent()->type());

  // Perhaps better to make configurable
  if (originVertex)
  {
    jet.setAssociatedObject("OriginVertex", originVertex);
  }
  else
  {
    ATH_MSG_VERBOSE("Could not set OriginVertex for jet!");
  }
}

std::pair<std::unique_ptr<xAOD::JetContainer>, std::unique_ptr<SG::IAuxStore>> JetClusterer::getJets() const
{
  // Return this in case of any problems
  auto nullreturn = std::make_pair(std::unique_ptr<xAOD::JetContainer>(nullptr), std::unique_ptr<SG::IAuxStore>(nullptr));

  // -----------------------
  // retrieve input
  SG::ReadHandle<PseudoJetContainer> pjContHandle(m_inputPseudoJets);
  if (!pjContHandle.isValid())
  {
    ATH_MSG_ERROR("No valid PseudoJetContainer with key " << m_inputPseudoJets.key());
    return nullreturn;
  }

  // Build the container to be returned
  // Avoid memory leaks with unique_ptr
  auto jets = std::make_unique<xAOD::JetContainer>();
  auto auxCont = std::make_unique<xAOD::JetAuxContainer>();
  jets->setStore(auxCont.get());

  const PseudoJetVector *pseudoJetVector = pjContHandle->casVectorPseudoJet();
  ATH_MSG_DEBUG("Pseudojet input container has size " << pseudoJetVector->size());

  std::unique_ptr<fastjet::ClusterSequence> clSequence = buildClusterSequence(pseudoJetVector);

  if (!clSequence)
    return nullreturn;

  // -----------------------
  // Build a new pointer to a PseudoJetVector containing the final PseudoJet
  // This allows us to own the vector of PseudoJet which we will put in the evt store.
  // Thus the contained PseudoJet will be kept frozen there and we can safely use pointer to them from the xAOD::Jet objects
  auto pjVector = std::make_unique<PseudoJetVector>(fastjet::sorted_by_pt(clSequence->inclusive_jets(m_ptmin)));
  ATH_MSG_DEBUG("Found jet count: " << pjVector->size());
  if (msgLvl(MSG::VERBOSE))
  {
    for (const auto &pj : *pjVector)
    {
      msg() << "  Pseudojet with pt " << std::setprecision(4) << pj.Et() * 1e-3 << " has " << pj.constituents().size() << " constituents" << endmsg;
    }
  }

  // No PseudoJets, so there's nothing else to do
  // Delete the cluster sequence before we go
  if (!pjVector->empty())
  {

    for (const fastjet::PseudoJet &pj : *pjVector)
    {
      processPseudoJet(pj, *pjContHandle, jets.get(), nullptr);
    }

    // -------------------------------------
    // record final PseudoJetVector
    SG::WriteHandle<PseudoJetVector> pjVectorHandle(m_finalPseudoJets);
    if (!pjVectorHandle.record(std::move(pjVector)))
    {
      ATH_MSG_ERROR("Can't record PseudoJetVector under key " << m_finalPseudoJets);
      return nullreturn;
    }
    // -------------------------------------
    // record ClusterSequence
    SG::WriteHandle<jet::ClusterSequence> clusterSeqHandle(m_clusterSequence);
    if (!clusterSeqHandle.record(std::move(clSequence)))
    {
      ATH_MSG_ERROR("Can't record ClusterSequence under key " << m_clusterSequence);
      return nullreturn;
    }
  }

  ATH_MSG_DEBUG("Reconstructed jet count: " << jets->size() << "  clusterseq=" << clSequence.get());
  // Return the jet container and aux, use move to transfer
  // ownership of pointers to caller
  return std::make_pair(std::move(jets), std::move(auxCont));
}

fastjet::AreaDefinition JetClusterer::buildAreaDefinition(bool &seedsok) const
{

  fastjet::GhostedAreaSpec gspec(5.0, 1, m_ghostarea);
  seedsok = true;
  std::vector<int> seeds;

  if (m_ranopt == 1)
  {
    // Use run/event number as random number seeds.
    auto evtInfoHandle = SG::makeHandle(m_eventinfokey);
    if (!evtInfoHandle.isValid())
    {
      ATH_MSG_ERROR("Unable to retrieve event info");
      seedsok = false;
      return fastjet::AreaDefinition();
    }

    JetClustererHelper::seedsFromEventInfo(evtInfoHandle.cptr(), seeds);
  }

  ATH_MSG_DEBUG("Active area specs:");
  ATH_MSG_DEBUG("  Requested ghost area: " << m_ghostarea);
  ATH_MSG_DEBUG("     Actual ghost area: " << gspec.actual_ghost_area());
  ATH_MSG_DEBUG("               Max eta: " << gspec.ghost_etamax());
  ATH_MSG_DEBUG("              # ghosts: " << gspec.n_ghosts());
  ATH_MSG_DEBUG("       # rapidity bins: " << gspec.nrap());
  ATH_MSG_DEBUG("            # phi bins: " << gspec.nphi());

  if (seeds.size() == 2)
  {
    ATH_MSG_DEBUG("          Random seeds: " << seeds[0] << ", " << seeds[1]);
  }
  else
  {
    ATH_MSG_WARNING("Random generator size is not 2: " << seeds.size());
    ATH_MSG_DEBUG("          Random seeds: ");
    for (auto seed : seeds)
      ATH_MSG_DEBUG("                 " << seed);
  }

  // We use with_fixed_seed() as recommended for thread safety in
  // fastjet 3.4.0.
  return fastjet::AreaDefinition(fastjet::active_area,
                                 gspec)
      .with_fixed_seed(seeds);
}
