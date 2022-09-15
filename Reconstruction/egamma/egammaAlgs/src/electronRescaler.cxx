/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "electronRescaler.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/ElectronContainer.h"

electronRescaler::electronRescaler(const std::string& name,
                                     ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode
electronRescaler::initialize()
{
  // the data handle keys
  ATH_CHECK(m_electronOutputKey.initialize());
  ATH_CHECK(m_electronInputKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode
electronRescaler::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode
electronRescaler::execute(const EventContext& ctx) const {

  //const EgammaRecContainer* inputElRecs = nullptr;
  //const EgammaRecContainer* inputPhRecs = nullptr;
  //xAOD::ElectronContainer* electrons = nullptr;
  //xAOD::PhotonContainer* photons = nullptr;
  /*
   * From here on if a Read/Write handle
   * is retrieved the above will be !=
   * nullptr for electron or photons or both
   */
  SG::ReadHandle<xAOD::ElectronContainer> inputContainer(m_electronInputKey,ctx);
  SG::WriteHandle<xAOD::ElectronContainer> outputContainer(m_electronOutputKey,ctx);
  ATH_CHECK(outputContainer.record(std::make_unique<xAOD::ElectronContainer>(),
				    std::make_unique<xAOD::ElectronAuxContainer>()));

  xAOD::ElectronContainer* electrons = outputContainer.ptr();
  electrons->reserve(inputContainer->size());
  for (const xAOD::Electron* old_el : *inputContainer) {
    xAOD::Electron* electron = new xAOD::Electron();
    electrons->push_back(electron);
    *electron=*old_el;
    electron->setPt(old_el->pt()*m_scaleValue);
  }
  
  return StatusCode::SUCCESS;
 
}
