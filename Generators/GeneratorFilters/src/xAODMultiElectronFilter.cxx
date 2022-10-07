/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMultiElectronFilter.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"




xAODMultiElectronFilter::xAODMultiElectronFilter(const std::string& name, ISvcLocator* pSvcLocator)
: GenFilter(name,pSvcLocator)
{
  declareProperty("Ptcut",m_ptmin = 10000.);
  declareProperty("Etacut",m_etaRange = 10.0);
  declareProperty("NElectrons",m_nElectrons = 2);
}
StatusCode xAODMultiElectronFilter::filterEvent() {  
  // Retrieve TruthElectron container
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  ATH_CHECK(evtStore()->retrieve(xTruthParticleContainer, "TruthElectrons"));
  int numElectrons = 0;
  
    for (auto *part: *xTruthParticleContainer) {
      if(  part->pt()>= m_ptmin && part->abseta() <= m_etaRange )
      {
        numElectrons++;
        if (numElectrons >= m_nElectrons)
        {
          setFilterPassed(true);
          return StatusCode::SUCCESS;
        }
      }
  }

  
  setFilterPassed(false);
  return StatusCode::SUCCESS;
}
