/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

// **********************************************************************
// TrackSelectionTool.cxx
// AUTHORS: Ben Cooper
// **********************************************************************

#include <cmath>



#include "TrackSelectionAlg.h"

#include "AthContainers/ConstDataVector.h"
#include "TrkTrack/Track.h"
#include "TrkTrackSummary/TrackSummary.h"

//---------------------------------------------------------------------------------------

TrackSelectionAlg::TrackSelectionAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator) {
  declareProperty("TrackMinPt", m_trackMinPt = 2.0);
  declareProperty("TrackMinEta", m_trackMinEta = 0.0);
  declareProperty("TrackMaxEta", m_trackMaxEta = 10000.0);
  declareProperty("TrackMaxVtxZ0", m_trackMaxVtxZ0 = 150.0);
  declareProperty("TrackMaxD0", m_trackMaxD0 = 10.0);
  declareProperty("MinPixelHits", m_minPixelHits = 0);
  declareProperty("MinSCTPixHits", m_minSCTPixHits = 0);
  declareProperty("MinBLayerHits", m_minBLayerHits = 0);
  declareProperty("MinTRTHits", m_minTRTHits = 0);
  declareProperty("MinTRTHitsHT", m_minTRTHitsHT = 0);
}

//---------------------------------------------------------------------------------------

TrackSelectionAlg::~TrackSelectionAlg() = default;


//---------------------------------------------------------------------------------------

StatusCode TrackSelectionAlg::initialize() {
  // get TrackSummaryTool
  if (m_trackSumTool.retrieve().isFailure()) {
    msg(MSG::FATAL) << "Failed to retrieve tool " << m_trackSumTool << endmsg;
    return StatusCode::FAILURE;
  } else {
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Retrieved tool " << m_trackSumTool << endmsg;
  }

  msg(MSG::INFO) << "TrackSelectionAlg initialized" << endmsg;

  ATH_CHECK(m_inputTrackCol.initialize());
  ATH_CHECK(m_outputTrackCol.initialize());
  ATH_CHECK(m_vertices.initialize());

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------------------

StatusCode TrackSelectionAlg::finalize() {
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------------------

StatusCode TrackSelectionAlg::execute() {
  //if this method the decision on which trackcollection and whether to require TRT hits
  //is made from the configuration of the TrackSlectionTool (in jobOptions)

  //retrieving input track collection from Storegate
  SG::ReadHandle<TrackCollection> tracks {
    m_inputTrackCol
  };
  if (not tracks.isValid()) {
    msg(MSG::ERROR) << "No TrackCollection with name " << m_inputTrackCol << " found in StoreGate" << endmsg;
  } else {
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "TrackCollection with name " << m_inputTrackCol.key() <<
        " found in StoreGate" << endmsg;
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Retrieved " << tracks->size() <<
      " reconstructed tracks from StoreGate" << endmsg;
  }

  //getting primary vertex collection from Storegate
  SG::ReadHandle<VxContainer> vertices {
    m_vertices
  };
  if (not vertices.isValid()) {
    msg(MSG::ERROR) << "No Collection with name " << "VxPrimaryCandidate" << " found in StoreGate" << endmsg;
  } else if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Collection with name " << m_vertices.key() << " with size " <<
    vertices->size() << " found in StoreGate" << endmsg;


  //choosing the primary vertex with the largest number of associated tracks
  //extract the z coord of this vertex
  float zVtx = -999.0;
  VxContainer::const_iterator vxItr = vertices->begin();
  VxContainer::const_iterator vxItrE = vertices->end();
  int ntrkMax = 0;
  for (; vxItr != vxItrE; ++vxItr) {
    int numTracksPerVertex = (*vxItr)->vxTrackAtVertex()->size();
    if (numTracksPerVertex > ntrkMax) {
      ntrkMax = numTracksPerVertex;
      //xVtx=(*vxItr)->recVertex().position()[0];
      //yVtx=(*vxItr)->recVertex().position()[1];
      zVtx = (*vxItr)->recVertex().position()[2];
    }
  }

  //if there is no primary vertex set to zero
  if (vertices->empty()) {
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "No vertex found => setting it to 0.0" << endmsg;
    zVtx = 0.0;
  }


  //this is the track view that will be filled
  auto selectedTracks = std::make_unique<TrackCollection>(SG::VIEW_ELEMENTS);

  //looping over input track collection and implementing track selection cuts
  TrackCollection::const_iterator trksItr = tracks->begin();
  TrackCollection::const_iterator trksItrE = tracks->end();
  for (; trksItr != trksItrE; ++trksItr) {
    const Trk::Track* track = *trksItr;
    bool trackPassed = makeTrackCuts(track, zVtx);
    if (trackPassed) selectedTracks->push_back(new Trk::Track(*track));
  }

  SG::WriteHandle<TrackCollection> outputTrackCol {
    m_outputTrackCol
  };
  ATH_CHECK(outputTrackCol.record(std::move(selectedTracks)));

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------------------

bool TrackSelectionAlg::makeTrackCuts(const Trk::Track* track, float zVtx) {
  bool trackPassed = true;

  const Trk::Perigee* startPerigee = track->perigeeParameters();

  float theta = startPerigee->parameters()[Trk::theta];
  float qOverPt = startPerigee->parameters()[Trk::qOverP] / std::sin(theta);
  float charge = startPerigee->charge();
  float eta = startPerigee->eta();
  float z0 = startPerigee->parameters()[Trk::z0];
  float d0 = startPerigee->parameters()[Trk::d0];
  float pT = (1 / qOverPt) * (charge) / 1000;

  float z0zVtx = (std::fabs(z0 - zVtx)) * std::sin(theta);

  //making kinematic cuts on tracks
  if (pT < m_trackMinPt) trackPassed = false;
  if (std::fabs(eta) > m_trackMaxEta) trackPassed = false;
  if (std::fabs(eta) < m_trackMinEta) trackPassed = false;
  if (z0zVtx > m_trackMaxVtxZ0) trackPassed = false;
  if (std::fabs(d0) > m_trackMaxD0) trackPassed = false;

  std::unique_ptr<Trk::TrackSummary> summary = m_trackSumTool->summary(*track);

  int nhtrt = 0;
  int nhtrtHT = 0;
  int nhpix = 0;
  int nhsct = 0;
  int nhblay = 0;

  if (!summary) {
    if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) <<
      "Could not create TrackSummary  - Track will likely fail hits requirements" << endmsg;
  } else {
    nhpix = summary->get(Trk::numberOfPixelHits);
    nhblay = summary->get(Trk::numberOfInnermostPixelLayerHits);
    nhsct = summary->get(Trk::numberOfSCTHits);
    nhtrt = summary->get(Trk::numberOfTRTHits);
    nhtrtHT = summary->get(Trk::numberOfTRTHighThresholdHits);
  }

  //making cuts on hit content of track
  if (nhpix < m_minPixelHits) trackPassed = false;
  if (nhblay < m_minBLayerHits) trackPassed = false;
  if ((nhpix + nhsct) < m_minSCTPixHits) trackPassed = false;
  if (nhtrt < m_minTRTHits) trackPassed = false;
  if (nhtrtHT < m_minTRTHitsHT) trackPassed = false;

  return trackPassed;
}
