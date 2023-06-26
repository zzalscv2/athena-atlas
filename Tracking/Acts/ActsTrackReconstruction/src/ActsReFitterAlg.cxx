/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsReFitterAlg.h"

// ATHENA
#include "GaudiKernel/ListItem.h"

#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkTrackSummary/TrackSummary.h"

// STL
#include <memory>
#include <vector>
#include <fstream>
#include <string>

using namespace Acts::UnitLiterals;

ActsReFitterAlg::ActsReFitterAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator){}

StatusCode ActsReFitterAlg::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_actsFitter.retrieve());
  ATH_CHECK(m_trackName.initialize());
  ATH_CHECK(m_newTrackName.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ActsReFitterAlg::execute(const EventContext &ctx) const {

  ATH_MSG_DEBUG ("ActsReFitterAlg::execute()");
  SG::ReadHandle<TrackCollection> tracks (m_trackName, ctx);

  if (!tracks.isValid()){
    ATH_MSG_ERROR("Track collection named " << m_trackName.key() << " not found, exit ReFitTrack.");
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG ("Tracks collection '" << m_trackName.key() << "' retrieved from EventStore.");
  }

  // Prepare the output data with MultiTrajectory
  std::vector<std::unique_ptr<Trk::Track> > new_tracks;
  new_tracks.reserve((*tracks).size());

  // Perform the fit for each input track
  for (TrackCollection::const_iterator track  = (*tracks).begin(); track < (*tracks).end(); ++track){

    auto newtrack = m_actsFitter->fit(ctx, (**track));

    if (newtrack) {
      if (msgLvl(MSG::VERBOSE)) {
        msg(MSG::VERBOSE) << "ATLAS param : " << endmsg;
        msg(MSG::VERBOSE) << *((**track).perigeeParameters()) << endmsg;
        msg(MSG::VERBOSE) << *((**track).perigeeParameters()->covariance()) << endmsg;
        msg(MSG::VERBOSE) << "ACTS param : " << endmsg;
        msg(MSG::VERBOSE) << *(newtrack->perigeeParameters()) << endmsg;
        msg(MSG::VERBOSE) << *(newtrack->perigeeParameters()->covariance()) << endmsg;

        msg(MSG::VERBOSE) << "ATLAS INFO : " << endmsg;
        msg(MSG::VERBOSE) << *((**track).trackSummary()) << endmsg;
        msg(MSG::VERBOSE) << "ACTS INFO : " << endmsg;
        msg(MSG::VERBOSE) << *(newtrack->trackSummary()) << endmsg;
        msg(MSG::VERBOSE) << "==========================" << endmsg;
      }
      new_tracks.push_back(std::move(newtrack));
    }
    else if (msgLvl(MSG::DEBUG)) {  // newtrack might be equal to a nullptr
      msg(MSG::DEBUG) << "The Acts Refitting (KF or GSF) has returned a nullptr. Below is information on the offending track." << endmsg; // TODO: solve the cases where we return a nullptr
      msg(MSG::DEBUG) << "ATLAS param : " << endmsg;
      msg(MSG::DEBUG) << *((**track).perigeeParameters()) << endmsg;
      msg(MSG::DEBUG) << *((**track).perigeeParameters()->covariance()) << endmsg;

      msg(MSG::DEBUG) << "ATLAS INFO : " << endmsg;
      msg(MSG::DEBUG) << *((**track).trackSummary()) << endmsg;
      msg(MSG::DEBUG) << "==========================" << endmsg;
    }
  }
  
  // Create a new track collection with the refitted tracks
  std::unique_ptr<TrackCollection> new_track_collection = std::make_unique<TrackCollection>();
  
  new_track_collection->reserve(new_tracks.size());
  for(std::unique_ptr<Trk::Track> &new_track : new_tracks ) {
    new_track_collection->push_back(std::move(new_track));
  }
  
  ATH_MSG_DEBUG ("Saving tracks");
  ATH_CHECK(SG::WriteHandle<TrackCollection>(m_newTrackName, ctx).record(std::move(new_track_collection)));
  return StatusCode::SUCCESS;
}
