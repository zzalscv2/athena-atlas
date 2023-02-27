/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuPatPrimitives/MuPatSegment.h"
#include "MuPatPrimitives/MuPatTrack.h"
#include <iostream>
namespace Muon {

    void MuPatSegment::addTrack(MuPatTrack* track) {
        m_associatedTracks.insert(track);
    }
    void MuPatSegment::removeTrack(MuPatTrack* track) {
        // look up track
        std::set<MuPatTrack*>::iterator pos = m_associatedTracks.find(track);
        if (pos != m_associatedTracks.end()) {
            // if found remove it from list
            m_associatedTracks.erase(pos);            
        }
        
    }

}  // namespace Muon
