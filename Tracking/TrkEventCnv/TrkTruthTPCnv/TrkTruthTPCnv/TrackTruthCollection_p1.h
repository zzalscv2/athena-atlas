/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

// Andrei.Gaponenko@cern.ch, 2007


#ifndef TRACKTRUTHCOLLECTION_P1_H
#define TRACKTRUTHCOLLECTION_P1_H

#include "DataModelAthenaPool/DataLink_p1.h"
#include "TrkTrack/TrackCollection.h"
#include "GeneratorObjectsTPCnv/HepMcParticleLink_p1.h"

#include <vector>


namespace Trk {
  class TrackTruthCollection_p1 {
  public:

    DataLink_p1 m_trackCollectionLink;

    // Perhaps can use here a 32 bit unsigned instead of the 64 bit one?
    typedef TrackCollection::size_type size_type;
    
    struct Entry {
      size_type index;

      // Do TrackTruth here instead of introducing a separate converer for it.
      // TrackTruth::m_flag is not used, don't store it.
      float probability;
      HepMcParticleLink_p1 particle;
    };

    typedef std::vector<Entry> CollectionType;
    CollectionType m_entries;
  };
  
}

#endif/*TRACKTRUTHCOLLECTION_P1_H*/
