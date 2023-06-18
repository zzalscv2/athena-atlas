// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimLRT/FPGATrackSimLLPRoadFilterTool.h"

FPGATrackSimLLPRoadFilterTool::FPGATrackSimLLPRoadFilterTool (const std::string& algname, const std::string& name, const IInterface* ifc) 
  : AthAlgTool(algname, name, ifc) {}


StatusCode FPGATrackSimLLPRoadFilterTool::filterUsedHits( std::vector<FPGATrackSimTrack> &tracks, 
                                              const std::vector<const FPGATrackSimHit*>& allHits, 
                                              std::vector<const FPGATrackSimHit*>& unusedHits ) {
    std::vector<FPGATrackSimHit> hitsInTracksVec;
    for (const auto& track : tracks) {
        for (const FPGATrackSimHit& hit : track.getFPGATrackSimHits()) {
            if (hit.isReal()) {
                hitsInTracksVec.push_back(hit);
            }
        }
    }
    std::set<const FPGATrackSimHit*, HitCompare > hitsInTracks;
    for ( auto& hit : hitsInTracksVec) {
        hitsInTracks.insert(&hit);
    }

    ATH_MSG_DEBUG("Number of hits from tracks " <<  hitsInTracks.size());

    std::set<const FPGATrackSimHit*, HitCompare> orderedHits;
    for ( auto hit: allHits){
        if ( hit->isReal()) {
            orderedHits.insert(hit);
        }
    }
    ATH_MSG_DEBUG("Number of all hits " << orderedHits.size() << " " << allHits.size() );


    std::set_difference( orderedHits.begin(), orderedHits.end(), 
                         hitsInTracks.begin(), hitsInTracks.end(), 
                         std::back_inserter(unusedHits), 
                         HitCompare() );

   return StatusCode::SUCCESS;
}
