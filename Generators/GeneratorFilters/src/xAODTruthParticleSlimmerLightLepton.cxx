/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "TruthUtils/HepMCHelpers.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

#include "GeneratorFilters/xAODTruthParticleSlimmerLightLepton.h"

xAODTruthParticleSlimmerLightLepton::xAODTruthParticleSlimmerLightLepton(const std::string &name, ISvcLocator *svcLoc)
    : AthReentrantAlgorithm(name, svcLoc)
{
}

StatusCode xAODTruthParticleSlimmerLightLepton::initialize()
{   
    ATH_CHECK(m_xaodTruthEventContainerNameReadHandleKey.initialize());
    ATH_CHECK(m_xaodTruthParticleContainerNameLightLeptonKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerLightLepton::execute(const EventContext& context) const
{
    // If the containers already exists then assume that nothing needs to be done
    if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthParticleContainerNameLightLeptonKey.key()))
    {
        ATH_MSG_WARNING("xAOD LightLeptons Truth Particles are already available in the event");
        return StatusCode::SUCCESS;
    }

    // Create new output container
    SG::WriteHandle<xAOD::TruthParticleContainer> xTruthParticleContainerLightLepton(m_xaodTruthParticleContainerNameLightLeptonKey, context);
	ATH_CHECK(xTruthParticleContainerLightLepton.record(std::make_unique<xAOD::TruthParticleContainer>(), std::make_unique<xAOD::TruthParticleAuxContainer>()));
        
    SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainerReadHandle(m_xaodTruthEventContainerNameReadHandleKey, context);
    if (!xTruthEventContainerReadHandle.isValid()) {
	  ATH_MSG_ERROR("Could not retrieve xAOD::TruthEventContainer with key:" << 
			m_xaodTruthEventContainerNameReadHandleKey.key());
	  return StatusCode::FAILURE;
	}

    xAOD::TruthEventContainer::const_iterator itr;
    for (itr = xTruthEventContainerReadHandle->begin(); itr!=xTruthEventContainerReadHandle->end(); ++itr) {

        unsigned int nPart = (*itr)->nTruthParticles();
        for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* particle =  (*itr)->truthParticle(iPart);

            //Save stable Electrons & Muons
            if (MC::isStable(particle) && (MC::isElectron(particle) || MC::isMuon(particle)) )
            {
                xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();
                xTruthParticleContainerLightLepton->push_back( xTruthParticle );

                // Fill with numerical content
                *xTruthParticle=*particle;
            }
        }

    }

    return StatusCode::SUCCESS;
}
