/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   TrackParticleMerger
//   Author: Bingxuan Liu, bingxuan.liu@cern.ch
//   This is a modified version of exisitng TrackCollectionMerger
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Track Particle merger to be used downstream, mainly for 
//          combining LRT and standard tracks 
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkInDet/TrackParticleMerger.h"

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

namespace DerivationFramework {

  TrackParticleMerger::TrackParticleMerger(const std::string& t,
      const std::string& n,
      const IInterface* p) :
    AthAlgTool(t,n,p)
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    // The default goal of this merger is to create a collection combining standard and LRT tracks
    m_outtrackParticleLocation = "InDetWithLRTTrackParticles"    ;
    m_outtrackParticleAuxLocation = "InDetWithLRTTrackParticlesAux."    ;
    declareProperty("TrackParticleLocation",         m_trackParticleLocation);
    declareProperty("OutputTrackParticleAuxLocation",   m_outtrackParticleAuxLocation);
    declareProperty("OutputTrackParticleLocation",   m_outtrackParticleLocation);
  }

  ///////////////////////////////////////////////////////////////////
  // Initialisation
  ///////////////////////////////////////////////////////////////////

  StatusCode TrackParticleMerger::initialize()
  {

    ATH_MSG_DEBUG("Initializing TrackParticleMerger");
    ATH_CHECK( m_trackParticleLocation.initialize() );
    ATH_CHECK( m_outtrackParticleLocation.initialize() );
    ATH_CHECK( m_outtrackParticleAuxLocation.initialize() );
    return StatusCode::SUCCESS;
  }

  StatusCode TrackParticleMerger::addBranches() const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    auto outputCol = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(
      m_createViewCollection ? SG::VIEW_ELEMENTS : SG::OWN_ELEMENTS);
    ATH_MSG_DEBUG("Number of Track Particle collections " << m_trackParticleLocation.size());
    
    std::unique_ptr<xAOD::TrackParticleAuxContainer> outputAuxCol;

    if(!m_createViewCollection) {
      outputAuxCol = std::make_unique<xAOD::TrackParticleAuxContainer>();
      outputCol->setStore(outputAuxCol.get());
    }   

    // pre-loop to reserve enough space in the output collection
    std::vector<const xAOD::TrackParticleContainer*> trackParticleCollections;
    trackParticleCollections.reserve(m_trackParticleLocation.size());
    size_t ttNumber = 0;
    for (const auto & tcname : m_trackParticleLocation){
      ///Retrieve track particles from StoreGate
      SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleCol (tcname, ctx);
      if (!trackParticleCol.isValid()) {
        ATH_MSG_FATAL("Unable to retrieve xAOD::TrackParticleContainer, \"" << tcname << "\", cannot run the LRT track merger!");
        return StatusCode::FAILURE;
      }
      trackParticleCollections.push_back(trackParticleCol.cptr());
      ttNumber += trackParticleCol->size();
    }
    outputCol->reserve(ttNumber);
    // merging loop
    for(auto& tciter : trackParticleCollections){
      // merge them in
      mergeTrackParticle(tciter,outputCol.get());
    }
    auto h_write = SG::makeHandle(m_outtrackParticleLocation, ctx);
    ATH_CHECK(h_write.record(std::move(outputCol)));	     
    // only write out the aux container if the create view collection flag is false
    if(!m_createViewCollection) { 
      SG::WriteHandle<xAOD::TrackParticleAuxContainer> h_aux_write(m_outtrackParticleAuxLocation, ctx);
      ATH_CHECK(h_aux_write.record(std::move(outputAuxCol)));	     
    }
    ATH_MSG_DEBUG("Done !");  
    return StatusCode::SUCCESS;
  }
  
  ///////////////////////////////////////////////////////////////////
  // Merge track collections and remove duplicates
  ///////////////////////////////////////////////////////////////////

  void TrackParticleMerger::mergeTrackParticle(const xAOD::TrackParticleContainer* trackParticleCol,
                                               ConstDataVector<xAOD::TrackParticleContainer>* outputCol) const
  {
    // loop over tracks, accept them and add them into association tool
    if(!trackParticleCol || trackParticleCol->empty()) {return;}
    ATH_MSG_DEBUG("Size of track particle collection " << trackParticleCol->size());
    // loop over tracks
    for(const xAOD::TrackParticle* rf: *trackParticleCol){
      // add track into output
      const xAOD::TrackParticle* newTrackParticle = m_createViewCollection ? rf : new xAOD::TrackParticle(*rf);
      outputCol->push_back(newTrackParticle);
    }
  }
}

