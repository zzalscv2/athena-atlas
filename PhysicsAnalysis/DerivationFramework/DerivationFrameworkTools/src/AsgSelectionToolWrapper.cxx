/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AsgSelectionToolWrapper.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: James Catmore (james.catmore@cern.ch)
//

#include "DerivationFrameworkTools/AsgSelectionToolWrapper.h"
#include "PATCore/AcceptData.h"
#include <StoreGate/ReadHandle.h>


namespace DerivationFramework {

  AsgSelectionToolWrapper::AsgSelectionToolWrapper(const std::string& t,
      const std::string& n,
      const IInterface* p) : 
    AthAlgTool(t,n,p)
   
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
   
  }

  StatusCode AsgSelectionToolWrapper::initialize() {
    if (m_sgName.value().empty()) {
      ATH_MSG_ERROR("No SG name provided for the output of invariant mass tool!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(m_tool.retrieve());
    ATH_CHECK(m_containerKey.initialize());
    m_decorKey = m_containerKey.key() + "." + m_sgName;
    ATH_CHECK(m_decorKey.initialize());
    return StatusCode::SUCCESS;
  }

  

  StatusCode AsgSelectionToolWrapper::addBranches() const
  {
    // retrieve container
    
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::IParticleContainer> particles{m_containerKey, ctx};
    if( ! particles.isValid() ) {
        ATH_MSG_ERROR ("Couldn't retrieve IParticles with key: " << m_containerKey.fullKey() );
        return StatusCode::FAILURE;
    }
    // Decorator
    const SG::AuxElement::Decorator< char > decorator(m_sgName);
    
    // Write mask for each element and record to SG for subsequent selection
    for ( const xAOD::IParticle* part : *particles) {
      auto theAccept = m_tool->accept(part);  // asg::AcceptData or TAccept
      if(m_cut.empty()){
        decorator(*part) = true && theAccept;    
      } else{
        decorator(*part) = true && theAccept.getCutResult(m_cut);
      }
    }
        
    return StatusCode::SUCCESS;
  }
}
