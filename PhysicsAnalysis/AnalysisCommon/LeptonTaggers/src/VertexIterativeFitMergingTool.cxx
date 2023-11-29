/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local
#include "LeptonTaggers/VertexIterativeFitMergingTool.h"
#include "LeptonTaggers/VarHolder.h"
#include "LeptonTaggers/PromptUtils.h"

// C/C++
#include <cmath>
#include <iostream>
#include <sstream>

//=============================================================================
Prompt::VertexIterativeFitMergingTool::VertexIterativeFitMergingTool(
  const std::string &name,
  const std::string &type,
  const IInterface  *parent
):
  AthAlgTool       (name, type, parent)
{
  declareInterface<Prompt::IVertexMergingTool>(this);
}

//=============================================================================
StatusCode Prompt::VertexIterativeFitMergingTool::initialize()
{
  ATH_CHECK(m_vertexFitterTool.retrieve());
  ATH_CHECK(m_histSvc.retrieve());

  ATH_CHECK(makeHist(m_histNvtx2TrkInit,     "nvtx_2trk_init",                   25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histNvtx2TrkPass,     "nvtx_2trk_pass",                   25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histNvtx2TrkUnmerged, "nvtx_2trk_unmerged",               25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histNvtxMerged,        "nvtx_merged",                      25, -0.5, 24.5));

  ATH_CHECK(makeHist(m_histNewVtxFitChi2,             "newVtxFit_chi2",                   100, 0.0, 50.0));
  ATH_CHECK(makeHist(m_histNewVtxFitProb,             "newVtxFit_prob",                   100, 0.0,  1.0));

  ATH_CHECK(makeHist(m_histNewVtxFitDistToCurr,      "newVtxFit_dist_toCurr",            100, 0.0, 10.0));
  ATH_CHECK(makeHist(m_histNewVtxFitDistToSeed,      "newVtxFit_dist_toSeed",            100, 0.0, 10.0));
  ATH_CHECK(makeHist(m_histNewVtxFitDistToSeedPass, "newVtxFit_dist_toSeed_pass",       100, 0.0, 10.0));
  ATH_CHECK(makeHist(m_histNewVtxFitDistToSeedFail, "newVtxFit_dist_toSeed_fail",       100, 0.0, 10.0));

  ATH_CHECK(makeHist(m_histNewVtxFitProbCandOverSeed,           "newVtxFit_prob_candOverSeed",           100, 0.0,  4.0));
  ATH_CHECK(makeHist(m_histNewVtxFitProbCandOverSeedPass,      "newVtxFit_prob_candOverSeed_pass",      100, 0.0,  4.0));
  ATH_CHECK(makeHist(m_histNewVtxFitProbCandOverSeedFail,      "newVtxFit_prob_candOverSeed_fail",      100, 0.0,  4.0));
  ATH_CHECK(makeHist(m_histNewVtxFitProbCandOverSeed3Trk,      "newVtxFit_prob_candOverSeed_3trk",      100, 0.0,  4.0));
  ATH_CHECK(makeHist(m_histNewVtxFitProbCandOverSeed3TrkPass, "newVtxFit_prob_candOverSeed_3trk_pass", 100, 0.0,  4.0));

  ATH_CHECK(makeHist(m_histVtx2TrkPairDist,      "Vtx2trkPair_dist",      100, 0.0, 100.0));
  ATH_CHECK(makeHist(m_histVtx2trkPairDistZoom, "Vtx2trkPair_dist_zoom", 100, 0.0,  10.0));
  ATH_CHECK(makeHist(m_histVtx2TrkPairSig1,      "Vtx2trkPair_sig1",      100, 0.0,  20.0));
  ATH_CHECK(makeHist(m_histVtx2TrkPairSig2,      "Vtx2trkPair_sig2",      100, 0.0,  20.0));

  ATH_CHECK(makeHist(m_histSelectedTrackCountAll,         "selectedTrack_CountAll",         25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histSelectedTrackCountMatch2Vtx ,  "selectedTrack_CountMatch2Vtx",   25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histSelectedTrackCountWithout2Vtx, "selectedTrack_CountWithout2Vtx", 25, -0.5, 24.5));

  ATH_CHECK(makeHist(m_histVtxWithoutLepton2TrkNTrack,        "vtxWithoutLepton2trk_NTrack",         25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histVtxWithoutLepton2TrkNPass,         "vtxWithoutLepton2trk_NPass",          25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histVtxWithoutLepton2TrkNPassUnmerged, "vtxWithoutLepton2trk_NPassUnmerged",  25, -0.5, 24.5));
  ATH_CHECK(makeHist(m_histVtxWithoutLepton2TrkNMerged,       "vtxWithoutLepton2trk_NMerged",        25, -0.5, 24.5));

  return StatusCode::SUCCESS;
}


//=============================================================================
Prompt::MergeResult Prompt::VertexIterativeFitMergingTool::mergeInitVertices(
  const FittingInput &input,
  const xAOD::TrackParticle *tracklep,
  std::vector<std::unique_ptr<xAOD::Vertex>> &initVtxs,
  const std::vector<const xAOD::TrackParticle *> &selectedTracks
)
{
  //
  // Merge initial (2-track) vertices into new merged vertices with three tracks or more
  //

  ATH_MSG_DEBUG("===========================================================================" << std::endl
             << name() << "::mergeInitVertices - start processing");


  MergeResult result;

  for(std::unique_ptr<xAOD::Vertex> &vtx: initVtxs) {
    if(passVertexSelection(vtx.get())) {
      result.vtxsInitPassed.push_back(std::move(vtx));
    }
  }

  const unsigned nvtxInit = initVtxs.size();
  const unsigned nvtxPass = result.vtxsInitPassed.size();

  fillTH1(m_histNvtx2TrkInit, nvtxInit);

  //---------------------------------------------------------------------------------------------
  // Find tracks that do not form 2-track vertex with lepton track
  //

  ATH_MSG_DEBUG(name() << "::mergeInitVertices - processes vertexes without lepton");


  std::vector<const xAOD::TrackParticle *> tracksWithoutVertex = getTracksWithoutVertex(
    result.vtxsInitPassed, selectedTracks
  );

  //
  // Rank the tracks by pT, will keep the top `m_maxExtraTracks` tracks for fitting vertexes without lepton
  //
  if(tracksWithoutVertex.size() > m_maxExtraTracks) {
    ATH_MSG_DEBUG(" number of tracks without good lepton+track vertex: " << tracksWithoutVertex.size() << std::endl
        << " will only keep the top " << m_maxExtraTracks << " tracks");

    std::sort(tracksWithoutVertex.begin(), tracksWithoutVertex.end(), Prompt::SortByIDTrackPt());

    tracksWithoutVertex.erase(tracksWithoutVertex.begin() + m_maxExtraTracks, tracksWithoutVertex.end());
  }


  MergeResult result_extra;
  result_extra.vtxsInitPassed = fit2TrackVertexes(input, tracksWithoutVertex, Prompt::kTwoTrackVtxWithoutLepton);


  ATH_MSG_DEBUG(
    name() << "::mergeInitVertices - will merge vertexes without lepton" << std::endl
    << "   number of selected tracks without good lepton+track vertex: "
    << tracksWithoutVertex        .size() << std::endl
    << "   number of 2-track vertexes without lepton:                  "
    << result_extra.vtxsInitPassed.size()
  );

  //
  // Merge 2-track vertex without lepton only when there are two or more vertices
  //
  ATH_MSG_WARNING("mergeIteratively2TrackVtxs has been blocked! Memory management needs to be fixed");
  // mergeIteratively2TrackVtxs(input, result_extra, Prompt::kIterativeFitVtxWithoutLepton);

  if(result_extra.vtxsInitPassed.size() > 1) {
    //
    // Fill histograms only when there are two or more vertices
    //
    fillTH1(m_histVtxWithoutLepton2TrkNTrack,        tracksWithoutVertex.size());
    fillTH1(m_histVtxWithoutLepton2TrkNPass,         result_extra.vtxsInitPassed           .size());
    fillTH1(m_histVtxWithoutLepton2TrkNPassUnmerged, result_extra.vtxsInitPassedNotMerged.size());
    fillTH1(m_histVtxWithoutLepton2TrkNMerged,       result_extra.vtxsNewMerged            .size());
  }


  ATH_MSG_DEBUG(name() << "::mergeInitVertices - finished merging vertexes without lepton" << std::endl
      << "   number of tracks without good lepton+track vertex:  " << tracksWithoutVertex                   .size() << std::endl
      << "   number of          2-track vertexes without lepton: " << result_extra.vtxsInitPassed           .size() << std::endl
      << "   number of unmerged 2-track vertexes without lepton: " << result_extra.vtxsInitPassedNotMerged.size() << std::endl
      << "   number of merged  vertexes without lepton:          " << result_extra.vtxsNewMerged            .size());


  //---------------------------------------------------------------------------------------------
  // Next, processes 2-track vertexes that contain lepton track
  //

  ATH_MSG_DEBUG("===========================================================================" << std::endl
      << name() << "::mergeInitVertices - process 2-track vertexes with lepton" << std::endl
      << "   lepton track pT="                      << tracklep->pt()                 << std::endl
      << "   number of initial  2-track vertices: " << initVtxs.size()               << std::endl
      << "   number of selected 2-track vertices: " << result.vtxsInitPassed.size() << std::endl
      << "   number of selected ID tracks:        " << selectedTracks.size() );


  //
  // Merge 2-track vertexes that contain lepton track
  //
  ATH_MSG_WARNING("mergeIteratively2TrackVtxs has been blocked! Memory management needs to be fixed");
  // mergeIteratively2TrackVtxs(input, result, Prompt::kIterativeFitVtx);

  if(result.vtxsInitPassed.size() > 1) {
    //
    // Fill histograms only when there are at least two vertexes to merge
    //
    fillTH1(m_histNvtx2TrkPass,     nvtxPass);
    fillTH1(m_histNvtx2TrkUnmerged, result.vtxsInitPassedNotMerged.size());
    fillTH1(m_histNvtxMerged,        result.vtxsNewMerged            .size());
  }

  //
  // Erase vector of 2-track vertices without lepton that were merged
  //
  result_extra.vtxsInitPassed.clear();

  //---------------------------------------------------------------------------------------------
  // Add vertices without lepton track to result
  //
  result.vtxsInitPassedNotMerged.insert(result.vtxsInitPassedNotMerged.end(),
      std::make_move_iterator(result_extra.vtxsInitPassedNotMerged.begin()),
      std::make_move_iterator(result_extra.vtxsInitPassedNotMerged.end()));

  result.vtxsNewMerged.insert(result.vtxsNewMerged.end(),
      std::make_move_iterator(result_extra.vtxsNewMerged.begin()),
      std::make_move_iterator(result_extra.vtxsNewMerged.end()));


  ATH_MSG_DEBUG("   number of initial  2-track vertices: " << initVtxs                         .size() << std::endl
      << "   number of passed   2-track vertices: " << result.vtxsInitPassed           .size() << std::endl
      << "   number of unmerged 2-track vertices: " << result.vtxsInitPassedNotMerged.size() << std::endl
      << "   number of merged           vertices: " << result.vtxsNewMerged            .size() << std::endl
      << std::endl
      << "   number of tracks without good lepton+track vertex:  " << tracksWithoutVertex                   .size() << std::endl
      << "   number of          2-track vertexes without lepton: " << result_extra.vtxsInitPassed           .size() << std::endl
      << "   number of unmerged 2-track vertexes without lepton: " << result_extra.vtxsInitPassedNotMerged.size() << std::endl
      << "   number of merged  vertexes without lepton:          " << result_extra.vtxsNewMerged            .size() << std::endl
      << name() << "::mergeInitVertices - ALL DONE" << std::endl
      << "===========================================================================" );


  return result;
}

// TODO: fix memory management in mergeIteratively2TrackVtxs
//=============================================================================
bool Prompt::VertexIterativeFitMergingTool::mergeIteratively2TrackVtxs(
  const FittingInput &input,
  MergeResult &result,
  const VtxType vtxType
)
{
  /*
     This function merges iterively vertexes with this algorithm:
     o) Sort 2-track vertexes by sum of track pT
     o) Select the vertex with the highest sum of track pT as the seed vertex
     - Sort all other vertexes by the distance to the seed vertex
     - Add tracks from the closest vertex to the selected vertex
     - Fit new vertex:
     -- if the new vertex passes cuts, select as the new seed vertex
     -- if the new vertex fails cuts, continue with the original seed vertex
     -- Remove this closest vertex from the list
     - Resort remaining tracks by the distance to the seed vertex and repeat
     o) Remove the 2-track vertexes that were merged from the global list and repeat
     */

  //
  // Only 0 or 1 2-track vertices - add the vertex to not merged list and return
  //
  if(result.vtxsInitPassed.size() < 2) {
    result.vtxsInitPassedNotMerged = std::move(result.vtxsInitPassed);


    ATH_MSG_DEBUG(name() << "::mergeIteratively2TrackVtxs - too few vertexes: nothing more to do");


    return false;
  }

  //
  // Make 2-track vertex data structures
  //
  std::vector<TwoTrackVtx> vtxs2Track;

  for(std::unique_ptr<xAOD::Vertex> &vtx: result.vtxsInitPassed) {
    if(vtx->nTrackParticles() != 2) {
      ATH_MSG_WARNING("mergeIteratively2TrackVtxs - wrong number of tracks: " << vtx->nTrackParticles());
      continue;
    }

    if(vtx->nTrackParticles() !=2 ) {
      ATH_MSG_WARNING("mergeIteratively2TrackVtxs - vertex does not contain 2 TrackParticles: ntrack=" << vtx->nTrackParticles());
      continue;
    }

    TwoTrackVtx vtx2track;
    vtx2track.trackId0 = vtx->trackParticle(0);
    vtx2track.trackId1 = vtx->trackParticle(1);

    if(!vtx2track.trackId0 || !vtx2track.trackId1) {
      ATH_MSG_WARNING("mergeIteratively2TrackVtxs - failed to find TrackParticles for 2-track vertex");
      continue;
    }

    vtx2track.vertex          = std::move(vtx);
    vtx2track.vertexFitProb = Prompt::getVertexFitProb(vtx.get());
    vtx2track.sumTrackPt    = vtx2track.trackId0->pt() + vtx2track.trackId1->pt();

    vtxs2Track.push_back(std::move(vtx2track));
  }


  ATH_MSG_DEBUG(name() << "::mergeIteratively2TrackVtxs - start processing with " << vtxs2Track.size() << " input vertexes ");


  if(vtxs2Track.size() < 2) {
    ATH_MSG_WARNING("mergeIteratively2TrackVtxs - logic error: found only " << vtxs2Track.size() << " 2-track vertex");
    return false;
  }

  //
  // Sort 2-track vertexes by ID track pT
  //
  std::sort(vtxs2Track.begin(), vtxs2Track.end(), Prompt::SortTwoTrackVtxBySumTrackPt());


  ATH_MSG_DEBUG(name() << "::mergeIteratively2TrackVtxs - number of 2 track passed vertexes=" << vtxs2Track.size());

  for(const TwoTrackVtx &vtx: vtxs2Track) {
    ATH_MSG_DEBUG("Input vertex with 2 tracks sum pT=" << vtx.sumTrackPt << "\n    " << vtxAsStr(vtx.vertex.get(), true));
    }


    //
    // Plot distances between all unique pairs of 2-track vertexes
    //
    plotVertexDistances(vtxs2Track);

    //
    // Iterative fit vertices for merging:
    //
    std::vector<TwoTrackVtx>::iterator currVit = vtxs2Track.begin();

    while(currVit != vtxs2Track.end()) {
      //
      // Seed new vertex with 2-track vertex containing highest pT ID track (non-lepton track)
      //
      const double currSumTrackPt = currVit->sumTrackPt;
      const double currTrack0Pt    = currVit->trackId0->pt();
      const double currTrack1Pt    = currVit->trackId1->pt();


      ATH_MSG_DEBUG("*************************************************************************** START NEW" << std::endl
          << "Curr sum track pT=" << currSumTrackPt
          << ", track0 pT =" << currTrack0Pt
          << ", track1 pT =" << currTrack1Pt );


    //
    // Select 2-track vertex other than the seed vertex itself
    //
    std::vector<TwoTrackVtx> others;

    for(std::vector<TwoTrackVtx>::iterator vit = vtxs2Track.begin(); vit != vtxs2Track.end(); vit++) {
      if(vit != currVit) {
        others.push_back(std::move(*vit));
      }
    }

    //
    // Sort other vertices by distance to the seed vertex
    //
    std::unique_ptr<xAOD::Vertex> seedVtx = std::move(currVit->vertex);

    std::sort(others.begin(), others.end(), Prompt::SortTwoTrackVtxByDistToSeed(seedVtx.get()));

    for(std::vector<TwoTrackVtx>::iterator vit = others.begin(); vit != others.end(); vit++) {
      const double dist = Prompt::getDistance(seedVtx.get(), vit->vertex.get());
      const double sig1 = Prompt::getNormDist(seedVtx->position(), vit->vertex->position(), seedVtx   ->covariance(), msg(MSG::WARNING));
      const double sig2 = Prompt::getNormDist(seedVtx->position(), vit->vertex->position(), vit->vertex->covariance(), msg(MSG::WARNING));


      ATH_MSG_DEBUG("    other: track0 pT=" << vit->trackId0->pt() << ", track1 pT=" << vit->trackId1->pt()
            << ", dist=" << dist
            << ", sig1=" << sig1
            << ", sig2=" << sig2 );

    }

    //
    // Call recursive function to fit seed+closest vertex pairs
    //
    std::unique_ptr<xAOD::Vertex> newMergedVtx = fitSeedVertexCluster(
      input, std::move(seedVtx), vtxType, others
    );

    if(!newMergedVtx) {
      ATH_MSG_ERROR("mergeIteratively2TrackVtxs - logic error: null vertex pointer is removed by fitSeedVertexCluster function");
      currVit++;
      continue;
    }

    if(newMergedVtx != seedVtx) {
      //
      // Remove 2-track vertexes that were merged
      //
      removeMerged2TrackVertexes(newMergedVtx.get(), vtxs2Track);

      //
      // Reset current vertex iterator to beginning
      //
      currVit = vtxs2Track.begin();


      ATH_MSG_DEBUG(name() << "::mergeIteratively2TrackVtxs - new merged vertex:\n" << vtxAsStr(newMergedVtx.get(), false));

      //
      // Save new merged vertex (it is new because it is distinct from seed vertex)
      //
      result.vtxsNewMerged.push_back(std::move(newMergedVtx));
    }
    else {
      //
      // This vertex could not be merged - try next one
      //
      currVit++;


      ATH_MSG_DEBUG(name() << "::mergeIteratively2TrackVtxs - could not merge 2-track vertex:\n" << vtxAsStr(seedVtx.get(), false));

    }


    ATH_MSG_DEBUG("Done with sum track pT=" << currSumTrackPt
       << ", track0 pT =" << currTrack0Pt
       << ", track1 pT =" << currTrack1Pt << std::endl
       << "***************************************************************************");

  }

  //
  // Record unmerged two track vertexes
  //

  for(TwoTrackVtx &vtx: vtxs2Track) {
    result.vtxsInitPassedNotMerged.push_back(std::move(vtx.vertex));


    ATH_MSG_DEBUG("Unmerged " << vtxAsStr(vtx.vertex.get(), true));

  }

  ATH_MSG_DEBUG(name() << "::mergeIteratively2TrackVtxs - finished processing:" << std::endl
  << "   number of unmerged 2-track vertexes=" << vtxs2Track           .size() << std::endl
  << "   number of merged           vertexes=" << result.vtxsNewMerged.size() );

  for(TwoTrackVtx &vtx: vtxs2Track) {
    ATH_MSG_DEBUG("Unmerged " << vtxAsStr(vtx.vertex.get(), true));
  }

  for(const std::unique_ptr<xAOD::Vertex>& vtx: result.vtxsNewMerged) {
    ATH_MSG_DEBUG("Merged " << vtxAsStr(vtx.get(), true));
  }

  return true;
}

//=============================================================================
std::unique_ptr<xAOD::Vertex> Prompt::VertexIterativeFitMergingTool::fitSeedVertexCluster(
  const FittingInput &input,
  std::unique_ptr<xAOD::Vertex> seedVtx,
  const VtxType vtxType,
  std::vector<TwoTrackVtx> &others
)
{
  //
  // Reached end of the recursive loop
  //
  if(others.empty()) {
    return seedVtx;
  }

  //
  // Re-sort other vertices by distance to the seed vertex - needed because seed position changes when vertices are merged
  //
  std::sort(others.begin(), others.end(), Prompt::SortTwoTrackVtxByDistToSeed(seedVtx.get()));

  //
  // Take closest vertex
  //
  std::unique_ptr<xAOD::Vertex> currVtx = std::move(others.front().vertex);

  if(!currVtx) {
    ATH_MSG_WARNING("VertexIterativeFitMergingTool::fitSeedVertexCluster - current vertex is null pointer");
    return seedVtx;
  }

  //
  // Remove closest vertex from the list
  //
  others.erase(others.begin());

  //
  // Found nearby vertex - fit merged vertex
  //
  std::unique_ptr<xAOD::Vertex> candVtx = fitSeedPlusOtherVertex(
    input, seedVtx.get(), currVtx.get(), vtxType
  );

  if(!candVtx) {
    //
    // Failed to fit new vertex - continue with current seed vertex
    //

    ATH_MSG_DEBUG("fitSeedVertexCluster - NEW MERGED VERTEX FIT FAILED" << std::endl
          << "---------------------------------------------------------------------------");

    return fitSeedVertexCluster(input, std::move(seedVtx), vtxType, others);
  }

  const double probCand = getVertexFitProb(candVtx.get());
  const double probSeed = getVertexFitProb(seedVtx.get());

  double probCandOverSeed = -1.0;

  if(probSeed > 0.0) {
    probCandOverSeed = probCand/probSeed;
  }

  const double distToSeed = getDistance(seedVtx.get(), candVtx.get());
  const double distToCurr = getDistance(currVtx.get(), candVtx.get());

  fillTH1(m_histNewVtxFitChi2, candVtx->chiSquared());
  fillTH1(m_histNewVtxFitProb, probCand);

  fillTH1(m_histNewVtxFitDistToSeed,       distToSeed);
  fillTH1(m_histNewVtxFitDistToCurr,       distToCurr);
  fillTH1(m_histNewVtxFitProbCandOverSeed, probCandOverSeed);

  if(seedVtx->nTrackParticles() > 2) {
    fillTH1(m_histNewVtxFitProbCandOverSeed3Trk, probCandOverSeed);
  }

  std::stringstream str;


    str << "   dist to seed=" << distToSeed << ", probCandOverSeed=" << probCandOverSeed << std::endl
  << "   seed: "        << vtxAsStr(seedVtx.get(), false) << std::endl
  << "   curr: "        << vtxAsStr(currVtx.get(), true)
  << "   cand: "        << vtxAsStr(candVtx.get(), true)
  << "fitSeedVertexCluster - finished" << std::endl
  << "---------------------------------------------------------------------------" << std::endl;



  if(!(passVertexSelection(candVtx.get()) && probCandOverSeed > m_minCandOverSeedFitProbRatio)) {
    //
    // New fitted merged vertex failed selection
    //

    ATH_MSG_DEBUG("fitSeedVertexCluster - FAIL NEW MERGED VERTEX\n" << str.str());


    fillTH1(m_histNewVtxFitDistToSeedFail,       distToSeed);
    fillTH1(m_histNewVtxFitProbCandOverSeedFail, probCandOverSeed);

    //
    // Continue with current seed vertex
    //
    return fitSeedVertexCluster(input, std::move(seedVtx), vtxType, others);
  }

  fillTH1(m_histNewVtxFitDistToSeedPass,       distToSeed);
  fillTH1(m_histNewVtxFitProbCandOverSeedPass, probCandOverSeed);

  if(seedVtx->nTrackParticles() > 2) {
    fillTH1(m_histNewVtxFitProbCandOverSeed3TrkPass, probCand/probSeed);
  }

  //
  // Succesfully fitted new vertex
  //

  ATH_MSG_DEBUG("fitSeedVertexCluster - PASS NEW MERGED VERTEX" << str.str());


  return fitSeedVertexCluster(input, std::move(candVtx), vtxType, others);
}

//=============================================================================
unsigned Prompt::VertexIterativeFitMergingTool::removeMerged2TrackVertexes(
  const xAOD::Vertex *mergedVtx,
  std::vector<TwoTrackVtx> &vtxs
)
{
  //
  // Remove 2-tracks that are succesfully merged into one vertex
  //
  if(!mergedVtx) {
    ATH_MSG_WARNING("VertexIterativeFitMergingTool::removeMerged2TrackVertexes - merged vertex is null pointer");
    return 0;
  }

  unsigned icount = 0;
  std::vector<TwoTrackVtx>::iterator vit = vtxs.begin();

  while(vit != vtxs.end()) {
    int iCountMatchedTrack = 0;

    for(unsigned k = 0; k < mergedVtx->nTrackParticles(); ++k) {
      const xAOD::TrackParticle *track = mergedVtx->trackParticle(k);

      if(!track) {
        ATH_MSG_WARNING("removeMerged2TrackVertexes - merged vertex contains null TrackParticle pointer");
        continue;
      }

      if(vit->trackId0 == track) { iCountMatchedTrack++; }
      if(vit->trackId1 == track) { iCountMatchedTrack++; }
    }

    if(iCountMatchedTrack == 2) {
      //
      // Found 2-track vertex that was merged - remove this vertex
      //
      vit = vtxs.erase(vit);
      icount++;

      ATH_MSG_DEBUG("removeMerged2TrackVertexes - removed merged 2-track vertex");
    }
    else {
      vit++;

      ATH_MSG_DEBUG("removeMerged2TrackVertexes - skip unmerged 2-track vertex");
    }
  }


  ATH_MSG_DEBUG(name() << "::removeMerged2TrackVertexes - merged vertex ntrack=" << mergedVtx->nTrackParticles()
   << ", removed " << icount << " merged 2-track vertexes");


  return icount;
}

//=============================================================================
void Prompt::VertexIterativeFitMergingTool::plotVertexDistances(const std::vector<TwoTrackVtx> &others)
{
  for(std::vector<TwoTrackVtx>::const_iterator fit = others.begin(); fit != others.end(); fit++) {
    for(std::vector<TwoTrackVtx>::const_iterator sit = fit+1; sit != others.end(); sit++) {
      const double dist = Prompt::getDistance(fit->vertex.get(), sit->vertex.get());
      const double sig1 = Prompt::getNormDist(fit->vertex->position(), sit->vertex->position(), fit->vertex->covariance(), msg(MSG::WARNING));
      const double sig2 = Prompt::getNormDist(fit->vertex->position(), sit->vertex->position(), sit->vertex->covariance(), msg(MSG::WARNING));

      fillTH1(m_histVtx2TrkPairDist,      dist);
      fillTH1(m_histVtx2trkPairDistZoom, dist);
      fillTH1(m_histVtx2TrkPairSig1,      sig1);
      fillTH1(m_histVtx2TrkPairSig2,      sig2);
    }
  }
}

//=============================================================================
std::vector<const xAOD::TrackParticle *> Prompt::VertexIterativeFitMergingTool::getTracksWithoutVertex(
  const std::vector<std::unique_ptr<xAOD::Vertex>> &passVtxs,
  const std::vector<const xAOD::TrackParticle *> &selectedTracks
)
{
  //
  // Plot unmatched tracks
  //
  std::vector<const xAOD::TrackParticle *> tracksWithoutVertex;

  unsigned iCountDoMatch = 0;

  for(const xAOD::TrackParticle *track: selectedTracks) {
    bool match = false;

    for(const std::unique_ptr<xAOD::Vertex> &vtx: passVtxs) {
      for(unsigned k = 0; k < vtx->nTrackParticles(); ++k) {
        const xAOD::TrackParticle *vtxTrack = vtx->trackParticle(k);

        if(vtxTrack == track) {
          match = true;
          break;
        }
      }
    }

    if(match) {
      iCountDoMatch++;
    }
    else {
      tracksWithoutVertex.push_back(track);
    }
  }

  fillTH1(m_histSelectedTrackCountAll,         selectedTracks.size());
  fillTH1(m_histSelectedTrackCountMatch2Vtx,   iCountDoMatch);
  fillTH1(m_histSelectedTrackCountWithout2Vtx, tracksWithoutVertex.size());

  return tracksWithoutVertex;
}

//=============================================================================
bool Prompt::VertexIterativeFitMergingTool::passVertexSelection(const xAOD::Vertex *vtx) const
{
  //
  // Check whether vertex passes quality cuts
  //
  if(!vtx) {
    ATH_MSG_WARNING("passVertexSelection - input vertex is null pointer");
    return false;
  }

  if(!(vtx->numberDoF() > 0 && vtx->chiSquared() >= 0)) {
    return false;
  }

  const double fitProb = Prompt::getVertexFitProb(vtx);

  ATH_MSG_DEBUG("passVertexSelection - vertex pointer=" << vtx << " chi2/ndof=" << vtx->chiSquared() << "/" << vtx->numberDoF() << ", prob=" << fitProb);

  return fitProb > m_minFitProb;
}

//=============================================================================
std::unique_ptr<xAOD::Vertex> Prompt::VertexIterativeFitMergingTool::fitSeedPlusOtherVertex(
  const FittingInput &input,
  const xAOD::Vertex *seedVtx,
  const xAOD::Vertex *otherVtx,
  const VtxType vtxType
)
{
  //
  // Fit two 2-track vertexes
  //
  if(!seedVtx) {
    ATH_MSG_WARNING("fitSeedPlusOtherVertex - null seed Vertex pointer");
    return 0;
  }

  if(!otherVtx) {
    ATH_MSG_WARNING("fitSeedPlusOtherVertex - null other Vertex pointer");
    return 0;
  }

  if(otherVtx->nTrackParticles() != 2) {
    ATH_MSG_WARNING("fitSeedPlusOtherVertex - other Vertex does not have 2 tracks: ntrack=" << otherVtx->nTrackParticles());
    return 0;
  }

  //
  // Collect tracks from the seed vertex
  //
  std::vector<const xAOD::TrackParticle *> tracks;

  for(unsigned k = 0; k < seedVtx->nTrackParticles(); ++k) {
    const xAOD::TrackParticle *track = seedVtx->trackParticle(k);

    if(track) {
      tracks.push_back(track);
    }
    else {
      ATH_MSG_WARNING("fitSeedPlusOtherVertex - seed vertex contains TrackParticle null pointer");
    }
  }

  //
  // Collect tracks from other vertices
  //
  for(unsigned k = 0; k < otherVtx->nTrackParticles(); ++k) {
    const xAOD::TrackParticle *track = otherVtx->trackParticle(k);

    if(track) {
      tracks.push_back(track);
    }
    else {
      ATH_MSG_WARNING("fitSeedPlusOtherVertex - other vertex contains TrackParticle null pointer");
    }
  }

  //
  // Fit new vertex
  //
  std::unique_ptr<xAOD::Vertex> secVtx = m_vertexFitterTool->fitVertexWithSeed(
    input, tracks, seedVtx->position(), vtxType
  );

  if(!secVtx) {
    ATH_MSG_WARNING("fitSeedPlusOtherVertex - failed to fit vertex");
    return 0;
  }

  return secVtx;
}

//=============================================================================
std::vector<std::unique_ptr<xAOD::Vertex>> Prompt::VertexIterativeFitMergingTool::fit2TrackVertexes(
  const FittingInput &input,
  std::vector<const xAOD::TrackParticle *> &selectedTracks,
  const VtxType vtxType
)
{
  //
  // Fit all possible combinations of two 2-track vertexes
  //
  std::vector<std::unique_ptr<xAOD::Vertex>> passVtxs;

  if(selectedTracks.size() < 2) {
    ATH_MSG_DEBUG("fit2TrackVertexeses - 0 or 1 input tracks - nothing to do");
    return passVtxs;
  }


  ATH_MSG_DEBUG(name() << "::fit2TrackVertexes - start with " << selectedTracks.size() << " tracks");


  //
  // Sort tracks by decreasing pT
  //
  std::sort(selectedTracks.begin(), selectedTracks.end(), SortTracksByPt());

  unsigned icount = 0;

  for(std::vector<const xAOD::TrackParticle *>::const_iterator it1 = selectedTracks.begin(); it1 != selectedTracks.end(); ++it1) {
    for(std::vector<const xAOD::TrackParticle *>::const_iterator it2 = it1 + 1; it2 != selectedTracks.end(); ++it2) {
      const xAOD::TrackParticle *track1 = *it1;
      const xAOD::TrackParticle *track2 = *it2;


      if(!track1 || !track2) {
        ATH_MSG_WARNING("fit2TrackVertexeses - logic error: TrackParticle null pointer");
        continue;
      }

      std::vector<const xAOD::TrackParticle *> fit_tracks = {track1, track2};

      //
      // Fit new vertex
      //
      std::unique_ptr<xAOD::Vertex> vtx = m_vertexFitterTool->fitVertexWithPrimarySeed(
        input, fit_tracks, vtxType
      );

      icount++;

      if(!vtx) {
        ATH_MSG_WARNING("fit2TrackVertexeses - failed to fit vertex");
        continue;
      }

      if(passVertexSelection(vtx.get())) {
        passVtxs.push_back(std::move(vtx));


        ATH_MSG_DEBUG("fit2TrackVertexeses - pass vertex: " << vtxAsStr(vtx.get(), true));

      }
    }
  }


  ATH_MSG_DEBUG(name() << "::fit2TrackVertexes - finished processing: " << std::endl
        << "   number of input tracks:            " << selectedTracks.size() << std::endl
        << "   number of 2-track combinations:    " << icount                 << std::endl
        << "   number of passed 2-track vertexes: " << passVtxs      .size() << std::endl
        << name() << "::fit2TrackVertexes - all is done" );


  return passVtxs;
}

//=============================================================================
StatusCode Prompt::VertexIterativeFitMergingTool::makeHist(TH1 *&h, const std::string &key, int nbin, double xmin, double xmax)
{
  //
  // Initiliase histogram pointer. If configured to run in validation mode, then create and register histogram
  //
  h = 0;

  if(m_outputStream.empty() || key.empty()) {
    return StatusCode::SUCCESS;
  }

  const std::string hname    = name() + "_" + key;
  const std::string hist_key = "/"+m_outputStream+"/"+hname;

  h = new TH1D(hname.c_str(), hname.c_str(), nbin, xmin, xmax);
  h->SetDirectory(0);

  return m_histSvc->regHist(hist_key, h);
}
