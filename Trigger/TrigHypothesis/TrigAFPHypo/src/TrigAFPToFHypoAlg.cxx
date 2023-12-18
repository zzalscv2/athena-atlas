/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigAFPToFHypoAlg.h"

using namespace TrigCompositeUtils;

TrigAFPToFHypoAlg::TrigAFPToFHypoAlg(const std::string &name, ISvcLocator *pSvcLocator): 
  ::HypoBase(name, pSvcLocator)
{
}

StatusCode TrigAFPToFHypoAlg::initialize()
{
  ATH_MSG_DEBUG("TrigAFPToFHypoAlg::initialize()");
  ATH_CHECK(m_inputIdVtxKey.initialize()); //ID vertex
  ATH_CHECK(m_inputAfpVtxKey.initialize()); //ToF vertex
  ATH_CHECK(m_hypoTools.retrieve());
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode TrigAFPToFHypoAlg::execute(const EventContext &context) const
{
  ATH_MSG_DEBUG("TrigAFPToFHypoAlg::execute()");

  // Create the input and output decision containers
  SG::ReadHandle<DecisionContainer> inputDecisionsHandle(decisionInput(), context);
  SG::WriteHandle<DecisionContainer> outputDecisionsHandle = createAndStore(decisionOutput(), context ); 

  // Jet vertex container
  SG::ReadHandle<xAOD::VertexContainer> idVtxHandle(m_inputIdVtxKey, context);
  ATH_CHECK(idVtxHandle.isValid());

  // AFP ToF vertex container
  SG::ReadHandle<xAOD::AFPVertexContainer> afpVtxHandle(m_inputAfpVtxKey, context);
  ATH_CHECK(afpVtxHandle.isValid());

  // monitor reason for skipping hypo tool
  auto skipHypoTool = Monitored::Scalar<int>("skipHypoTool", 0);

  // vertex handle error checking
  if (!afpVtxHandle.isValid()) {
    ATH_MSG_DEBUG("AFP vertex handle invalid");
    skipHypoTool = 1;
  }
  if (!idVtxHandle.isValid()) {
    ATH_MSG_DEBUG("ID vertex handle invalid");
    skipHypoTool = 2;
  }

  // check vertex handle size 
  if (afpVtxHandle->size() == 0) {
    ATH_MSG_DEBUG("No AFP vertices");
    skipHypoTool = 3;
  }
  if (idVtxHandle->size() == 0) {
    ATH_MSG_DEBUG("No ID vertices");
    skipHypoTool = 4;
  }

  // check if the only vertex has tracks (exclude 0 AFP vertices)
  if (skipHypoTool != 3 && idVtxHandle->size() == 1 && idVtxHandle->at(0)->vxTrackAtVertexAvailable() == false) {
    ATH_MSG_DEBUG("The only ID vertex has no tracks"); 
    skipHypoTool = 5;
  }

  // monitor number of vertices
  auto n_afp_vertex = Monitored::Scalar<size_t>("afp_vtx_count", 0);
  auto n_id_vertex = Monitored::Scalar<size_t>("id_vtx_count", 0);

  // loop over AFP vertices 
  for (size_t i = 0; i < afpVtxHandle->size(); ++i) {
    const xAOD::AFPVertex* afpVtx = afpVtxHandle->at(i);
    // if hypo tool is not skipped, print vertex information
    if (skipHypoTool == 0) ATH_MSG_DEBUG("AFP vertex [" << i << "] z position: " << afpVtx->position());
    auto afp_vtx_z = Monitored::Scalar<float>("afp_vtx_z", afpVtx->position());
    Monitored::fill(m_monTool, afp_vtx_z);
    n_afp_vertex += 1;
  }
  Monitored::fill(m_monTool, n_afp_vertex);

  // loop over ID vertices
  for (size_t i = 0; i < idVtxHandle->size(); ++i) {
    const xAOD::Vertex* idVtx = idVtxHandle->at(i);
    // if hypo tool is not skipped, print vertex information
    if (skipHypoTool == 0) ATH_MSG_DEBUG("ID vertex [" << i << "] z position: " << idVtx->z()
                                        << " chi2: " << idVtx->chiSquared()
                                        << " ndof: " << idVtx->numberDoF());
    // skip dummy vertex
    if (idVtx->vxTrackAtVertexAvailable() == false) continue;
    if (idVtx->nTrackParticles() == 0) continue; 
    
    auto id_vtx_z = Monitored::Scalar<float>("id_vtx_z", idVtx->z());
    Monitored::fill(m_monTool, id_vtx_z);
    n_id_vertex += 1;
  }
  Monitored::fill(m_monTool,n_id_vertex);

  // Vector of structs to pass to hypo tool
  std::vector<TrigAFPToFHypoTool::AFPToFHypoToolInfo> hypoToolInput;

  // monitor the reason for skipping the hypo tool
  Monitored::fill(m_monTool, skipHypoTool);

  if (skipHypoTool != 0)
  {
    ATH_MSG_DEBUG("Skipping hypo tool");
  }
  else
  {
    for (size_t i = 0; i < inputDecisionsHandle->size(); ++i) {
      const Decision* previousDecision = inputDecisionsHandle->at(i);

      const xAOD::AFPVertex* afpVtx = afpVtxHandle->at(0);
      const xAOD::Vertex* idVtx = idVtxHandle->at(0);

      // monitor delta z between vertices
      auto delta_z = Monitored::Scalar<float>("delta_z", afpVtx->position() - idVtx->z());
      Monitored::fill(m_monTool, delta_z);

      // Create an output Decision Object to track this object in the trigger navigation.
      Decision* outputDecision = newDecisionIn(outputDecisionsHandle.ptr(), hypoAlgNodeName());

      // We link this 'outputDecision' to its parent, 'inputDecision'
      linkToPrevious(outputDecision, previousDecision, context);

      // We attach the "feature" ElementLink which associates the DecisionObject with the AFP vertex
      outputDecision->setObjectLink(featureString(), ElementLink<xAOD::AFPVertexContainer>(*afpVtxHandle, i, context));

      // We collect the set of chains which were active on this inputDecision coming in to this Step.
      DecisionIDContainer inputPassingChains;
      decisionIDs(previousDecision, inputPassingChains);

      hypoToolInput.emplace_back(inputPassingChains, afpVtx, idVtx, outputDecision);
    }
  }

  // Each individual chain has its own HypoTool, we loop over them and give them the hypoToolInput to discriminate over
  for (const auto &tool : m_hypoTools) {
    ATH_CHECK(tool->decide(hypoToolInput));
  }

  // Performs some runtime-checks.
  ATH_CHECK( hypoBaseOutputProcessing(outputDecisionsHandle) );

  return StatusCode::SUCCESS;
}
