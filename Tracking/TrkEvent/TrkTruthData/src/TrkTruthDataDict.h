/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKTRUTHDATA_TRKTRUTHDATADICT_H
#define TRKTRUTHDATA_TRKTRUTHDATADICT_H

#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTruthData/TrackTruthKey.h"
#include "TrkTruthData/TrackTruth.h"
#include "TrkTruthData/DetailedTrackTruth.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "TrkTruthData/DetailedTrackTruthCollection.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "AthLinks/DataLink.h"
#include <utility>


// Helpers for use from python.
namespace TrkTruthDataHelpers {

  
std::vector<std::pair<Identifier, HepMcParticleLink> >
getData (const PRD_MultiTruthCollection& prd)
{
  std::vector<std::pair<Identifier, HepMcParticleLink> > v;
  v.reserve (prd.size());
  for (const auto& p : prd) {
    v.emplace_back (p.first, p.second);
  }
  return v;
}


} // namespace TrkTruthDataHelpers

 
namespace 
{
  struct temp_TrkTruthData
  {
    std::pair<Trk::TrackTruthKey,TrackTruth> m_2;
    std::pair<Trk::TrackTruthKey, DetailedTrackTruth > m_4;    
    std::pair<const Trk::TrackTruthKey, DetailedTrackTruth > m_4c;
  };
}

#endif
