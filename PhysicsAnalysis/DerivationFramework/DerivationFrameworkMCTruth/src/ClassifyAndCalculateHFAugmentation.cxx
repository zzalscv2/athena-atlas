/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ClassifyAndCalculateHFAugmentation.cxx                               //
// Implementation file for class ClassifyAndCalculateHFAugmentation     //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
////////////////////////////////////////////////////////////////////////// 

// Header of the class ClassifyAndCalculateHFAugmentation.

#include "DerivationFrameworkMCTruth/ClassifyAndCalculateHFAugmentation.h"

namespace DerivationFramework {

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------- Constructor/Destructor --------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  ClassifyAndCalculateHFAugmentation::ClassifyAndCalculateHFAugmentation(const std::string& t, const std::string& n, const IInterface* p) : 
  AthAlgTool(t,n,p),                 // Athena tool.
  m_JetMatchingTool_Tool(""),        // Hadron-jet matching tool.
  m_HFClassification_tool(""),       // HF classifier tool.
  m_HadronOriginClassifier_Tool("")  // HF hadron origin tool.
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    
    // Declare a set of tool properties to set them exertanally:
    //  -m_HFClassification_tool:       The tool to compute the HF classifier.
    //  -m_HadronOriginClassifier_Tool: The tool to determine the origin of the HF hadrons.
    //  -m_JetMatchingTool_Tool:        The tool to match the hadrons with the jets.

    declareProperty("ClassifyAndComputeHFtool",   m_HFClassification_tool);
    declareProperty("HadronOriginClassifierTool", m_HadronOriginClassifier_Tool);
    declareProperty("JetMatchingTool",            m_JetMatchingTool_Tool);
  }

  ClassifyAndCalculateHFAugmentation::~ClassifyAndCalculateHFAugmentation(){}

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  --------------------------------------------------------- Initialize/Finalize ---------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  StatusCode ClassifyAndCalculateHFAugmentation::initialize(){

    ATH_MSG_INFO("Initialize HF computation");

    // Print the string variables.

    ATH_MSG_INFO("Jets Container Name "            << m_jetCollectionName);
    ATH_MSG_INFO("Truth Particles Container Name " << m_TruthParticleContainerName);
    ATH_MSG_INFO("HF Classifier Name "             << m_hfDecorationName);
    ATH_MSG_INFO("Simple HF Classifier Name "      << m_SimplehfDecorationName);

    // Check if the necessary tools can be retrieved.

    if(m_HFClassification_tool.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to retrieve the tool " << m_HFClassification_tool);
      return StatusCode::FAILURE;
    }

    if(m_HadronOriginClassifier_Tool.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to retrieve the tool " << m_HadronOriginClassifier_Tool);
      return StatusCode::FAILURE;
    }
  
    if(m_JetMatchingTool_Tool.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to retrieve the tool " << m_JetMatchingTool_Tool);
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode ClassifyAndCalculateHFAugmentation::finalize(){
    return StatusCode::SUCCESS;
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------------- AddBranches -------------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  StatusCode ClassifyAndCalculateHFAugmentation::addBranches() const
  {

    // Create two pointers where the HF classifer and the simple HF classifier will be saved.

    std::unique_ptr< int > hfclassif(new int());
    std::unique_ptr< int > simpleclassif(new int());

    // Retrieve the truth particle container from the event.

    const xAOD::TruthParticleContainer* xTruthParticleContainer = nullptr;

    if (evtStore()->retrieve(xTruthParticleContainer, m_TruthParticleContainerName).isFailure()) {
      ATH_MSG_ERROR("could not retrieve TruthParticleContainer '" << m_TruthParticleContainerName << "'");
      return StatusCode::FAILURE;
    }

    // Retrieve the jets container from the event.

    const xAOD::JetContainer* JetCollection = nullptr;
    
    if(evtStore()->retrieve(JetCollection,m_jetCollectionName).isFailure()) {
      ATH_MSG_ERROR("could not retrieve JetContainer '" << m_jetCollectionName << "'");
      return StatusCode::FAILURE;
    }

    // Compute a map that associates the HF hadrons with their origin using the tool m_HadronOriginClassifier_Tool.

    std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id> hadronMap = m_HadronOriginClassifier_Tool->GetOriginMap(); 

    // Create a map with a list of matched hadrons for each jet.

    std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>> particleMatch = m_JetMatchingTool_Tool->matchHadronsToJets(xTruthParticleContainer, JetCollection);

    // Calculate the necessary information from the jets to compute the HF classifier.

    m_HFClassification_tool->flagJets(JetCollection, particleMatch, hadronMap, m_hfDecorationName);

    // Compute the HF classifier and the simple HF classifier.

    *hfclassif     = m_HFClassification_tool->computeHFClassification(JetCollection, m_hfDecorationName);
    *simpleclassif = m_HFClassification_tool->getSimpleClassification(*hfclassif);

    // Retrieve the EventInfo container from the event.

    const xAOD::EventInfo* EventInfo = nullptr;
    
    if(evtStore()->retrieve(EventInfo,"EventInfo").isFailure()) {
      ATH_MSG_ERROR("could not retrieve 'EventInfo'");
      return StatusCode::FAILURE;
    }

    // Dectorate the EventInfo with the HF Classification and the simple version

    SG::AuxElement::Decorator< int > decorator_HFClassification(m_hfDecorationName); 
    decorator_HFClassification(*EventInfo) = *hfclassif;

    SG::AuxElement::Decorator< int > decorator_SimpleHFClassification(m_SimplehfDecorationName); 
    decorator_SimpleHFClassification(*EventInfo) = *simpleclassif;

    return StatusCode::SUCCESS;
  }
}
