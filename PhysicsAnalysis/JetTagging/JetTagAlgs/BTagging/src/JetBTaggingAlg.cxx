/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BTagging/JetBTaggingAlg.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include <string>
#include <optional>

namespace Analysis {


  JetBTaggingAlg::JetBTaggingAlg(const std::string& n, ISvcLocator *p) : 
    AthReentrantAlgorithm(n,p),
    m_JetName(""),
    m_bTagTool("Analysis::BTagTool",this),
    m_bTagSecVtxTool("Analysis::BTagSecVertexing",this)
  {
    declareProperty("JetCalibrationName", m_JetName);
    declareProperty("BTagTool", m_bTagTool);
    declareProperty("BTagSecVertexing", m_bTagSecVtxTool);
  }

  StatusCode JetBTaggingAlg::initialize() {

    m_IncomingTracks = m_JetCollectionName.key() + "." + m_IncomingTracks.key();
    m_OutgoingTracks = m_BTaggingCollectionName.key() + "." + m_OutgoingTracks.key();

    // This will check that the properties were initialized properly
    // by job configuration.
    ATH_CHECK( m_JetCollectionName.initialize() );
    ATH_CHECK( m_IncomingTracks.initialize() );
    ATH_CHECK( m_OutgoingTracks.initialize() );
    ATH_CHECK( m_BTaggingCollectionName.initialize() );
    ATH_CHECK( m_jetBTaggingLinkName.initialize() );
    ATH_CHECK( m_bTagJetDecorLinkName.initialize() );

    // this is a terrible, awful hack
    // but right now there aren't any muons for b-tagging in the trigger
    // so if an empty muon container is passed, DON'T DECLARE A DEPENDENCY
    // we'll make an empty container on the b-tagging object later...
    m_DoMuons = !m_IncomingMuons.key().empty();

    if (m_DoMuons) {
      ATH_MSG_DEBUG("#BTAG# muons requested for: " << m_JetCollectionName.key());
      m_IncomingMuons = m_JetCollectionName.key() + "." + m_IncomingMuons.key();
      ATH_CHECK( m_IncomingMuons.initialize(true) );
    } else {
      ATH_MSG_DEBUG("#BTAG# no muons requested for: " << m_JetCollectionName.key());
      ATH_CHECK( m_IncomingMuons.initialize(false) );
    }

    m_OutgoingMuons = m_BTaggingCollectionName.key() + "." + m_OutgoingMuons.key();
    ATH_CHECK( m_OutgoingMuons.initialize() );

    ATH_MSG_DEBUG("#BTAG# Jet container name: " << m_JetCollectionName.key());
    ATH_MSG_DEBUG("#BTAG# BTagging container name: " << m_BTaggingCollectionName.key());
    ATH_MSG_DEBUG("#BTAG# EL from Jet to BTagging: " << m_jetBTaggingLinkName.key());
    ATH_MSG_DEBUG("#BTAG# EL from BTagging to Jet: " << m_bTagJetDecorLinkName.key());
   
    /// retrieve the main BTagTool
    if ( m_bTagTool.retrieve().isFailure() ) {
      ATH_MSG_FATAL("#BTAG# Failed to retrieve tool " << m_bTagTool);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG("#BTAG# Retrieved tool " << m_bTagTool);
    }

    /// retrieve the bTagSecVtxTool
    if ( m_bTagSecVtxTool.retrieve().isFailure() ) {
      ATH_MSG_FATAL("#BTAGVTX# Failed to retrieve tool " << m_bTagSecVtxTool);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG("#BTAGVTX# Retrieved tool " << m_bTagSecVtxTool);
    }

    /// handle to the magnetic field cache
    ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );

    return StatusCode::SUCCESS;
  }


  StatusCode JetBTaggingAlg::execute(const EventContext& ctx) const {
    //retrieve the Jet container
    SG::ReadHandle<xAOD::JetContainer> h_JetCollectionName (m_JetCollectionName, ctx);
    if (!h_JetCollectionName.isValid()) {
      ATH_MSG_ERROR( " cannot retrieve jet container with key " << m_JetCollectionName.key()  );
      return StatusCode::FAILURE;
    }

    if (h_JetCollectionName->empty()) {
      ATH_MSG_DEBUG("#BTAG# Empty JetContainer !!");
    }
    else {
      ATH_MSG_DEBUG("#BTAG#  Nb jets in JetContainer: "<< h_JetCollectionName->size());
    }

    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink< xAOD::IParticleContainer> > >
      h_IncomingTracks(m_IncomingTracks, ctx);

    std::optional<SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink< xAOD::IParticleContainer> > > >
      h_IncomingMuons;
      
    if (m_DoMuons) {
      SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink< xAOD::IParticleContainer> > >
        tmp(m_IncomingMuons, ctx);
      h_IncomingMuons = tmp;
    }


    SG::WriteDecorHandle<xAOD::BTaggingContainer, std::vector<ElementLink< xAOD::TrackParticleContainer> > >
      h_OutgoingTracks(m_OutgoingTracks, ctx);

    SG::WriteDecorHandle<xAOD::BTaggingContainer, std::vector<ElementLink< xAOD::MuonContainer> > >
      h_OutgoingMuons(m_OutgoingMuons, ctx);


    //Decor Jet with element link to the BTagging
    SG::WriteDecorHandle<xAOD::JetContainer, ElementLink< xAOD::BTaggingContainer > > h_jetBTaggingLinkName(m_jetBTaggingLinkName, ctx);
    //Decor BTagging with element link to the Jet
    SG::WriteDecorHandle<xAOD::BTaggingContainer, ElementLink< xAOD::JetContainer > > h_bTagJetLinkName(m_bTagJetDecorLinkName, ctx);

    //Create a xAOD::BTaggingContainer in any case (must be done)
    std::string bTaggingContName = m_BTaggingCollectionName.key();
    ATH_MSG_DEBUG("#BTAG#  Container name: "<< bTaggingContName);

    /* Record the BTagging  output container */
    SG::WriteHandle<xAOD::BTaggingContainer> h_BTaggingCollectionName (m_BTaggingCollectionName, ctx);
    ATH_CHECK( h_BTaggingCollectionName.record(std::make_unique<xAOD::BTaggingContainer>(),
                    std::make_unique<xAOD::BTaggingAuxContainer>()) );

    MagField::AtlasFieldCache    fieldCache;
    // Get field cache object
    SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
    const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
   
    if (fieldCondObj == nullptr) {
      ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
      return StatusCode::FAILURE;
    }
    fieldCondObj->getInitializedCache (fieldCache);

    if (!fieldCache.solenoidOn()) {
      for (const auto *jet : *h_JetCollectionName) {
        ElementLink< xAOD::BTaggingContainer> linkBTagger;
        h_jetBTaggingLinkName(*jet) = linkBTagger;
      }
      return StatusCode::SUCCESS;
    } else { //Solenoid ON
      for (const auto *jet : *h_JetCollectionName.ptr()) {
        xAOD::BTagging *newBTagMT = new xAOD::BTagging();
        h_BTaggingCollectionName->push_back(newBTagMT);

        // Track association
        const std::vector<ElementLink<xAOD::IParticleContainer> >& trackLinks = h_IncomingTracks(*jet);

        std::vector<ElementLink<xAOD::TrackParticleContainer> > tmpTracks;

        tmpTracks.reserve(trackLinks.size());
        for (const ElementLink<xAOD::IParticleContainer>& elpart : trackLinks)
          tmpTracks.emplace_back(elpart.key(), elpart.index());

        h_OutgoingTracks(*newBTagMT) = tmpTracks;

        // Muon association
        // awful hack part deux
        // if a non-empty incoming muon key was requested
        // then we actually copy them over
        // otherwise just create an empty container
        std::vector<ElementLink<xAOD::MuonContainer> > tmpMuons;
        if (m_DoMuons) {
          const std::vector<ElementLink<xAOD::IParticleContainer> >& muonLinks = (*h_IncomingMuons)(*jet);

          for (const ElementLink<xAOD::IParticleContainer>& elpart : muonLinks)
            tmpMuons.emplace_back(elpart.key(), elpart.index());
        }

        h_OutgoingMuons(*newBTagMT) = tmpMuons;

      }

    }

    // Secondary vertex reconstruction.
    StatusCode SV = m_bTagSecVtxTool->BTagSecVertexing_exec(h_JetCollectionName.ptr(), h_BTaggingCollectionName.ptr());
    if (SV.isFailure()) {
      ATH_MSG_WARNING("#BTAG# Failed to reconstruct sec vtx");
    }


    //Tag the jets
    SV = m_bTagTool->tagJet( h_JetCollectionName.ptr(), h_BTaggingCollectionName.ptr(), m_JetName);
    if (SV.isFailure()) {
      ATH_MSG_WARNING("#BTAG# Failed in taggers call");
    }

    //Create the element link from the jet to the btagging and reverse link
    for (size_t jetIndex=0; jetIndex < h_JetCollectionName->size() ; ++jetIndex) {
      const xAOD::Jet * jetToTag = h_JetCollectionName->at(jetIndex);
      xAOD::BTagging * itBTag = h_BTaggingCollectionName->at(jetIndex);
      ElementLink< xAOD::BTaggingContainer> linkBTagger;
      linkBTagger.toContainedElement(*h_BTaggingCollectionName.ptr(), itBTag);
      h_jetBTaggingLinkName(*jetToTag) = linkBTagger;
      ElementLink< xAOD::JetContainer> linkJet;
      linkJet.toContainedElement(*h_JetCollectionName.ptr(), jetToTag);
      h_bTagJetLinkName(*itBTag) = linkJet;
    }

    return StatusCode::SUCCESS;

  }

} //// namespace analysis
