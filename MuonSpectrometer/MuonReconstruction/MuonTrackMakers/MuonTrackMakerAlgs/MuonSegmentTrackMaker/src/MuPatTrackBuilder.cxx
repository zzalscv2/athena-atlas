/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "MuPatTrackBuilder.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"

#include "StoreGate/DataHandle.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "MuonSegment/MuonSegment.h"

#include "TrkTrack/TrackStateOnSurface.h"
#include "Particle/TrackParticleContainer.h"
#include <vector>

using namespace Muon;

StatusCode MuPatTrackBuilder::initialize()
{
  if (m_trackMaker.retrieve().isFailure()){
    msg(MSG::FATAL) <<"Could not get " << m_trackMaker <<endmsg; 
    return StatusCode::FAILURE;
  }
  if (m_edmHelperSvc.retrieve().isFailure()){
    msg(MSG::FATAL) <<"Could not get " << m_edmHelperSvc <<endmsg; 
    return StatusCode::FAILURE;
  }
  if( msgLvl(MSG::DEBUG) ) msg(MSG::DEBUG) << "Retrieved " << m_trackMaker << endmsg;
  
  ATH_CHECK( m_segmentKey.initialize() );
  ATH_CHECK( m_spectroTrackKey.initialize() );

  if ( not m_monTool.name().empty() ) {
    ATH_CHECK( m_monTool.retrieve() );
  }

  return StatusCode::SUCCESS; 
}

StatusCode MuPatTrackBuilder::execute()
{
  typedef std::vector<const Muon::MuonSegment*> MuonSegmentCollection;

  SG::ReadHandle<Trk::SegmentCollection> segmentColl (m_segmentKey);
  if (!segmentColl.isValid() ) {
    msg(MSG::WARNING) << "Could not find MuonSegmentCollection at " << segmentColl.name() <<endmsg;
    return StatusCode::RECOVERABLE;
  }
    
  if( !segmentColl.cptr() ) {
    msg(MSG::WARNING) << "Obtained zero pointer for MuonSegmentCollection at " << segmentColl.name() <<endmsg;
    return StatusCode::RECOVERABLE;
  }
      
  if( msgLvl(MSG::DEBUG) ) msg(MSG::DEBUG) << "Retrieved MuonSegmentCollection "  << segmentColl->size() << endmsg;

  MuonSegmentCollection msc;
  msc.reserve(segmentColl->size());
  for (unsigned int i=0;i<segmentColl->size();++i){
    if (!segmentColl->at(i)) continue;
    const Muon::MuonSegment * ms = dynamic_cast<const Muon::MuonSegment*>(segmentColl->at(i));
    if (ms) msc.push_back( ms );
  }

  if (msc.size() != segmentColl->size()){
    msg(MSG::WARNING) << "Input segment collection (size " << segmentColl->size() << ") and translated MuonSegment collection (size "
                      << msc.size() << ") are not the same size." << endmsg;
  }

  TrackCollection * newtracks = m_trackMaker->find(msc);
  if (!newtracks) newtracks = new TrackCollection();

  SG::WriteHandle<TrackCollection> spectroTracks(m_spectroTrackKey); 	  
  if (spectroTracks.record(std::unique_ptr<TrackCollection>(newtracks)).isFailure()){    
      ATH_MSG_WARNING( "New Track Container " << spectroTracks.name() << " could not be recorded in StoreGate !");
      return StatusCode::RECOVERABLE;
  }
  ATH_MSG_DEBUG ("TrackCollection '" << m_spectroTrackKey.key() << "' recorded in storegate, ntracks: " << newtracks->size());

  //---------------------------------------------------------------------------------------------------------------------//
  //------------                Monitoring of muon segments and tracks inside the trigger algs               ------------//
  //------------ Author:  Laurynas Mince                                                                     ------------//
  //------------ Created: 03.10.2019                                                                         ------------//
  //---------------------------------------------------------------------------------------------------------------------//

  // Only run monitoring for online algorithms
  if ( not m_monTool.name().empty() ) {
    std::vector<int>    ini_mstrksn(0);
    std::vector<double> ini_mstrkspt(0);
    std::vector<double> ini_mstrkseta(0);
    std::vector<double> ini_mstrksphi(0);
    auto mstrks_n     = Monitored::Collection("mstrks_n", ini_mstrksn);
    auto mstrks_pt    = Monitored::Collection("mstrks_pt", ini_mstrkspt);
    auto mstrks_eta   = Monitored::Collection("mstrks_eta", ini_mstrkseta);
    auto mstrks_phi   = Monitored::Collection("mstrks_phi", ini_mstrksphi);

    std::vector<int>    ini_mssegsn(0);
    std::vector<double> ini_mssegseta(0);
    std::vector<double> ini_mssegsphi(0);
    auto mssegs_n     = Monitored::Collection("mssegs_n", ini_mssegsn);
    auto mssegs_eta   = Monitored::Collection("mssegs_eta", ini_mssegseta);
    auto mssegs_phi   = Monitored::Collection("mssegs_phi", ini_mssegsphi);

    auto monitorIt = Monitored::Group(m_monTool, mstrks_n, mstrks_pt, mstrks_eta, mstrks_phi, mssegs_n, mssegs_eta, mssegs_phi);

    // MS-only extrapolated tracks
    int count_mstrks = 0;
    for (auto const& mstrk : *newtracks) {
      count_mstrks++;
      const Trk::Perigee* perigee = mstrk->perigeeParameters();
      const Amg::Vector3D mom = perigee->momentum();
      ini_mstrkspt.push_back(mom.perp()/1000.0); // Converted to GeV
      double theta = perigee->parameters()[Trk::theta];
      double eta = -log(tan(theta*0.5));
      ini_mstrkseta.push_back(eta);
      ini_mstrksphi.push_back(perigee->parameters()[Trk::phi0]);
    }
    ini_mstrksn.push_back(count_mstrks);

    int count_mssegs = 0;
    for (auto const& seg : msc) {
      count_mssegs++;
      ini_mssegseta.push_back(seg->globalDirection().eta());
      ini_mssegsphi.push_back(seg->globalDirection().phi());      
    }
    ini_mssegsn.push_back(count_mssegs);
  }



  return StatusCode::SUCCESS;
} // execute

