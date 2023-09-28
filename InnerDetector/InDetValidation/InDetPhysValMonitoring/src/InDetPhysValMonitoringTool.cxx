/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPhysValMonitoringTool.cxx
 * @author shaun roe
 **/

#include "GaudiKernel/SystemOfUnits.h"

#include "InDetPhysValMonitoring/InDetPhysValMonitoringTool.h"
#include "InDetPhysHitDecoratorAlg.h"
#include "InDetRttPlots.h"
#include "InDetPerfPlot_nTracks.h"
#include "AthTruthSelectionTool.h"
#include "CachedGetAssocTruth.h"

#include "safeDecorator.h"
// #include "TrackTruthLookup.h"
//
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackStateValidation.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthPileupEvent.h"
#include "xAODTruth/TruthPileupEventAuxContainer.h"

#include "TrkTrack/TrackCollection.h"
//
#include <algorithm>
#include <limits>
#include <cmath> // to get std::isnan(), std::abs etc.
// #include <functional> // to get std::plus
#include <utility>
#include <cstdlib> // to getenv
#include <vector>

#include "InDetTruthVertexValidation/InDetVertexTruthMatchUtils.h"

namespace { // utility functions used here

  // get truth/track matching probability
  float
  getMatchingProbability(const xAOD::TrackParticle& trackParticle) {
    float result(std::numeric_limits<float>::quiet_NaN());

    if (trackParticle.isAvailable<float>("truthMatchProbability")) {
      result = trackParticle.auxdata<float>("truthMatchProbability");
    }
    return result;
  }

  template <class T>
  inline float
  safelyGetEta(const T& pTrk, const float safePtThreshold = 0.1) {
    return (pTrk->pt() > safePtThreshold) ? (pTrk->eta()) : std::nan("");
  }

  // general utility function to check value is in range
  template <class T>
  inline bool
  inRange(const T& value, const T& minVal, const T& maxVal) {
    return not ((value < minVal)or(value > maxVal));
  }

  template<class T>
  inline bool
  inRange(const T& value, const T& absoluteMax) {
    return not (std::abs(value) > absoluteMax);
  }

  // Cuts on various objects
  bool
  passJetCuts(const xAOD::Jet& jet) {
    const float absEtaMax = 2.5;
    const float jetPtMin = 100.0;  // in GeV
    const float jetPtMax = 5000.0; // in GeV
    const float jetPt = jet.pt() / Gaudi::Units::GeV; // GeV
    const float jetEta = jet.eta();

    return inRange(jetPt, jetPtMin, jetPtMax) and inRange(jetEta, absEtaMax);
  }

  // general utility function to return bin index given a value and the upper endpoints of each bin
  template <class T>
  unsigned int
  binIndex(const T& val, const std::vector<T>& partitions) {// signature should allow other containers
    unsigned int i(0);
    bool nf = true;

    while (nf and i != partitions.size()) {
      nf = (val > partitions[i++]);
    }
    return nf ? i : i - 1;
  }

  bool
  acceptTruthVertex(const xAOD::TruthVertex* vtx) {
    const float x(vtx->x()), y(vtx->y()), z(vtx->z());
    const float vrtR2 = (x * x + y * y); // radial distance squared

    return inRange(z, 500.f) and not (vrtR2 > 1); // units?
  }

  const std::vector<float> ETA_PARTITIONS = {
    2.7, 3.5, std::numeric_limits<float>::infinity()
  };

}// namespace

///Parametrized constructor
InDetPhysValMonitoringTool::InDetPhysValMonitoringTool(const std::string& type, const std::string& name,
                                                       const IInterface* parent) :
  ManagedMonitorToolBase(type, name, parent){
}

InDetPhysValMonitoringTool::~InDetPhysValMonitoringTool() = default;

StatusCode
InDetPhysValMonitoringTool::initialize() {
  ATH_CHECK(ManagedMonitorToolBase::initialize());
  // Get the track selector tool only if m_useTrackSelection is true;
  // first check for consistency i.e. that there is a trackSelectionTool if you ask
  // for trackSelection
  ATH_CHECK(m_trackSelectionTool.retrieve(EnableTool {m_useTrackSelection} ));
  ATH_CHECK(m_truthSelectionTool.retrieve(EnableTool {not m_truthParticleName.key().empty()} ));
  ATH_CHECK(m_vtxValidTool.retrieve(EnableTool {m_useVertexTruthMatchTool}));
  ATH_CHECK(m_trackTruthOriginTool.retrieve( EnableTool {m_doTruthOriginPlots} ));
  ATH_CHECK(m_hardScatterSelectionTool.retrieve());
  ATH_CHECK(m_grlTool.retrieve());

  ATH_MSG_DEBUG("m_useVertexTruthMatchTool ====== " <<m_useVertexTruthMatchTool);
  if (m_truthSelectionTool.get() ) {
    m_truthCutFlow = CutFlow(m_truthSelectionTool->nCuts());
  }

  m_monPlots = std::make_unique<InDetRttPlots>
    (nullptr, static_cast<std::string>(m_dirName) + static_cast<std::string>(m_folder),
     getFilledPlotConfig()); // m_detailLevel := DEBUG, enable expert histograms

  ATH_CHECK( m_trkParticleName.initialize() );
  ATH_CHECK( m_truthParticleName.initialize( (m_pileupSwitch == "HardScatter" or m_pileupSwitch == "All") and not m_truthParticleName.key().empty() ) );
  ATH_CHECK( m_vertexContainerName.initialize( not m_vertexContainerName.empty() ) );
  ATH_CHECK( m_truthVertexContainerName.initialize( not m_truthVertexContainerName.key().empty() ) );
  ATH_CHECK( m_eventInfoContainerName.initialize() );

  ATH_CHECK( m_truthEventName.initialize( (m_pileupSwitch == "HardScatter" or m_pileupSwitch == "All") and not m_truthEventName.key().empty() ) );
  ATH_CHECK( m_truthPileUpEventName.initialize( (m_pileupSwitch == "PileUp" or m_pileupSwitch == "All") and not m_truthPileUpEventName.key().empty() ) );
  ATH_CHECK( m_jetContainerName.initialize( not m_jetContainerName.key().empty()) );  

  std::vector<std::string> required_float_track_decorations {"d0","hitResiduals_residualLocX","d0err"};
  std::vector<std::string> required_int_track_decorations {};
  std::vector<std::string> required_float_truth_decorations {"d0"};
  std::vector<std::string> required_int_truth_decorations {};
  std::vector<std::string> required_int_jet_decorations {"HadronConeExclTruthLabelID"};

  std::string empty_prefix;
  IDPVM::addReadDecoratorHandleKeys(*this, m_trkParticleName, empty_prefix, required_float_track_decorations, m_floatTrkDecor);
  IDPVM::addReadDecoratorHandleKeys(*this, m_trkParticleName, empty_prefix, required_int_truth_decorations,   m_intTrkDecor);
  if (!m_truthParticleName.key().empty()) {
    IDPVM::addReadDecoratorHandleKeys(*this, m_truthParticleName, empty_prefix, required_float_truth_decorations, m_floatTruthDecor);
    IDPVM::addReadDecoratorHandleKeys(*this, m_truthParticleName, empty_prefix, required_int_truth_decorations,   m_intTruthDecor);
  }
  if (m_doTrackInJetPlots && m_doBjetPlots){
    IDPVM::addReadDecoratorHandleKeys(*this, m_jetContainerName, empty_prefix, required_int_jet_decorations, m_intJetDecor);
  }

  m_usingSpecialPileupSwitch = (m_pileupSwitch != "All");
  return StatusCode::SUCCESS;
}


InDetRttPlotConfig InDetPhysValMonitoringTool::getFilledPlotConfig() const{


  InDetRttPlotConfig rttConfig; 
  rttConfig.detailLevel = m_detailLevel; 

  rttConfig.doTrkInJetPlots = m_doTrackInJetPlots;
  rttConfig.doTrkInJetPlots_fake = m_doTrackInJetPlots;
  rttConfig.doTrkInJetPlots_matched = m_doTrackInJetPlots;
  rttConfig.doTrkInJetPlots_unlinked = m_doTrackInJetPlots;

  rttConfig.doHitResidualPlot = m_doHitLevelPlots; 
  rttConfig.doHitEffPlot = m_doHitLevelPlots; 

  rttConfig.doTrkInJetPlots_bjets = m_doBjetPlots;
  rttConfig.doTrkInJetPlots_fake_bjets = m_doBjetPlots;
  rttConfig.doTrkInJetPlots_matched_bjets = m_doBjetPlots;
  rttConfig.doTrkInJetPlots_unlinked_bjets = m_doBjetPlots;

  rttConfig.doTrkInJetPlots_truthFromB = m_doTruthOriginPlots && m_doTrackInJetPlots; 
  rttConfig.doResolutionPlotPrim_truthFromB = m_doTruthOriginPlots; 

  rttConfig.doNtupleTruthToReco = m_fillTruthToRecoNtuple;

  rttConfig.doFakesPerAuthor = m_doPerAuthorPlots;
  rttConfig.doTrackParametersPerAuthor = m_doPerAuthorPlots;
  rttConfig.doEfficienciesPerAuthor = m_doPerAuthorPlots;
  rttConfig.doResolutionsPerAuthor = m_doPerAuthorPlots;

  rttConfig.doTrtExtensionPlots = m_doTRTExtensionPlots;

  /// turn off truth if none is present
  if (m_truthParticleName.key().empty()){
    rttConfig.doFakePlots = false; 
    rttConfig.doMissingTruthFakePlots = false; 
    rttConfig.doHitsFakeTracksPlots = false; 
    rttConfig.doHitsUnlinkedTracksPlots = false; 
    rttConfig.doEffPlots = false; 
    rttConfig.doResolutionPlotPrim = false; 
    rttConfig.doResolutionPlotPrim_truthFromB = false; 
    rttConfig.doResolutionPlotSecd = false; 
    rttConfig.doHitsMatchedTracksPlots = false; 
    rttConfig.doVertexTruthMatchingPlots = false; 
    rttConfig.doHardScatterVertexTruthMatchingPlots = false; 
    rttConfig.doEfficienciesPerAuthor = false; 
    rttConfig.doFakesPerAuthor = false; 
    rttConfig.doResolutionsPerAuthor = false; 
    rttConfig.doNtupleTruthToReco = false; 
    rttConfig.doTrkInJetPlots_fake_bjets = false;
    rttConfig.doTrkInJetPlots_matched_bjets = false;
    rttConfig.doTrkInJetPlots_unlinked_bjets = false;
    rttConfig.doTrkInJetPlots_truthFromB = false;
    rttConfig.doResolutionPlotPrim_truthFromB = false;
  }

  /// allow to disable non-truth-matched 
  if (m_onlyFillMatched){ 
    rttConfig.doTrackParameters = false;
    rttConfig.doNTracks = false;
    rttConfig.doHitResidualPlot = false;
    rttConfig.doHitEffPlot = false;
    rttConfig.doHitsRecoTracksPlots = false;
    rttConfig.doTrtExtensionPlots = false;
    rttConfig.doFakePlots = false;
    rttConfig.doMissingTruthFakePlots = false;
    rttConfig.doHitsFakeTracksPlots = false;
    rttConfig.doHitsUnlinkedTracksPlots = false;
    rttConfig.doVertexPlots = false;
    rttConfig.doVerticesVsMuPlots = false;
    rttConfig.doHardScatterVertexPlots = false;
    rttConfig.doVertexTruthMatchingPlots = false;
    rttConfig.doHardScatterVertexTruthMatchingPlots = false;
    rttConfig.doAnTrackingPlots = false;
    rttConfig.doTrkInJetPlots = false;
    rttConfig.doTrkInJetPlots_bjets = false;
    rttConfig.doTrkInJetPlots_matched = false;
    rttConfig.doTrkInJetPlots_matched_bjets = false;
    rttConfig.doTrkInJetPlots_fake = false;
    rttConfig.doTrkInJetPlots_fake_bjets = false;
    rttConfig.doTrkInJetPlots_unlinked = false;
    rttConfig.doTrkInJetPlots_unlinked_bjets = false;
    rttConfig.doTrkInJetPlots_truthFromB = false;
  }

  // For IDTIDE derivation
  // disable the vertex plots since no covariance from IDTIDE
  if (m_doIDTIDEPlots){ 
    rttConfig.doVertexPlots = false;
    rttConfig.doVerticesVsMuPlots = false;
    rttConfig.doHardScatterVertexPlots = false;
    rttConfig.doVertexTruthMatchingPlots = false;
    rttConfig.doHardScatterVertexTruthMatchingPlots = false;
    rttConfig.doTrkInJetPlots = true;
    rttConfig.doTrkInJetPlots_bjets = true;
    rttConfig.doTrkInJetPlots_matched = true;
    rttConfig.doTrkInJetPlots_matched_bjets = true;
    rttConfig.doTrkInJetPlots_fake = true;
    rttConfig.doTrkInJetPlots_fake_bjets = true;
    rttConfig.doTrkInJetPlots_unlinked = true;
    rttConfig.doTrkInJetPlots_unlinked_bjets = true;
    rttConfig.doTrkInJetPlots_truthFromB = true;
  }

  /// account for detail level 
  if (m_detailLevel < 200){
    rttConfig.doResolutionPlotSecd = false;
    rttConfig.doHitsMatchedTracksPlots = false;
    rttConfig.doHitsFakeTracksPlots = false; 
    rttConfig.doHitsUnlinkedTracksPlots = false; 
    rttConfig.doVertexTruthMatchingPlots = false; 
    rttConfig.doFakesPerAuthor = false;
    rttConfig.doTrackParametersPerAuthor = false;
    rttConfig.doEfficienciesPerAuthor = false;
    rttConfig.doResolutionsPerAuthor = false;
    rttConfig.doTrkInJetPlots_matched = false;  
    rttConfig.doTrkInJetPlots_fake = false;  
    rttConfig.doTrkInJetPlots_unlinked = false;  
    rttConfig.doTrkInJetPlots_matched_bjets = false;  
    rttConfig.doTrkInJetPlots_fake_bjets = false;  
    rttConfig.doTrkInJetPlots_unlinked_bjets = false;  
  }

  return rttConfig; 
}

StatusCode
InDetPhysValMonitoringTool::fillHistograms() {
  ATH_MSG_DEBUG("Filling hists " << name() << "...");
  // function object could be used to retrieve truth: IDPVM::CachedGetAssocTruth getTruth;

  // retrieve trackParticle container
  SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_trkParticleName);
  SG::ReadHandle<xAOD::TruthPileupEventContainer> truthPileupEventContainer;
  if( not m_truthPileUpEventName.key().empty()) {
    truthPileupEventContainer = SG::ReadHandle<xAOD::TruthPileupEventContainer> (m_truthPileUpEventName);
  }

  SG::ReadHandle<xAOD::EventInfo> pie = SG::ReadHandle<xAOD::EventInfo>(m_eventInfoContainerName);

  if (m_useGRL and !pie->eventType(xAOD::EventInfo::IS_SIMULATION)) {
      if (!m_grlTool->passRunLB(*pie)) {
	  ATH_MSG_VERBOSE("GRL veto");
	  return StatusCode::SUCCESS;
      }
  }
 
  std::vector<const xAOD::TruthParticle*> truthParticlesVec = getTruthParticles();

  // Mark the truth particles in our vector as "selected". 
  // This is needed because we later access the truth matching via xAOD decorations, where we do not 'know' about membership to this vector.
  if (m_usingSpecialPileupSwitch) markSelectedByPileupSwitch(truthParticlesVec);

  IDPVM::CachedGetAssocTruth getAsTruth; // only cache one way, track->truth, not truth->tracks 

  if (not tracks.isValid()) {
    ATH_MSG_ERROR("Invalid trackname = " << m_trkParticleName << "!");
    return StatusCode::FAILURE;
  }
  if (not pie.isValid()){
    ATH_MSG_WARNING("Shouldn't happen - EventInfo is buggy, setting mu to 0");
  }

  const xAOD::Vertex* primaryvertex = nullptr;
  float puEvents = 0;
  float nVertices = 0;
  float beamSpotWeight = 1;

  if(not m_vertexContainerName.key().empty()){
    ATH_MSG_DEBUG("Getting number of pu interactings per event");

    ATH_MSG_DEBUG("Filling vertex plots");
    SG::ReadHandle<xAOD::VertexContainer>  vertices(m_vertexContainerName);
    puEvents = !m_truthPileUpEventName.key().empty() and truthPileupEventContainer.isValid() ?  static_cast<int>( truthPileupEventContainer->size() ) : pie.isValid() ? pie->actualInteractionsPerCrossing() : 0;
    nVertices = not vertices->empty() ? vertices->size() : 0;
    beamSpotWeight = pie->beamSpotWeight();
    ATH_MSG_DEBUG("beamSpotWeight is equal to " <<  beamSpotWeight);

    if (vertices.isValid() and not vertices->empty()) {
      ATH_MSG_DEBUG("Number of vertices retrieved for this event " << vertices->size());
      //Find the HS vertex following the user-configured strategy
      primaryvertex = m_hardScatterSelectionTool->getHardScatter(vertices.get());
      if (!primaryvertex){
	/// In case of no HS, print a debug message - no warning since this is expected
	/// in single particle MC. The downstream code is able to handle the absence of a HS vertex.
	ATH_MSG_DEBUG("Failed to find a hard scatter vertex in this event.");
      }
      //Filling plots for all reconstructed vertices and the hard-scatter
      ATH_MSG_DEBUG("Filling vertices info monitoring plots");

      // Fill vectors of truth HS and PU vertices
      std::pair<std::vector<const xAOD::TruthVertex*>, std::vector<const xAOD::TruthVertex*>> truthVertices = getTruthVertices();
      std::vector<const xAOD::TruthVertex*> truthHSVertices = truthVertices.first;
      std::vector<const xAOD::TruthVertex*> truthPUVertices = truthVertices.second;

      // Decorate vertices
      if (m_useVertexTruthMatchTool && m_vtxValidTool) {
	ATH_CHECK(m_vtxValidTool->matchVertices(*vertices));
	ATH_MSG_DEBUG("Hard scatter classification type: " << InDetVertexTruthMatchUtils::classifyHardScatter(*vertices) << ", vertex container size = " << vertices->size());
      }
      m_monPlots->fill(*vertices, primaryvertex, truthHSVertices, truthPUVertices, beamSpotWeight);

      ATH_MSG_DEBUG("Filling vertex/event info monitoring plots");
      //Filling vertexing plots for the reconstructed hard-scatter as a function of mu
      m_monPlots->fill(*vertices, puEvents, beamSpotWeight);
    } else {
      //FIXME: Does this happen for single particles?
      ATH_MSG_WARNING("Skipping vertexing plots.");
    }
  }


  if( not m_truthVertexContainerName.key().empty()){
    // get truth vertex container name - m_truthVertexContainerName
    SG::ReadHandle<xAOD::TruthVertexContainer> truthVrt = SG::ReadHandle<xAOD::TruthVertexContainer>( m_truthVertexContainerName );

    //
    //Get the HS vertex position from the truthVertexContainer
    //FIXME: Add plots w.r.t truth HS positions (vertexing plots)
    //
    const xAOD::TruthVertex* truthVertex = nullptr;
    if (truthVrt.isValid()) {
      const auto& stdVertexContainer = truthVrt->stdcont();
      //First truth particle vertex?
      auto findVtx = std::find_if(stdVertexContainer.rbegin(), stdVertexContainer.rend(), acceptTruthVertex);
      truthVertex = (findVtx == stdVertexContainer.rend()) ? nullptr : *findVtx;
    } else {
      ATH_MSG_WARNING("Cannot open " << m_truthVertexContainerName.key() << " truth vertex container");
    }
    if (not truthVertex) ATH_MSG_INFO ("Truth vertex did not pass cuts");
  }
  //
  //Counters for cutflow
  //
  unsigned int nSelectedTruthTracks(0), nSelectedRecoTracks(0), nSelectedMatchedTracks(0), nAssociatedTruth(0), nMissingAssociatedTruth(0), nTruths(0);

  CutFlow tmp_truth_cutflow( m_truthSelectionTool.get() ?  m_truthSelectionTool->nCuts() : 0 );
  
  //
  //Loop over all reconstructed tracks
  //
  // If writing ntuples, use a truth-to-track(s) cache to handle truth matching.
  // Based on the assumption that multiple tracks can (albeit rarely) share the same truth association.
  std::map<const xAOD::TruthParticle*, std::vector<const xAOD::TrackParticle*>> cacheTruthMatching {};
  //
  std::vector<const xAOD::TrackParticle*> selectedTracks {};
  selectedTracks.reserve(tracks->size());
  unsigned int nTrackBAT = 0, nTrackSTD = 0, nTrackANT = 0, nTrackTOT = 0;
  for (const auto *const thisTrack: *tracks) {
    //FIXME: Why is this w.r.t the primary vertex?
    const asg::AcceptData& accept = m_trackSelectionTool->accept(*thisTrack, primaryvertex);
    if (m_useTrackSelection and not accept) continue;
    fillTrackCutFlow(accept); //?? Is this equal???

    selectedTracks.push_back(thisTrack);
    //Number of selected reco tracks
    nSelectedRecoTracks++;

    //Fill plots for selected reco tracks, hits / perigee / ???
    std::bitset<xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo>  patternInfo = thisTrack->patternRecoInfo();
    bool isBAT = patternInfo.test(xAOD::TrackPatternRecoInfo::TRTSeededTrackFinder);
    bool isANT = patternInfo.test(xAOD::TrackPatternRecoInfo::SiSpacePointsSeedMaker_LargeD0);
    bool isSTD = not isBAT and not isANT;
    if(isBAT) nTrackBAT++;
    if(isSTD) nTrackSTD++;
    if(isANT) nTrackANT++;
    nTrackTOT++;
    m_monPlots->fill(*thisTrack, beamSpotWeight);                                      
    m_monPlots->fill(*thisTrack, puEvents, nVertices, beamSpotWeight);  //fill mu dependent plots
    const xAOD::TruthParticle* associatedTruth = getAsTruth.getTruth(thisTrack);
    float prob = getMatchingProbability(*thisTrack); 

    // This is where the Fake, and Really Fake fillers need to go. Where would the really really fakes go?
    if (associatedTruth) {
      nAssociatedTruth++;

      // if there is associated truth also a truth selection tool was retrieved.
      IAthSelectionTool::CutResult passed( m_truthSelectionTool ?  m_truthSelectionTool->accept(associatedTruth) : IAthSelectionTool::CutResult(0) );
      if (m_truthSelectionTool) {
        //FIXME: What is this for???
        tmp_truth_cutflow.update( passed.missingCuts() );
      }

      if ((not std::isnan(prob)) and (prob > m_lowProb) and passed and (not m_usingSpecialPileupSwitch or isSelectedByPileupSwitch(*associatedTruth)) ) {
        nSelectedMatchedTracks++; 
        bool truthIsFromB = false;
        if ( m_doTruthOriginPlots and m_trackTruthOriginTool->isFrom(associatedTruth, 5) ) {
          truthIsFromB = true;
        }
        m_monPlots->fill(*thisTrack, *associatedTruth, truthIsFromB, puEvents, beamSpotWeight); // Make plots requiring matched truth
      }
    }

    const bool isAssociatedTruth = associatedTruth != nullptr;
    const bool isFake = not std::isnan(prob) ? (prob < m_lowProb) : true;

    if(!isAssociatedTruth) nMissingAssociatedTruth++;
    m_monPlots->fillFakeRate(*thisTrack, isFake, isAssociatedTruth, puEvents, nVertices, beamSpotWeight);

    if (m_fillTruthToRecoNtuple) {
      // Decorate track particle with extra flags
      decorateTrackParticle(*thisTrack, accept);

      if (isAssociatedTruth) {
        // Decorate truth particle with extra flags
        decorateTruthParticle(*associatedTruth, m_truthSelectionTool->accept(associatedTruth));

        // Cache truth-to-track associations
        auto cachedAssoc = cacheTruthMatching.find(associatedTruth);
        // Check if truth particle already present in cache
        if (cachedAssoc == cacheTruthMatching.end()) {
          // If not yet present, add truth-to-track association in cache
          cacheTruthMatching[associatedTruth] = {thisTrack};
        }
        else {
          // If already present, cache additional track associations (here multiple track particle can be linked to the same truth particle)
          cachedAssoc->second.push_back(thisTrack);
        }
      }
      else {
        // Fill track only entries with dummy truth values
        m_monPlots->fillNtuple(*thisTrack, primaryvertex);
      }
    }
  }
  if (m_fillTruthToRecoNtuple) {
    // Now fill all truth-to-track associations
    // Involves some double-filling of truth particles in cases where multiple tracks share the same truth association,
    // these duplicates can be filtered in the ntuple by selecting only the 'best matched' truth-associated track particles.
    for (auto& cachedAssoc: cacheTruthMatching) {
      const xAOD::TruthParticle* thisTruth = cachedAssoc.first;

      // Decorate that this truth particle is being filled to prevent double counting in truth particle loop
      m_dec_hasTruthFilled(*thisTruth) = true;

      // Sort all associated tracks by truth match probability
      std::sort(cachedAssoc.second.begin(), cachedAssoc.second.end(), 
        [](const xAOD::TrackParticle* t1, const xAOD::TrackParticle* t2) { return getMatchingProbability(*t1) > getMatchingProbability(*t2); }
      );

      // Fill all tracks associated to to this truth particle, also recording 'truth match ranking' as index in probability-sorted vector of matched tracks
      for (int itrack = 0; itrack < (int) cachedAssoc.second.size(); itrack++) {
          const xAOD::TrackParticle* thisTrack = cachedAssoc.second[itrack];
          
          // Fill track entries with truth association
          m_monPlots->fillNtuple(*thisTrack, *thisTruth, primaryvertex, itrack);
      }
    }
  }
  m_monPlots->fill(nTrackANT, nTrackSTD, nTrackBAT, puEvents, nVertices,beamSpotWeight);
  m_monPlots->fill(nTrackTOT, puEvents, nVertices,beamSpotWeight);

  //FIXME: I don't get why... this is here
  if (m_truthSelectionTool.get()) {
    ATH_MSG_DEBUG( CutFlow(tmp_truth_cutflow).report(m_truthSelectionTool->names()) );
    std::lock_guard<std::mutex> lock(m_mutex);
    m_truthCutFlow.merge(std::move(tmp_truth_cutflow));
  }
  
  //
  //TruthParticle loop to fill efficiencies
  //
  for (int itruth = 0; itruth < (int) truthParticlesVec.size(); itruth++) {  // Outer loop over all truth particles
    nTruths++;
    const xAOD::TruthParticle* thisTruth = truthParticlesVec[itruth];

    // if the vector of truth particles is not empty also a truthSelectionTool was retrieved
    const IAthSelectionTool::CutResult accept = m_truthSelectionTool->accept(thisTruth);
    if (accept) {
      ++nSelectedTruthTracks; // total number of truth which pass cuts per event
      bool isEfficient(false); // weight for the trackeff histos
      
      m_monPlots->fill(*thisTruth, beamSpotWeight); // This is filling truth-only plots
      //
      //Loop over reco tracks to find the match
      //
      const xAOD::TrackParticle* matchedTrack = nullptr;
      for (const auto& thisTrack: selectedTracks) { // Inner loop over selected track particleis
        const xAOD::TruthParticle* associatedTruth = getAsTruth.getTruth(thisTrack);
        if (associatedTruth && associatedTruth == thisTruth) {
          float prob = getMatchingProbability(*thisTrack);
          if (not std::isnan(prob) && prob > m_lowProb) {
            isEfficient = true;
            matchedTrack = thisTrack;
            break;
          }
        }
      }
      if (!thisTruth){ 
        ATH_MSG_ERROR("An error occurred: Truth particle for tracking efficiency calculation is a nullptr");
      }
      else if (isEfficient && !matchedTrack){
        ATH_MSG_ERROR("Something went wrong - we log a truth particle as reconstructed, but the reco track is a nullptr! Bailing out... ");  
      }
      else{ 
        ATH_MSG_DEBUG("Filling efficiency plots info monitoring plots");
        m_monPlots->fillEfficiency(*thisTruth, matchedTrack, isEfficient, puEvents, nVertices, beamSpotWeight);
        if (m_fillTechnicalEfficiency) {
          ATH_MSG_DEBUG("Filling technical efficiency plots info monitoring plots");
          static const SG::AuxElement::ConstAccessor< float > nSilHitsAcc("nSilHits");
          if (nSilHitsAcc.isAvailable(*thisTruth)) {
            if (nSilHitsAcc(*thisTruth) >= m_minHits.value().at(getIndexByEta(*thisTruth)))
              m_monPlots->fillTechnicalEfficiency(*thisTruth, isEfficient, puEvents, beamSpotWeight);
          } else {
            ATH_MSG_DEBUG("Cannot fill technical efficiency. Missing si hit information for truth particle.");
          }
        }
      }
    }
    
    if (m_fillTruthToRecoNtuple) {
      // Skip if already filled in track loop
      if (hasTruthFilled(*thisTruth)) continue;

      // Decorate truth particle with extra flags
      decorateTruthParticle(*thisTruth, accept);
      
      // Fill truth only entries with dummy track values
      m_monPlots->fillNtuple(*thisTruth);
    }
  }

  if (nSelectedRecoTracks == nMissingAssociatedTruth) {
    if (not m_truthParticleName.key().empty()) {
      ATH_MSG_DEBUG("NO TRACKS had associated truth.");
    }
  } else {
    ATH_MSG_DEBUG(nAssociatedTruth << " tracks out of " << tracks->size() << " had associated truth.");
  }

  m_monPlots->fillCounter(nSelectedRecoTracks, InDetPerfPlot_nTracks::SELECTEDRECO, beamSpotWeight);
  m_monPlots->fillCounter(tracks->size(), InDetPerfPlot_nTracks::ALLRECO, beamSpotWeight);
  m_monPlots->fillCounter(nSelectedTruthTracks, InDetPerfPlot_nTracks::SELECTEDTRUTH, beamSpotWeight);
  m_monPlots->fillCounter(nTruths, InDetPerfPlot_nTracks::ALLTRUTH, beamSpotWeight);
  m_monPlots->fillCounter(nAssociatedTruth, InDetPerfPlot_nTracks::ALLASSOCIATEDTRUTH, beamSpotWeight);
  m_monPlots->fillCounter(nSelectedMatchedTracks, InDetPerfPlot_nTracks::MATCHEDRECO, beamSpotWeight);
  
  // Tracking In Dense Environment
  if (!m_doTrackInJetPlots) return StatusCode::SUCCESS;

  if (m_jetContainerName.key().empty()) {
    ATH_MSG_WARNING("Not a valid key for the jetContainer, skipping TIDE plots");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::JetContainer> jets(m_jetContainerName);
  SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer> > > ghosttruth("GhostTruth");
  SG::AuxElement::ConstAccessor<int> btagLabel("HadronConeExclTruthLabelID");
  
  if (not jets.isValid() or truthParticlesVec.empty()) {
    ATH_MSG_WARNING(
      "Cannot open " << m_jetContainerName <<
        " jet container or TruthParticles truth particle container. Skipping jet plots.");
  } else {
    for (const auto *const thisJet: *jets) {         // The big jets loop
      if (not passJetCuts(*thisJet)) {
        continue;
      }
      bool isBjet = false; 
      if (!btagLabel.isAvailable(*thisJet)){
           ATH_MSG_WARNING("Failed to extract b-tag truth label from jet");
      }
      else{
        isBjet = (btagLabel(*thisJet) == 5); 
      }
      if(!ghosttruth.isAvailable(*thisJet)) {
           ATH_MSG_WARNING("Failed to extract ghost truth particles from jet");
      } else {
        for(const auto& el : ghosttruth(*thisJet)){ 
          if(el.isValid()) {
            const xAOD::TruthParticle *truth = static_cast<const xAOD::TruthParticle*>(*el);
            if (thisJet->p4().DeltaR(truth->p4()) > m_maxTrkJetDR) {
                continue;
            }

            const IAthSelectionTool::CutResult accept = m_truthSelectionTool->accept(truth);
              
            if(!accept) continue;
            bool isEfficient(false);

            for (const auto *thisTrack: *tracks) {
              if (m_useTrackSelection and not (m_trackSelectionTool->accept(*thisTrack, primaryvertex))) {
                continue;
              }

              const xAOD::TruthParticle* associatedTruth = getAsTruth.getTruth(thisTrack);
              if (associatedTruth and associatedTruth == truth) {
                float prob = getMatchingProbability(*thisTrack);
                if (not std::isnan(prob) && prob > m_lowProb) {
                  isEfficient = true;
                  break;
                }
              }
            }

            bool truthIsFromB = false;
            if ( m_doTruthOriginPlots and m_trackTruthOriginTool->isFrom(truth, 5) ) {
              truthIsFromB = true;
            }
            m_monPlots->fillEfficiency(*truth, *thisJet, isEfficient, isBjet, truthIsFromB, beamSpotWeight);
          }
        }
      }

      for (const auto *thisTrack: *tracks) {    // The beginning of the track loop
        if (m_useTrackSelection and not (m_trackSelectionTool->accept(*thisTrack, primaryvertex))) {
          continue;
        }
        if (thisJet->p4().DeltaR(thisTrack->p4()) > m_maxTrkJetDR) {
          continue;
        }
        float prob = getMatchingProbability(*thisTrack);
        if(std::isnan(prob)) prob = 0.0;
      
        const xAOD::TruthParticle* associatedTruth = getAsTruth.getTruth(thisTrack); 
        const bool unlinked = (associatedTruth==nullptr);
        const bool isFake = (associatedTruth && prob < m_lowProb);
        bool truthIsFromB = false;
        if ( m_doTruthOriginPlots and m_trackTruthOriginTool->isFrom(associatedTruth, 5) ) {
          truthIsFromB = true;
        }
        m_monPlots->fill(*thisTrack, *thisJet, isBjet, isFake, unlinked, truthIsFromB, beamSpotWeight);                                   
        if (associatedTruth){
          m_monPlots->fillFakeRate(*thisTrack, *thisJet, isFake, isBjet, truthIsFromB, beamSpotWeight);
       }
      }
    }
  } // loop over jets

  return StatusCode::SUCCESS;
}

void InDetPhysValMonitoringTool::decorateTrackParticle(const xAOD::TrackParticle & track, const asg::AcceptData & passed) const {
  // Decorate outcome of track selection
  m_dec_passedTrackSelection(track) = (bool)(passed);
}

void InDetPhysValMonitoringTool::decorateTruthParticle(const xAOD::TruthParticle & truth, const IAthSelectionTool::CutResult & passed) const {  
  // Decorate outcome of truth selection
  m_dec_passedTruthSelection(truth) = (bool)(passed);

  // Decorate if selected by pileup switch
  m_dec_selectedByPileupSwitch(truth) = isSelectedByPileupSwitch(truth);
}

bool InDetPhysValMonitoringTool::hasTruthFilled(const xAOD::TruthParticle & truth) const{
  if (!m_acc_hasTruthFilled.isAvailable(truth)) {
    ATH_MSG_DEBUG("Truth particle not yet filled in ntuple");
    return false;
  }
  return m_acc_hasTruthFilled(truth);
}

bool InDetPhysValMonitoringTool::isSelectedByPileupSwitch(const xAOD::TruthParticle & truth) const{
  if (!m_acc_selectedByPileupSwitch.isAvailable(truth)) {
      ATH_MSG_DEBUG("Selected by pileup switch decoration requested from a truth particle but not available");
      return false;
  }
  return m_acc_selectedByPileupSwitch(truth);
}

void InDetPhysValMonitoringTool::markSelectedByPileupSwitch(const std::vector<const xAOD::TruthParticle*> & truthParticles) const{
  for (const auto& thisTruth: truthParticles) {
    m_dec_selectedByPileupSwitch(*thisTruth) = true;
  }
}

StatusCode
InDetPhysValMonitoringTool::bookHistograms() {
  ATH_MSG_INFO("Booking hists " << name() << "with detailed level: " << m_detailLevel);
  m_monPlots->initialize();
  std::vector<HistData> hists = m_monPlots->retrieveBookedHistograms();
  for (const auto& hist : hists) {
    ATH_CHECK(regHist(hist.first, hist.second, all)); // ??
  }
  // do the same for Efficiencies, but there's a twist:
  std::vector<EfficiencyData> effs = m_monPlots->retrieveBookedEfficiencies();
  for (auto& eff : effs) {
    // reg**** in the monitoring baseclass doesnt have a TEff version, but TGraph *
    // pointers just get passed through, so we use that method after an ugly cast
    ATH_CHECK(regGraph(reinterpret_cast<TGraph*>(eff.first), eff.second, all)); // ??
  }
  // register trees for ntuple writing
  if (m_fillTruthToRecoNtuple) {
    std::vector<TreeData> trees = m_monPlots->retrieveBookedTrees();
    for (auto& t : trees) {
      ATH_CHECK(regTree(t.first, t.second, all));
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode
InDetPhysValMonitoringTool::procHistograms() {
  ATH_MSG_INFO("Finalising hists " << name() << "...");
  //TODO: ADD Printouts for Truth??
  if (m_useTrackSelection) {
    ATH_MSG_INFO("");
    ATH_MSG_INFO("Now Cutflow for track cuts:");
    ATH_MSG_INFO("");
    for (int i = 0; i < (int) m_trackCutflow.size(); ++i) {
      ATH_MSG_INFO("number after " << m_trackCutflowNames[i] << ": " << m_trackCutflow[i]);
    }
  }

  ATH_MSG_INFO("");
  ATH_MSG_INFO("Cutflow for truth tracks:");
  if (m_truthSelectionTool.get()) {
    ATH_MSG_INFO("Truth selection report: " << m_truthCutFlow.report( m_truthSelectionTool->names()) );
  }
  if (endOfRunFlag()) {
    m_monPlots->finalize();
  }
  ATH_MSG_INFO("Successfully finalized hists");
  return StatusCode::SUCCESS;
}

const std::vector<const xAOD::TruthParticle*>
InDetPhysValMonitoringTool::getTruthParticles() const {
  // truthParticles.clear();
  std::vector<const xAOD::TruthParticle*> tempVec {};
  if (m_pileupSwitch == "All") {

    if (m_truthParticleName.key().empty()) {
      return tempVec;
    }
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainer( m_truthParticleName);
    if (not truthParticleContainer.isValid()) {
      return tempVec;
    }
    tempVec.insert(tempVec.begin(), truthParticleContainer->begin(), truthParticleContainer->end());
  } else {
    if (m_pileupSwitch == "HardScatter") {
      // get truthevent container to separate out pileup and hardscatter truth particles
      if (not m_truthEventName.key().empty()) {
      SG::ReadHandle<xAOD::TruthEventContainer> truthEventContainer( m_truthEventName);
      const xAOD::TruthEvent* event = (truthEventContainer.isValid()) ? truthEventContainer->at(0) : nullptr;
      if (not event) {
        return tempVec;
      }
      const auto& links = event->truthParticleLinks();
      tempVec.reserve(event->nTruthParticles());
      for (const auto& link : links) {
        if (link.isValid()){
          tempVec.push_back(*link);
        }
      }
      }
    } else if (m_pileupSwitch == "PileUp") {
      if (not m_truthPileUpEventName.key().empty()) {
      ATH_MSG_VERBOSE("getting TruthPileupEvents container");
      // get truth particles from all pileup events
      SG::ReadHandle<xAOD::TruthPileupEventContainer> truthPileupEventContainer(m_truthPileUpEventName);
      if (truthPileupEventContainer.isValid()) {
        const unsigned int nPileup = truthPileupEventContainer->size();
        tempVec.reserve(nPileup * 200); // quick initial guess, will still save some time
        for (unsigned int i(0); i != nPileup; ++i) {
          const auto *eventPileup = truthPileupEventContainer->at(i);
          // get truth particles from each pileup event
          int ntruth = eventPileup->nTruthParticles();
          ATH_MSG_VERBOSE("Adding " << ntruth << " truth particles from TruthPileupEvents container");
          const auto& links = eventPileup->truthParticleLinks();
          for (const auto& link : links) {
            if (link.isValid()){
              tempVec.push_back(*link);
            }
          }
        }
      } else {
        ATH_MSG_ERROR("no entries in TruthPileupEvents container!");
      }
      }
    } else {
      ATH_MSG_ERROR("bad value for PileUpSwitch");
    }
  }
  return tempVec;
}

std::pair<const std::vector<const xAOD::TruthVertex*>, const std::vector<const xAOD::TruthVertex*>>
InDetPhysValMonitoringTool::getTruthVertices() const {

  std::vector<const xAOD::TruthVertex*> truthHSVertices = {};
  truthHSVertices.reserve(5);
  std::vector<const xAOD::TruthVertex*> truthPUVertices = {};
  truthPUVertices.reserve(100);
  const xAOD::TruthVertex* truthVtx = nullptr;

  bool doHS = false;
  bool doPU = false;
  if (m_pileupSwitch == "All") {
    doHS = true;
    doPU = true;
  }
  else if (m_pileupSwitch == "HardScatter") {
    doHS = true;
  }
  else if (m_pileupSwitch == "PileUp") {
    doPU = true;
  }
  else {
    ATH_MSG_ERROR("Bad value for PileUpSwitch: " << m_pileupSwitch);
  }

  if (doHS) {
    if (not m_truthEventName.key().empty()) {
      ATH_MSG_VERBOSE("Getting HS TruthEvents container.");
      SG::ReadHandle<xAOD::TruthEventContainer> truthEventContainer(m_truthEventName);
      if (truthEventContainer.isValid()) {
        for (const auto *const evt : *truthEventContainer) {
          truthVtx = evt->signalProcessVertex();
          if (truthVtx) {
            truthHSVertices.push_back(truthVtx);
          }
        }
      }
      else {
        ATH_MSG_ERROR("No entries in TruthEvents container!");
      }
    }
  }

  if (doPU) {
    if (not m_truthPileUpEventName.key().empty()) {
      ATH_MSG_VERBOSE("Getting PU TruthEvents container.");
      SG::ReadHandle<xAOD::TruthPileupEventContainer> truthPileupEventContainer(m_truthPileUpEventName);
      if (truthPileupEventContainer.isValid()) {
        for (const auto *const evt : *truthPileupEventContainer) {
          // Get the PU vertex
          // In all cases tested i_vtx=2 for PU
          // but better to keep something generic
          truthVtx = nullptr;
          size_t i_vtx = 0; size_t n_vtx = evt->nTruthVertices();
          while(!truthVtx && i_vtx<n_vtx){
            truthVtx = evt->truthVertex(i_vtx);
            i_vtx++;
          }

          if (truthVtx) {
            truthPUVertices.push_back(truthVtx);
          }
        }
      }
      else {
        ATH_MSG_ERROR("No entries in TruthPileupEvents container!");
      }
    }
  }

  return std::make_pair<const std::vector<const xAOD::TruthVertex*>, const std::vector<const xAOD::TruthVertex*>>((const std::vector<const xAOD::TruthVertex*>)truthHSVertices, (const std::vector<const xAOD::TruthVertex*>)truthPUVertices);

}

void
InDetPhysValMonitoringTool::fillTrackCutFlow(const asg::AcceptData& accept) {
  fillCutFlow(accept, m_trackCutflowNames, m_trackCutflow);
}

void
InDetPhysValMonitoringTool::fillCutFlow(const asg::AcceptData& accept, std::vector<std::string>& names,
                                        std::vector<int>& cutFlow) {
  // initialise cutflows
  if (cutFlow.empty()) {
    names.emplace_back("preCut");
    cutFlow.push_back(0);
    for (unsigned int i = 0; i != accept.getNCuts(); ++i) {
      cutFlow.push_back(0);
      names.push_back((std::string) accept.getCutName(i));
    }
  }
  // get cutflow
  cutFlow[0] += 1;
  bool cutPositive = true;
  for (unsigned int i = 0; i != (accept.getNCuts() + 1); ++i) {
    if (!cutPositive) {
      continue;
    }
    if (accept.getCutResult(i)) {
      cutFlow[i + 1] += 1;
    } else {
      cutPositive = false;
    }
  }
  }

int InDetPhysValMonitoringTool::getIndexByEta(const xAOD::TruthParticle& truth) const {
  double absEta = std::abs(truth.eta());
  if (absEta > m_etaBins.value().back() || absEta < m_etaBins.value().front()) {
    absEta = std::clamp(absEta, m_etaBins.value().front(), m_etaBins.value().back());
    ATH_MSG_INFO("Requesting cut value outside of configured eta range: clamping eta = "
                 << std::abs(truth.eta()) << " to eta= " << absEta);
  } else
    absEta = std::clamp(absEta, m_etaBins.value().front(), m_etaBins.value().back());
  const auto pVal =  std::lower_bound(m_etaBins.value().begin(), m_etaBins.value().end(), absEta);
  const int bin = std::distance(m_etaBins.value().begin(), pVal) - 1;
  ATH_MSG_DEBUG("Checking (abs(eta)/bin) = (" << absEta << "," << bin << ")");
  return bin;
}
