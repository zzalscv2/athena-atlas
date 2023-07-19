/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GeneratorObjects/xAODTruthParticleLink.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/HepMCHelpers.h"

#include "GeneratorFilters/xAODTruthParticleSlimmerMET.h"

#include "MCTruthClassifier/IMCTruthClassifier.h"

xAODTruthParticleSlimmerMET::xAODTruthParticleSlimmerMET(const std::string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
    , m_classif("MCTruthClassifier/DFCommonTruthClassifier")
{
    declareProperty("xAODTruthParticleContainerName", m_xaodTruthParticleContainerName = "TruthParticles");
    declareProperty("xAODTruthParticleContainerNameMET", m_xaodTruthParticleContainerNameMET = "TruthMET");
    declareProperty("xAODTruthEventContainerName", m_xaodTruthEventContainerName = "TruthEvents");
}

StatusCode xAODTruthParticleSlimmerMET::initialize()
{
    ATH_MSG_INFO("xAOD input TruthParticleContainer name = " << m_xaodTruthParticleContainerName);
    ATH_MSG_INFO("xAOD output TruthParticleContainerMET name = " << m_xaodTruthParticleContainerNameMET);

    ATH_CHECK(m_classif.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerMET::execute()
{
    // If the containers already exists then assume that nothing needs to be done
    if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthParticleContainerNameMET))
    {
        ATH_MSG_WARNING("xAOD MET Truth Particles are already available in the event");
        return StatusCode::SUCCESS;
    }

    // Create new output container
    xAOD::TruthParticleContainer *xTruthParticleContainerMET = new xAOD::TruthParticleContainer();
    CHECK(evtStore()->record(xTruthParticleContainerMET, m_xaodTruthParticleContainerNameMET));
    xAOD::TruthParticleAuxContainer *xTruthParticleAuxContainerMET = new xAOD::TruthParticleAuxContainer();
    CHECK(evtStore()->record(xTruthParticleAuxContainerMET, m_xaodTruthParticleContainerNameMET + "Aux."));
    xTruthParticleContainerMET->setStore(xTruthParticleAuxContainerMET);
    ATH_MSG_INFO("Recorded TruthParticleContainerMET with key: " << m_xaodTruthParticleContainerNameMET);

    // Retrieve full TruthParticle container
    const xAOD::TruthParticleContainer *xTruthParticleContainer;
    if (evtStore()->retrieve(xTruthParticleContainer, m_xaodTruthParticleContainerName).isFailure())
    {
        ATH_MSG_ERROR("No TruthParticle collection with name " << m_xaodTruthParticleContainerName << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    // Retrieve full TruthEventContainer container
    const xAOD::TruthEventContainer *xTruthEventContainer=NULL;
    if (evtStore()->retrieve(xTruthEventContainer, m_xaodTruthEventContainerName).isFailure())
    {
        ATH_MSG_ERROR("No TruthEvent collection with name " << m_xaodTruthEventContainerName << " found in StoreGate!");
        return StatusCode::FAILURE;
    }

    // Set up decorators if needed
    const static SG::AuxElement::Decorator<bool> isPrompt("isPrompt");

    // Loop over full TruthParticle container
    xAOD::TruthEventContainer::const_iterator itr;
    for (itr = xTruthEventContainer->begin(); itr!=xTruthEventContainer->end(); ++itr) {

        unsigned int nPart = (*itr)->nTruthParticles();
        for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* theParticle =  (*itr)->truthParticle(iPart);

          // stable and non-interacting, implemented from DerivationFramework 
          //https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/python/MCTruthCommon.py#L183
          // which in turn use the implementation from Reconstruction
          //https://gitlab.cern.ch/atlas/athena/blob/21.0/Reconstruction/MET/METReconstruction/Root/METTruthTool.cxx#L143
          if (!theParticle->isGenStable()) continue;
          if (MC::isInteracting(theParticle->pdgId())) continue;


          xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();
          xTruthParticleContainerMET->push_back( xTruthParticle );

          // Fill with numerical content
          xTruthParticle->setPdgId(theParticle->pdgId());
          xTruthParticle->setBarcode(theParticle->barcode());
          xTruthParticle->setStatus(theParticle->status());
          xTruthParticle->setM(theParticle->m());
          xTruthParticle->setPx(theParticle->px());
          xTruthParticle->setPy(theParticle->py());
          xTruthParticle->setPz(theParticle->pz());
          xTruthParticle->setE(theParticle->e());

          //Decorate
          isPrompt(*xTruthParticle) = prompt(theParticle);
        }
    }

    return StatusCode::SUCCESS;
}

bool xAODTruthParticleSlimmerMET::prompt( const xAOD::TruthParticle* part ) const
{

    MCTruthPartClassifier::ParticleOrigin orig = m_classif->particleTruthClassifier( part ).second;
    ATH_MSG_DEBUG("Particle has origin " << orig);
      
    switch(orig) {
    case MCTruthPartClassifier::NonDefined:
    case MCTruthPartClassifier::PhotonConv:
    case MCTruthPartClassifier::DalitzDec:
    case MCTruthPartClassifier::ElMagProc:
    case MCTruthPartClassifier::Mu:
    case MCTruthPartClassifier::LightMeson:
    case MCTruthPartClassifier::StrangeMeson:
    case MCTruthPartClassifier::CharmedMeson:
    case MCTruthPartClassifier::BottomMeson:
    case MCTruthPartClassifier::CCbarMeson:
    case MCTruthPartClassifier::JPsi:
    case MCTruthPartClassifier::BBbarMeson:
    case MCTruthPartClassifier::LightBaryon:
    case MCTruthPartClassifier::StrangeBaryon:
    case MCTruthPartClassifier::CharmedBaryon:
    case MCTruthPartClassifier::BottomBaryon:
    case MCTruthPartClassifier::PionDecay:
    case MCTruthPartClassifier::KaonDecay: 
      return false;
    default:
      break;
    }
    
    return true;
  }

