/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetRttPlots.cxx
 * @author shaun roe
 **/

#include "InDetRttPlots.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include <cmath> // std::isnan()
#include <limits>

InDetRttPlots::InDetRttPlots(InDetPlotBase* pParent, const std::string& sDir, const int iDetailLevel) : InDetPlotBase(pParent, sDir),
  m_trackParameters(this, "Tracks/Selected/Parameters"),
  m_matchedTrackParameters(this, "Tracks/Matched/Parameters"),
  m_mergedTrackParameters(this, "Tracks/Merged/Parameters"),
  m_fakeTrackParameters(this, "Tracks/Fake/Parameters"),
  m_nTracks(this, "Tracks/Tracks"),
  m_hitResidualPlot(this, "Tracks/Hits/Residuals"),
  m_hitEffPlot(this, "Tracks/Hits/Efficiency"),
  m_fakePlots(this, "Tracks/FakeRate"),
  m_missingTruthFakePlots(this, "Tracks/Unlinked/FakeRate"),
  m_resolutionPlotPrim(this, "Tracks/Matched/Resolutions/Primary"),
  m_resolutionPlotPrim_truthFromB(this, "Tracks/Matched/Resolutions/TruthFromB"),
  m_hitsRecoTracksPlots(this, "Tracks/Selected/HitsOnTracks"),
  m_effPlots(this, "Tracks/Efficiency"),
  m_verticesVsMuPlots(this, "Vertices/AllPrimaryVertices"),
  m_vertexPlots(this, "Vertices/AllPrimaryVertices"),
  m_hardScatterVertexPlots(this, "Vertices/HardScatteringVertex"),
  m_hardScatterVertexTruthMatchingPlots(this, "Vertices/HardScatteringVertex"),
  m_trtExtensionPlots(this, "Tracks/TRTExtension"),
  m_anTrackingPlots(this, "Tracks/ANT"),
  m_ntupleTruthToReco(this, "Ntuples", "TruthToReco"),
  m_resolutionPlotSecd(nullptr),
  m_doTrackInJetPlots(true),
  m_doTrackInBJetPlots(true),
  m_doTruthOriginPlots(true) // plots are created, but not filled without --doTruthOrigin flag
{
  this->m_iDetailLevel = iDetailLevel;
  m_trackParticleTruthProbKey = "truthMatchProbability";
  m_truthProbLowThreshold = 0.5;
  
  if(m_iDetailLevel >= 200){
    m_resolutionPlotSecd = std::make_unique<InDetPerfPlot_Resolution>(this, "Tracks/Matched/Resolutions/Secondary");
    m_hitsMatchedTracksPlots = std::make_unique<InDetPerfPlot_Hits>(this, "Tracks/Matched/HitsOnTracks");
    m_hitsFakeTracksPlots = std::make_unique<InDetPerfPlot_Hits>(this, "Tracks/Fakes/HitsOnTracks");
    m_hitsUnlinkedTracksPlots = std::make_unique<InDetPerfPlot_Hits>(this, "Tracks/Unlinked/HitsOnTracks");
    m_vertexTruthMatchingPlots = std::make_unique<InDetPerfPlot_VertexTruthMatching>(this, "Vertices/AllPrimaryVertices", m_iDetailLevel);

    //Split by track author
    m_effSiSPSeededFinderPlots = std::make_unique<InDetPerfPlot_Efficiency>(this, "TracksByAuthor/SiSPSeededFinder/Tracks/Efficiency");
    m_effInDetExtensionProcessorPlots = std::make_unique<InDetPerfPlot_Efficiency>(this, "TracksByAuthor/InDetExtensionProcessor/Tracks/Efficiency");
    m_effTRTSeededTrackFinderPlots = std::make_unique<InDetPerfPlot_Efficiency>(this, "TracksByAuthor/TRTSeededTrackFinder/Tracks/Efficiency");
    m_effTRTStandalonePlots = std::make_unique<InDetPerfPlot_Efficiency>(this, "TracksByAuthor/TRTStandalone/Tracks/Efficiency");
    m_effSiSpacePointsSeedMaker_LargeD0Plots = std::make_unique<InDetPerfPlot_Efficiency>(this, "TracksByAuthor/SiSpacePointsSeedMaker_LargeD0/Tracks/Efficiency");

    m_fakeSiSPSeededFinderPlots = std::make_unique<InDetPerfPlot_FakeRate>(this, "TracksByAuthor/SiSPSeededFinder/Tracks/FakeRate");
    m_fakeInDetExtensionProcessorPlots = std::make_unique<InDetPerfPlot_FakeRate>(this, "TracksByAuthor/InDetExtensionProcessor/Tracks/FakeRate");
    m_fakeTRTSeededTrackFinderPlots = std::make_unique<InDetPerfPlot_FakeRate>(this, "TracksByAuthor/TRTSeededTrackFinder/Tracks/FakeRate");
    m_fakeTRTStandalonePlots = std::make_unique<InDetPerfPlot_FakeRate>(this, "TracksByAuthor/TRTStandalone/Tracks/FakeRate");
    m_fakeSiSpacePointsSeedMaker_LargeD0Plots = std::make_unique<InDetPerfPlot_FakeRate>(this, "TracksByAuthor/SiSpacePointsSeedMaker_LargeD0/Tracks/FakeRate");

    m_trkParaSiSPSeededFinderPlots = std::make_unique<InDetPerfPlot_TrackParameters>(this, "TracksByAuthor/SiSPSeededFinder/Tracks/Parameters");
    m_trkParaInDetExtensionProcessorPlots = std::make_unique<InDetPerfPlot_TrackParameters>(this, "TracksByAuthor/InDetExtensionProcessor/Tracks/Parameters");
    m_trkParaTRTSeededTrackFinderPlots = std::make_unique<InDetPerfPlot_TrackParameters>(this, "TracksByAuthor/TRTSeededTrackFinder/Tracks/Parameters");
    m_trkParaTRTStandalonePlots = std::make_unique<InDetPerfPlot_TrackParameters>(this, "TracksByAuthor/TRTStandalone/Tracks/Parameters");
    m_trkParaSiSpacePointsSeedMaker_LargeD0Plots = std::make_unique<InDetPerfPlot_TrackParameters>(this, "TracksByAuthor/SiSpacePointsSeedMaker_LargeD0/Tracks/Parameters");

    m_resSiSPSeededFinderPlots = std::make_unique<InDetPerfPlot_Resolution>(this, "TracksByAuthor/SiSPSeededFinder/Tracks/Resolution");
    m_resInDetExtensionProcessorPlots = std::make_unique<InDetPerfPlot_Resolution>(this, "TracksByAuthor/InDetExtensionProcessor/Tracks/Resolution");
    m_resTRTSeededTrackFinderPlots = std::make_unique<InDetPerfPlot_Resolution>(this, "TracksByAuthor/TRTSeededTrackFinder/Tracks/Resolution");
    m_resTRTStandalonePlots = std::make_unique<InDetPerfPlot_Resolution>(this, "TracksByAuthor/TRTStandalone/Tracks/Resolution");
    m_resSiSpacePointsSeedMaker_LargeD0Plots = std::make_unique<InDetPerfPlot_Resolution>(this, "TracksByAuthor/SiSpacePointsSeedMaker_LargeD0/Tracks/Resolution");

  }

  /// update detail level of all the child tools
  setDetailLevel(m_iDetailLevel);
}


void InDetRttPlots::SetFillJetPlots(bool fillJets, bool fillBJets){

  m_doTrackInJetPlots = fillJets;
  m_doTrackInBJetPlots = fillBJets;

  if(m_doTrackInJetPlots){
    m_trkInJetPlots = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInJets/Tracks");
    if (m_iDetailLevel >= 200){
      m_trkInJetPlots_matched = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInJets/Matched",false);
      m_trkInJetPlots_fake = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInJets/Fakes",false);
      m_trkInJetPlots_unlinked = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInJets/Unlinked",false);
    }
    if(m_doTrackInBJetPlots){
      m_trkInJetPlots_bjets = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInBJets/Tracks");
      if (m_iDetailLevel >= 200){
        m_trkInJetPlots_matched_bjets = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInBJets/Matched",false);
        m_trkInJetPlots_fake_bjets = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInBJets/Fakes",false);
        m_trkInJetPlots_unlinked_bjets = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInBJets/Unlinked",false);
      }
    }
    if(m_doTruthOriginPlots){
      m_trkInJetPlots_truthFromB = std::make_unique<InDetPerfPlot_TrkInJet>(this, "TracksInBJets/TruthFromB");
    }
  }

}


//
//Fill plots for matched particles
//

void
InDetRttPlots::fill(const xAOD::TrackParticle& particle, const xAOD::TruthParticle& truthParticle, bool isFromB, float mu, float weight) {
  // fill measurement bias, resolution, and pull plots

  // fill ITK resolutions (bias / resolutions)
  if (particle.isAvailable<float>(m_trackParticleTruthProbKey)) {
    const float prob = particle.auxdata<float>(m_trackParticleTruthProbKey);
    float barcode = truthParticle.barcode();
    if (barcode < 200000 && barcode != 0 && prob > 0.5) {
        m_resolutionPlotPrim.fill(particle, truthParticle, weight);
    } else if (barcode >= 200000 && prob > 0.7 && m_iDetailLevel >= 200) {
        m_resolutionPlotSecd->fill(particle, truthParticle, weight);
    }
    if ( m_doTruthOriginPlots and isFromB ) {
      m_resolutionPlotPrim_truthFromB.fill(particle, truthParticle, weight);
    }

    if(m_iDetailLevel >= 200 and (barcode < 200000 and barcode != 0 and prob > 0.5)){
      std::bitset<xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo>  patternInfo = particle.patternRecoInfo();
    
      bool isSiSpSeededFinder = patternInfo.test(0);
      bool isInDetExtensionProcessor = patternInfo.test(3);
      bool isTRTSeededTrackFinder = patternInfo.test(4);
      bool isTRTStandalone = patternInfo.test(20);
      bool isSiSpacePointsSeedMaker_LargeD0 = patternInfo.test(49);

      if(isSiSpSeededFinder and not isInDetExtensionProcessor) m_resSiSPSeededFinderPlots->fill(particle, truthParticle, weight);
      if(isInDetExtensionProcessor and not (isTRTSeededTrackFinder or isSiSpacePointsSeedMaker_LargeD0)) m_resInDetExtensionProcessorPlots->fill(particle, truthParticle, weight);
      if(isTRTSeededTrackFinder and not isTRTStandalone) m_resTRTSeededTrackFinderPlots->fill(particle, truthParticle, weight);
      if(isTRTStandalone) m_resTRTStandalonePlots->fill(particle, truthParticle, weight);
      if(isSiSpacePointsSeedMaker_LargeD0) m_resSiSpacePointsSeedMaker_LargeD0Plots->fill(particle, truthParticle, weight);

    }

    if (barcode < 200000 && barcode != 0 && prob > 0.5) m_trtExtensionPlots.fill(particle, truthParticle, weight);


  }
 
  if(m_iDetailLevel >= 200){
    float barcode = truthParticle.barcode();
    if (barcode < 200000 && barcode != 0) { 
      m_hitsMatchedTracksPlots->fill(particle, mu, weight);
    }
  }
}

//
//Fill basic track properties for reconstructed tracks 
//

void
InDetRttPlots::fill(const xAOD::TrackParticle& particle, float weight) {
  m_hitResidualPlot.fill(particle, weight);
  m_hitEffPlot.fill(particle, weight);
  // fill pt plots
  m_trackParameters.fill(particle, weight);
  m_anTrackingPlots.fill(particle, weight);

  if(m_iDetailLevel >= 200){
    std::bitset<xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo>  patternInfo = particle.patternRecoInfo();
    
    bool isSiSpSeededFinder = patternInfo.test(0);
    bool isInDetExtensionProcessor = patternInfo.test(3);
    bool isTRTSeededTrackFinder = patternInfo.test(4);
    bool isTRTStandalone = patternInfo.test(20);
    bool isSiSpacePointsSeedMaker_LargeD0 = patternInfo.test(49);

    if(isSiSpSeededFinder and not isInDetExtensionProcessor) m_trkParaSiSPSeededFinderPlots->fill(particle, weight);
    else if(isInDetExtensionProcessor and not (isTRTSeededTrackFinder or isSiSpacePointsSeedMaker_LargeD0)) m_trkParaInDetExtensionProcessorPlots->fill(particle, weight);
    else if(isTRTSeededTrackFinder and not isTRTStandalone) m_trkParaTRTSeededTrackFinderPlots->fill(particle, weight);
    else if(isTRTStandalone) m_trkParaTRTStandalonePlots->fill(particle, weight);
    else if(isSiSpacePointsSeedMaker_LargeD0) m_trkParaSiSpacePointsSeedMaker_LargeD0Plots->fill(particle, weight);

  }

  m_trtExtensionPlots.fill(particle, weight);
}

void
InDetRttPlots::fill(const xAOD::TrackParticle& particle, const float mu, const unsigned int nVtx, float weight) {

  m_trtExtensionPlots.fill(particle, mu, nVtx, weight);
  m_hitsRecoTracksPlots.fill(particle, mu, weight);

}

void
InDetRttPlots::fill(const unsigned int nTrkANT, const unsigned int nTrkSTD, const unsigned int nTrkBAT, const float mu, const unsigned int nVtx, const float weight) { 

  m_anTrackingPlots.fill(nTrkANT, nTrkSTD, nTrkBAT, mu, nVtx, weight);

}

void
InDetRttPlots::fill(const unsigned int ntracks, const unsigned int mu, const unsigned int nvertices, const float weight) {

  m_nTracks.fill(ntracks, mu, nvertices, weight);

  
}
//
//Fill plots for selected truth particle
//

void
InDetRttPlots::fill(const xAOD::TruthParticle& truthParticle, float weight) {
  // fill truth plots 
  m_trackParameters.fill(truthParticle, weight);
}

//
//Fill Efficiencies
//

void
InDetRttPlots::fillEfficiency(const xAOD::TruthParticle& truth, const xAOD::TrackParticle& track, const bool isGood, const float mu, const unsigned int nVtx, float weight) {
  m_effPlots.fill(truth, isGood, weight);

  m_anTrackingPlots.fillEfficiency(truth, track, isGood, mu, nVtx, weight);
  if(m_iDetailLevel >= 200){
    if(isGood){
      std::bitset<xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo>  patternInfo = track.patternRecoInfo();
    
      bool isSiSpSeededFinder = patternInfo.test(0);
      bool isInDetExtensionProcessor = patternInfo.test(3);
      bool isTRTSeededTrackFinder = patternInfo.test(4);
      bool isTRTStandalone = patternInfo.test(20);
      bool isSiSpacePointsSeedMaker_LargeD0 = patternInfo.test(49);

      if(isSiSpSeededFinder and not isInDetExtensionProcessor) m_effSiSPSeededFinderPlots->fill(truth, isGood, weight);
      if(isInDetExtensionProcessor and not (isTRTSeededTrackFinder or isSiSpacePointsSeedMaker_LargeD0)) m_effInDetExtensionProcessorPlots->fill(truth, isGood, weight);
      if(isTRTSeededTrackFinder and not isTRTStandalone) m_effTRTSeededTrackFinderPlots->fill(truth, isGood, weight);
      if(isTRTStandalone) m_effTRTStandalonePlots->fill(truth, isGood, weight);
      if(isSiSpacePointsSeedMaker_LargeD0) m_effSiSpacePointsSeedMaker_LargeD0Plots->fill(truth, isGood, weight);
    } else {
      m_effSiSPSeededFinderPlots->fill(truth, isGood, weight);
      m_effInDetExtensionProcessorPlots->fill(truth, isGood, weight);
      m_effTRTSeededTrackFinderPlots->fill(truth, isGood, weight);
      m_effTRTStandalonePlots->fill(truth, isGood, weight);
      m_effSiSpacePointsSeedMaker_LargeD0Plots->fill(truth, isGood, weight);

    }
    
  }

}

//
//Fill Fake Rates
//

void
InDetRttPlots::fillFakeRate(const xAOD::TrackParticle& track, const bool isFake, const bool isAssociatedTruth, const float mu, const unsigned int nVtx, float weight){

  m_missingTruthFakePlots.fill(track, !isAssociatedTruth, weight);
  m_anTrackingPlots.fillUnlinked(track, !isAssociatedTruth, mu, nVtx, weight);
  if(m_iDetailLevel >= 200){
    if (!isAssociatedTruth) m_hitsUnlinkedTracksPlots->fill(track, mu, weight);
    else m_hitsFakeTracksPlots->fill(track, mu, weight);
  }
  if(isAssociatedTruth) {
    m_fakePlots.fill(track, isFake, weight);
      m_anTrackingPlots.fillFakeRate(track, isFake, mu, nVtx, weight);

    if(m_iDetailLevel >= 200){
      std::bitset<xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo>  patternInfo = track.patternRecoInfo();
      
      bool isSiSpSeededFinder = patternInfo.test(0);
      bool isInDetExtensionProcessor = patternInfo.test(3);
      bool isTRTSeededTrackFinder = patternInfo.test(4);
      bool isTRTStandalone = patternInfo.test(20);
      bool isSiSpacePointsSeedMaker_LargeD0 = patternInfo.test(49);

      if(isSiSpSeededFinder and not isInDetExtensionProcessor) m_fakeSiSPSeededFinderPlots->fill(track, isFake, weight); //No extensions 
      if(isInDetExtensionProcessor and not (isTRTSeededTrackFinder or isSiSpacePointsSeedMaker_LargeD0)) m_fakeInDetExtensionProcessorPlots->fill(track, isFake, weight); //Extensions but not Back-tracking
      if(isTRTSeededTrackFinder and not isTRTStandalone) m_fakeTRTSeededTrackFinderPlots->fill(track, isFake, weight); //BackTracking
      if(isTRTStandalone) m_fakeTRTStandalonePlots->fill(track, isFake, weight); //TRT standalone
      if(isSiSpacePointsSeedMaker_LargeD0) m_fakeSiSpacePointsSeedMaker_LargeD0Plots->fill(track, isFake, weight); //ANT
    }
  }

}



//
//Fill Vertexing Plots
//
void
InDetRttPlots::fill(const xAOD::VertexContainer& vertexContainer, const std::vector<const xAOD::TruthVertex*>& truthHSVertices, const std::vector<const xAOD::TruthVertex*>& truthPUVertices, float weight) {
  // fill vertex container general properties
  // m_verticesVsMuPlots.fill(vertexContainer); //if ever needed
  // fill vertex-specific properties, for all vertices and for hard-scattering vertex

  for (const auto& vtx : vertexContainer.stdcont()) {
    if (vtx->vertexType() == xAOD::VxType::NoVtx) {
      ATH_MSG_DEBUG("IN InDetRttPlots::fill, found xAOD::VxType::NoVtx");
      continue; // skip dummy vertex
    }
    m_vertexPlots.fill(*vtx, weight);
    ATH_MSG_DEBUG("IN InDetRttPlots::fill, filling for all vertices");
    if (vtx->vertexType() == xAOD::VxType::PriVtx) {
      m_hardScatterVertexPlots.fill(*vtx, weight);
      if(truthHSVertices.size()>0)m_hardScatterVertexTruthMatchingPlots.fill(*vtx,truthHSVertices[0],weight);
      else m_hardScatterVertexTruthMatchingPlots.fill(*vtx,nullptr,weight); 
      ATH_MSG_DEBUG("IN InDetRttPlots::fill, filling for all HS vertex");
    }
  }
  if(m_iDetailLevel >= 200){
    m_vertexTruthMatchingPlots->fill(vertexContainer, truthHSVertices, truthPUVertices, weight);
  }
}


void
InDetRttPlots::fill(const xAOD::VertexContainer& vertexContainer, unsigned int nPU, float weight) {
  m_verticesVsMuPlots.fill(vertexContainer, nPU, weight);
}

//
//Fill Counters
//
void
InDetRttPlots::fillCounter(const unsigned int freq, const InDetPerfPlot_nTracks::CounterCategory counter, float weight) {
  m_nTracks.fill(freq, counter, weight);
}

//Track in Jet Plots
void
InDetRttPlots::fill(const xAOD::TrackParticle& track, const xAOD::Jet& jet, bool isBjet, bool isFake, bool isUnlinked, bool truthIsFromB, float weight){
  m_trkInJetPlots->fill(track, jet,weight);
  if (m_iDetailLevel >= 200){
    if (isFake){
      m_trkInJetPlots_fake->fill(track,jet,weight); 
    }
    else if (isUnlinked){
      m_trkInJetPlots_unlinked->fill(track,jet,weight); 
    }
    else {
      m_trkInJetPlots_matched->fill(track,jet,weight); 
    }
  }
  if(isBjet && m_doTrackInBJetPlots){
    m_trkInJetPlots_bjets->fill(track, jet,weight);
    if ( truthIsFromB ) { // truth from B decay
      m_trkInJetPlots_truthFromB->fill(track, jet,weight);
    }
     
    if (m_iDetailLevel >= 200){
      if (isFake){
        m_trkInJetPlots_fake_bjets->fill(track,jet,weight); 
      }
      else if (isUnlinked){
        m_trkInJetPlots_unlinked_bjets->fill(track,jet,weight); 
      }
      else {
        m_trkInJetPlots_matched_bjets->fill(track,jet,weight); 
      }
    }
  }
}

void
InDetRttPlots::fillEfficiency(const xAOD::TruthParticle& truth, const xAOD::Jet& jet, bool isEfficient, bool isBjet, bool truthIsFromB, float weight) {
  m_trkInJetPlots->fillEfficiency(truth, jet, isEfficient, weight); 
  if(isBjet && m_doTrackInBJetPlots) m_trkInJetPlots_bjets->fillEfficiency(truth, jet, isEfficient, weight);
  
  if ( isBjet and m_doTrackInBJetPlots and truthIsFromB ) { // truth is from B
    m_trkInJetPlots_truthFromB->fillEfficiency(truth, jet, isEfficient, weight);
  }
}

void
InDetRttPlots::fillFakeRate(const xAOD::TrackParticle& track, const xAOD::Jet& jet, bool isFake, bool isBjet, bool truthIsFromB, float weight) {
  m_trkInJetPlots->fillFakeRate(track, jet, isFake, weight); 
  if(isBjet && m_doTrackInBJetPlots) m_trkInJetPlots_bjets->fillFakeRate(track, jet, isFake, weight); 

  if ( isBjet and m_doTrackInBJetPlots and truthIsFromB ) { // truth is from B
    m_trkInJetPlots_truthFromB->fillFakeRate(track, jet, isFake, weight);
  }
}

//IDPVM Ntuple
void
InDetRttPlots::fillNtuple(const xAOD::TrackParticle& track) {
  // Fill track only entries with dummy truth values
  m_ntupleTruthToReco.fillTrack(track);
  m_ntupleTruthToReco.fillTree();
}

void
InDetRttPlots::fillNtuple(const xAOD::TruthParticle& truth) {
  // Fill truth only entries with dummy track values
  m_ntupleTruthToReco.fillTruth(truth);
  m_ntupleTruthToReco.fillTree();
}

void 
InDetRttPlots::fillNtuple(const xAOD::TrackParticle& track, const xAOD::TruthParticle& truth, const int truthMatchRanking) {
  // Fill track and truth entries
  m_ntupleTruthToReco.fillTrack(track, truthMatchRanking);
  m_ntupleTruthToReco.fillTruth(truth);
  m_ntupleTruthToReco.fillTree();
}
