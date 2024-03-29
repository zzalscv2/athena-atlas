/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRKTRACKSCORINGTOOL_H
#define TRKTRACKSCORINGTOOL_H

#include "GaudiKernel/ToolHandle.h"
#include "TrkEventPrimitives/TrackScore.h"
#include "TrkToolInterfaces/ITrackScoringTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include <vector>

namespace Trk 
{

class Track;
class TrackSummary;

/**Concrete implementation of the ITrackScoringTool pABC*/
 class TrackScoringTool final: virtual public ITrackScoringTool, public AthAlgTool{

  public:
    TrackScoringTool(const std::string&,const std::string&,const IInterface*);
    virtual ~TrackScoringTool () = default;
    /** create a score based on how good the passed track is*/
    TrackScore score( const Track& track ) const override;

    /** create a score based on how good the passed TrackSummary is*/
    TrackScore simpleScore( const Track& track, const TrackSummary& trackSummary ) const override;

  private:
    /**holds the scores assigned to each Trk::SummaryType from the track's Trk::TrackSummary*/
    std::vector<TrackScore> m_summaryTypeScore;
  }; 
}
#endif 
