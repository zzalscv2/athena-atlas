///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METSoftAssociator.cxx
// Implementation file for class METSoftAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
///////////////////////////////////////////////////////////////////

// METReconstruction includes
#include "METReconstruction/METSoftAssociator.h"
#include "xAODCaloEvent/CaloVertexedClusterBase.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

namespace met {

  using namespace xAOD;
  static const SG::AuxElement::Decorator<std::vector<ElementLink<IParticleContainer> > > dec_softConst("softConstituents");

  // Constructors
  ////////////////
  METSoftAssociator::METSoftAssociator(const std::string& name) :
    AsgTool(name),
    METAssociator(name),
    m_lcmodclus_key("LCOriginTopoClusters"),
    m_emmodclus_key("EMOriginTopoClusters")
  {
    declareProperty("DecorateSoftConst", m_decorateSoftTermConst=false);
    declareProperty("LCModClusterKey",   m_lcmodclus_key);
    declareProperty("EMModClusterKey",   m_emmodclus_key);
  }

  // Destructor
  ///////////////
  METSoftAssociator::~METSoftAssociator()
  = default;

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METSoftAssociator::initialize()
  {
    ATH_CHECK( METAssociator::initialize() );
    ATH_MSG_VERBOSE ("Initializing " << name() << "...");
    ATH_CHECK( m_lcmodclus_key.initialize());
    ATH_CHECK( m_emmodclus_key.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode METSoftAssociator::finalize()
  {
    ATH_MSG_VERBOSE ("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
  }

  // executeTool
  ////////////////
  StatusCode METSoftAssociator::executeTool(xAOD::MissingETContainer* metCont, xAOD::MissingETAssociationMap* metMap) const
  {

    // Add MET terms to the container
    // Always do this in order that the terms exist even if the method fails
    MissingET* metCoreCl = new MissingET(0.,0.,0.,"SoftClusCore",MissingETBase::Source::softEvent() | MissingETBase::Source::clusterLC());
    metCont->push_back(metCoreCl);
    MissingET* metCoreTrk = new MissingET(0.,0.,0.,"PVSoftTrkCore",MissingETBase::Source::softEvent() | MissingETBase::Source::track());
    metCont->push_back(metCoreTrk);

    ATH_MSG_VERBOSE ("In execute: " << name() << "...");
    met::METAssociator::ConstitHolder constits;
    if (retrieveConstituents(constits).isFailure()) {
      ATH_MSG_WARNING("Unable to retrieve constituent containers");
      return StatusCode::FAILURE;
    }

    if(m_pflow){
      if(!m_fecollKey.key().empty()){
        // PFOs have been provided as FlowElements
        const IParticleContainer* uniquePFOs = metMap->getUniqueSignals(constits.feCont,MissingETBase::UsageHandler::Policy::ParticleFlow);
        if(m_decorateSoftTermConst) {
          dec_softConst(*metCoreTrk) = std::vector<ElementLink<IParticleContainer> >();
          dec_softConst(*metCoreTrk).reserve(uniquePFOs->size());
          dec_softConst(*metCoreCl) = std::vector<ElementLink<IParticleContainer> >();
          dec_softConst(*metCoreCl).reserve(uniquePFOs->size());
        }
        for(const IParticle* sig : *uniquePFOs) {
          const xAOD::FlowElement *pfo = static_cast<const xAOD::FlowElement*>(sig);
          if (pfo->isCharged()) { // Charged PFOs
            // We set a small -ve pt for cPFOs that were rejected
            // by the ChargedHadronSubtractionTool
            const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");        
            if (PVMatchedAcc(*pfo) && ( !m_cleanChargedPFO || isGoodEoverP(static_cast<const xAOD::TrackParticle*>(pfo->chargedObject(0))) ) ) {
              // For the TST, we add the track pt, as this need not be
              // corrected for nearby energy in the calo
              *metCoreTrk += pfo->chargedObject(0);
              // For CST we add the PFO pt, which is weighted down
              // to account for energy in the calo that may not have
              // been subtracted
              *metCoreCl  += sig;
              if(m_decorateSoftTermConst) {
                dec_softConst(*metCoreTrk).emplace_back(*static_cast<const IParticleContainer*>(sig->container()),sig->index());
                dec_softConst(*metCoreCl).emplace_back(*static_cast<const IParticleContainer*>(sig->container()),sig->index());
              }
            }
          } else { // Neutral PFOs
            if (pfo->e()>FLT_MIN) {
              // This is a non-issue; just add the four-vector
              *metCoreCl += sig;
              if(m_decorateSoftTermConst) dec_softConst(*metCoreCl).emplace_back(*static_cast<const IParticleContainer*>(sig->container()),sig->index());
            }
          }
        }
        delete uniquePFOs;
      }
      else{
        const IParticleContainer* uniquePFOs = metMap->getUniqueSignals(constits.pfoCont,MissingETBase::UsageHandler::Policy::ParticleFlow);
        if(m_decorateSoftTermConst) {
          dec_softConst(*metCoreTrk) = std::vector<ElementLink<IParticleContainer> >();
          dec_softConst(*metCoreTrk).reserve(uniquePFOs->size());
          dec_softConst(*metCoreCl) = std::vector<ElementLink<IParticleContainer> >();
          dec_softConst(*metCoreCl).reserve(uniquePFOs->size());
        }
        for(const auto *const sig : *uniquePFOs) {
          const PFO *pfo = static_cast<const PFO*>(sig);
          if (pfo->isCharged()) { // Charged PFOs
            // We set a small -ve pt for cPFOs that were rejected
            // by the ChargedHadronSubtractionTool
            const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");        
            if (PVMatchedAcc(*pfo) && ( !m_cleanChargedPFO || isGoodEoverP(pfo->track(0)) ) ) {
              // For the TST, we add the track pt, as this need not be
              // corrected for nearby energy in the calo
              *metCoreTrk += pfo->track(0);
              // For CST we add the PFO pt, which is weighted down
              // to account for energy in the calo that may not have
              // been subtracted
              *metCoreCl  += sig;
              if(m_decorateSoftTermConst) {
                dec_softConst(*metCoreTrk).emplace_back(*static_cast<const IParticleContainer*>(sig->container()),sig->index());
                dec_softConst(*metCoreCl).emplace_back(*static_cast<const IParticleContainer*>(sig->container()),sig->index());
              }
            }
          } else { // Neutral PFOs
            if (pfo->e()>FLT_MIN) {
              // This is a non-issue; just add the four-vector
              *metCoreCl += sig;
              if(m_decorateSoftTermConst) dec_softConst(*metCoreCl).emplace_back(*static_cast<const IParticleContainer*>(sig->container()),sig->index());
            }
          }
        }
        delete uniquePFOs;
      }
    }
    else {
      MissingET* metCoreEMCl = new MissingET(0.,0.,0.,"SoftClusEMCore",MissingETBase::Source::softEvent() | MissingETBase::Source::clusterEM());
      metCont->push_back(metCoreEMCl);
      const IParticleContainer* uniqueClusters = metMap->getUniqueSignals(constits.tcCont,MissingETBase::UsageHandler::AllCalo);
      const IParticleContainer* uniqueTracks = constits.trkCont == nullptr ? new const IParticleContainer() : metMap->getUniqueSignals(constits.trkCont);
      if(m_decorateSoftTermConst) {
        dec_softConst(*metCoreTrk) = std::vector<ElementLink<IParticleContainer> >();
        dec_softConst(*metCoreTrk).reserve(uniqueTracks->size());
        dec_softConst(*metCoreCl) = std::vector<ElementLink<IParticleContainer> >();
        dec_softConst(*metCoreCl).reserve(uniqueClusters->size());
      }
      SG::ReadHandle<xAOD::CaloClusterContainer> lctc(m_lcmodclus_key);
      SG::ReadHandle<xAOD::CaloClusterContainer> emtc(m_emmodclus_key);

      for(const auto *const cl : *uniqueClusters) {
        if (cl->e()>FLT_MIN) {
          if(m_useModifiedClus) {
            if(lctc.isValid() && emtc.isValid()) {
              size_t cl_idx(cl->index());
              // clusters at LC scale
              *metCoreCl += (*lctc)[cl_idx];
              if(m_decorateSoftTermConst) dec_softConst(*metCoreCl).emplace_back(*static_cast<const IParticleContainer*>(lctc.cptr()),cl->index());
              // clusters at EM scale
              *metCoreEMCl += (*emtc)[cl_idx];
            } else {
              ATH_MSG_WARNING("Invalid LC/EM modified cluster collections -- cannot add cluster to soft term!");
            }
          } else {
            // clusters at LC scale
            if (cl->type()==xAOD::Type::CaloCluster) {
        CaloVertexedClusterBase stateClLC(*(static_cast<const CaloCluster*>(cl)),xAOD::CaloCluster::CALIBRATED);
              *metCoreCl += (&stateClLC);
            } else *metCoreCl += cl;
            if(m_decorateSoftTermConst) dec_softConst(*metCoreCl).emplace_back(*static_cast<const IParticleContainer*>(cl->container()),cl->index());
            // clusters at EM scale
            if (cl->type()==xAOD::Type::CaloCluster) {
        CaloVertexedClusterBase stateClEM( *(static_cast<const CaloCluster*>(cl)),xAOD::CaloCluster::UNCALIBRATED);
        *metCoreEMCl += (&stateClEM);
            } else *metCoreEMCl += cl;
          }
        }
      }

      if(constits.pv) {
        for(const auto *const trk : *uniqueTracks) {
          ATH_MSG_VERBOSE("Test core track with pt " << trk->pt());
          if(acceptTrack(static_cast<const TrackParticle*>(trk),constits.pv) && isGoodEoverP(static_cast<const TrackParticle*>(trk))) {
            ATH_MSG_VERBOSE("Add core track with pt " << trk->pt());
            *metCoreTrk += trk;
            if(m_decorateSoftTermConst) dec_softConst(*metCoreTrk).emplace_back(*static_cast<const IParticleContainer*>(trk->container()),trk->index());

          }
        }
      }
      delete uniqueClusters;
      delete uniqueTracks;
    }
    return StatusCode::SUCCESS;
  }
}
