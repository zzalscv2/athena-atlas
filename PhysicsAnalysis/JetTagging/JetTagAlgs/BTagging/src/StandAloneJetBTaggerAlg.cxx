/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "BTagging/StandAloneJetBTaggerAlg.h"

#include "xAODCore/ShallowCopy.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODBTagging/BTagVertexContainer.h"
#include "xAODBTagging/BTagVertexAuxContainer.h"

#include "BTagging/JetBTaggerTool.h"

#include <iostream>
#include <string>

namespace Analysis {


StandAloneJetBTaggerAlg::StandAloneJetBTaggerAlg(const std::string& n, ISvcLocator *p) : 
  AthAlgorithm(n,p),
  //m_BTagName(""),
  m_JetCollectionName(""),
  m_suffix(""),
  m_dupPFlow(false),
  m_JetBTaggerTool("Analysis::JetBTaggerTool")
{
  declareProperty( "JetBTaggerTool", m_JetBTaggerTool);
  declareProperty( "outputCollectionSuffix", m_suffix );
  declareProperty( "JetCollectionName", m_JetCollectionName );
  declareProperty( "DuplicatePFlow", m_dupPFlow);
}

StandAloneJetBTaggerAlg::~StandAloneJetBTaggerAlg()
{
  /////....
}


StatusCode StandAloneJetBTaggerAlg::initialize() {

  /// retrieve the main JetBTaggerTool
  if ( m_JetBTaggerTool.retrieve().isFailure() ) {
    ATH_MSG_FATAL("#BTAG# Failed to retrieve tool " << m_JetBTaggerTool);
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG("#BTAG# Retrieved tool " << m_JetBTaggerTool);
  }
 
  return StatusCode::SUCCESS;
}


StatusCode StandAloneJetBTaggerAlg::execute() {
  // Check if there are Jet collection to tag
  if (evtStore()->contains<xAOD::JetContainer > ( m_JetCollectionName )) {
      std::string BTaggingCollectionName = std::string("BTagging_") + m_JetCollectionName;
      BTaggingCollectionName.erase(BTaggingCollectionName.length()-4);
      BTaggingCollectionName += m_suffix;
      //no PV0 in BTagging Collection name
      std::size_t start_position_PV0 = BTaggingCollectionName.find("PV0");
      if (start_position_PV0!=std::string::npos) {
 	BTaggingCollectionName.erase(start_position_PV0, 3);	
      }
      //Check if Jet collection already not tagged
      if (!evtStore()->contains<xAOD::BTaggingContainer>(BTaggingCollectionName)) {
        if ((m_JetCollectionName == "AntiKt4EMPFlowJets") && !m_dupPFlow) {
          ATH_MSG_DEBUG("#BTAG# Deep copy of Jet container:" << m_JetCollectionName );
	  if (evtStore()->contains<xAOD::JetAuxContainer>(m_JetCollectionName+"Aux.")) {
	    if (overwrite<xAOD::JetContainer, xAOD::JetAuxContainer>(m_JetCollectionName).isFailure()) {
	      ATH_MSG_FATAL( "Couldn't call overwrite with concrete auxiliary store" );
	      return StatusCode::FAILURE;
	    }
	  } else if (evtStore()->template contains< xAOD::AuxContainerBase >(m_JetCollectionName+"Aux.")) {
	    if (overwrite<xAOD::JetContainer, xAOD::AuxContainerBase>(m_JetCollectionName).isFailure()) {
	      ATH_MSG_FATAL( "Couldn't call overwrite with generic auxiliary store" );
	      return StatusCode::FAILURE;
	    }
	  }
	  xAOD::JetContainer* jets(0);
	  CHECK( evtStore()->retrieve(jets, m_JetCollectionName) );
          int ret = m_JetBTaggerTool->modify(*jets);
          if (!ret) {
            ATH_MSG_DEBUG("#BTAG# Failed to call JetBTaggerTool");
          }
        }
        // Run the same (public) tools on AntiKt4EMPFlowJets to tag it twice (2 different tunes)
        else {
          const xAOD::JetContainer* constjets(0);
          CHECK( evtStore()->retrieve( constjets, m_JetCollectionName ) );
          //shallow copy with Oct 2018 taggers
          std::string suffix = "_BTagging201810";
          ATH_MSG_DEBUG("#BTAG# Shallow copy of Jet container:" << m_JetCollectionName << " and registration with new name " << m_JetCollectionName + suffix);
          std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > pflow_shallowCopy = xAOD::shallowCopyContainer( *constjets );
          //Record in StoreGate the copy of AntiKt4EMPFlowJets with renaming
          CHECK( evtStore()->record( pflow_shallowCopy.first, m_JetCollectionName + suffix) );
          CHECK( evtStore()->record( pflow_shallowCopy.second, m_JetCollectionName + suffix+"Aux.") );
          //tag it, the other one is already tagged
          int ret = m_JetBTaggerTool->modify(*pflow_shallowCopy.first);

          //shallow copy with Mar 2019 taggers
          suffix = "_BTagging201903";
          ATH_MSG_DEBUG("#BTAG# Shallow copy of Jet container:" << m_JetCollectionName << " and registration with new name " << m_JetCollectionName + suffix);
          pflow_shallowCopy = xAOD::shallowCopyContainer( *constjets );
          //Record in StoreGate the copy of AntiKt4EMPFlowJets with renaming
          CHECK( evtStore()->record( pflow_shallowCopy.first, m_JetCollectionName + suffix) );
          CHECK( evtStore()->record( pflow_shallowCopy.second, m_JetCollectionName + suffix+"Aux.") );
          //tag it, the other one is already tagged
          ret = m_JetBTaggerTool->modify(*pflow_shallowCopy.first);
          if (!ret) {
            ATH_MSG_DEBUG("#BTAG# Failed to call JetBTaggerTool");
          }
        }        
      }
      else { //Shallow copy for re-tagging already tagged jet
        const xAOD::JetContainer * jetsAOD = 0;
        if ((m_dupPFlow) && evtStore()->contains<xAOD::BTaggingContainer>(BTaggingCollectionName+"_201810")) {
          ATH_MSG_DEBUG("#BTAG# 2019 Retraining campaign: Time stamped Jet b-tagged in AODFix. Nothing to do");
          return 1;
        }

        CHECK( evtStore()->retrieve(jetsAOD, m_JetCollectionName));
        ATH_MSG_DEBUG("#BTAG# Shallow copy of Jet container:" << m_JetCollectionName << " with " << jetsAOD->size() << " jets");
        auto rec = xAOD::shallowCopyContainer (*jetsAOD);
        if (m_dupPFlow) {
          std::string suffix = "_BTagging201810";
          ATH_MSG_DEBUG("#BTAG# 2019 Retraining campaign for " << m_JetCollectionName + suffix);
          CHECK( evtStore()->record( rec.first, m_JetCollectionName + suffix) );
          CHECK( evtStore()->record( rec.second, m_JetCollectionName + suffix + "Aux.") );
        }

        int ret = m_JetBTaggerTool->modify(*rec.first);

        if (!m_dupPFlow) {
          delete rec.first;
          delete rec.second;
        }
        if (!ret) {
          ATH_MSG_DEBUG("#BTAG# Failed to call JetBTaggerTool");
        }
      }
  }
  else {
    ATH_MSG_VERBOSE("#BTAG# No Jet container " << m_JetCollectionName << " in store, no re-tagging");
  }
  
  return 1;

}

template< class CONTAINER, class AUXSTORE >
StatusCode
StandAloneJetBTaggerAlg::overwrite(const std::string& key) const {
 
  // Retrieve the const container:
  const CONTAINER* c = 0;
  ATH_CHECK( evtStore()->retrieve(c, key) );
 
  // Create the new container:
  CONTAINER* copy = new CONTAINER();
  AUXSTORE* copyAux = new AUXSTORE();
  copy->setStore(copyAux);

  // Create the new objects, doing a deep copy if requested
  for (auto oldObj : *c ) {
    auto newObj = new typename CONTAINER::base_value_type();
    copy->push_back(newObj);
    *newObj = *oldObj;
  }
 
  // Do the overwrite:
  ATH_CHECK( evtStore()->overwrite( copy, key, true, false ) );
  ATH_CHECK( evtStore()->overwrite( copyAux, key + "Aux.", true, false ) );
 
  // Return gracefully:
  return StatusCode::SUCCESS;
}

} //// namespace analysis
