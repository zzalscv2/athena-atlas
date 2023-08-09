/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "TrigJetMonitoring/L1JetCopyAlgorithm.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCore/AuxContainerBase.h"

//**********************************************************************

template<typename T>
L1JetCopyAlgorithm<T>::L1JetCopyAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm(name,pSvcLocator){
  declareProperty("JetInContainerName"  ,m_jetInContainerKey="");
  declareProperty("JetOutContainerName"  ,m_jetOutContainerKey="");
}

template<typename T>
StatusCode L1JetCopyAlgorithm<T>::initialize() {
  
  ATH_MSG_INFO(" Initializing " << name());
  ATH_CHECK( m_jetInContainerKey.initialize() );
  ATH_CHECK( m_jetOutContainerKey.initialize() );

  return StatusCode::SUCCESS;
}

template<typename T>
StatusCode L1JetCopyAlgorithm<T>::finalize() {
  ATH_MSG_INFO ("Finalizing " << name());
  return StatusCode::SUCCESS;
}

//**********************************************************************

template<typename T>
StatusCode L1JetCopyAlgorithm<T>::execute(const EventContext& ctx) const {
  SG::ReadHandle<JetContainer> inputJetsHandle(m_jetInContainerKey, ctx); // read handle
  if (!inputJetsHandle.isValid() ) {
    ATH_MSG_ERROR("evtStore() does not contain jet Collection with name "<< m_jetInContainerKey);
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Shallow-copying "<<m_jetInContainerKey);
  SG::WriteHandle<JetContainer> jetsOut(m_jetOutContainerKey,ctx);

  auto shallowcopy = xAOD::shallowCopyContainer(*inputJetsHandle);
  std::unique_ptr<JetContainer> copiedjets(shallowcopy.first);
  std::unique_ptr<xAOD::ShallowAuxContainer> shallowaux(shallowcopy.second);

  if(copiedjets.get() == nullptr || shallowaux.get() == nullptr) {
      ATH_MSG_ERROR("Failed to make shallow copy of "<<m_jetInContainerKey<<".");
      return StatusCode::FAILURE;
  }
  ATH_CHECK( jetsOut.record(std::move(copiedjets), std::move(shallowaux)) );

 return StatusCode::SUCCESS;
}

template class L1JetCopyAlgorithm<JTM_JetRoIContainer>;
template class L1JetCopyAlgorithm<JTM_gFexJetRoIContainer>;
template class L1JetCopyAlgorithm<JTM_jFexLRJetRoIContainer>;
template class L1JetCopyAlgorithm<JTM_jFexSRJetRoIContainer>;
