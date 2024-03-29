/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef DETAILEDTRACKTRUTHBUILDER_H
#define DETAILEDTRACKTRUTHBUILDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "AthLinks/ElementLink.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "TrkEventUtils/InverseMultiMap.h"

#include "TrkToolInterfaces/IDetailedTrackTruthBuilder.h"
#include "TrkToolInterfaces/ITruthTrajectoryBuilder.h"
#include "TrkTruthData/DetailedTrackTruth.h"
#include "TrkTruthData/SubDetHitStatistics.h"

namespace Trk {
  
  class DetailedTrackTruthBuilder final: virtual public IDetailedTrackTruthBuilder,
				   public AthAlgTool
  {
  public:
    DetailedTrackTruthBuilder(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize();

    /** See description for IDetailedTrackTruthBuilder::buildDetailedTrackTruth() */
    virtual void buildDetailedTrackTruth(DetailedTrackTruthCollection *output,
					 const TrackCollection& tracks,
					 const std::vector<const PRD_MultiTruthCollection*>& prdTruth) const;

  private:
    typedef InverseMultiMap<PRD_MultiTruthCollection> PRD_InverseTruth;

    const AtlasDetectorID *m_idHelper;

    ToolHandle<Trk::ITruthTrajectoryBuilder> m_truthTrajBuilder;

    SubDetHitStatistics::SubDetType findSubDetType(const Identifier& id) const;
    
    void addTrack(DetailedTrackTruthCollection *output,
		  const ElementLink<DataVector<Trk::Track> > &track,
		  const std::vector<const PRD_MultiTruthCollection*>& orderedPRD_Truth,
		  const PRD_InverseTruth& inverseTruth) const;

    static void makeTruthToRecMap( PRD_InverseTruth& result, const PRD_MultiTruthCollection& rec2truth) ;

    SubDetHitStatistics countPRDsOnTruth(const TruthTrajectory& traj,
                                         const PRD_InverseTruth& inverseTruth) const;

  };
  
} // end namespace Trk

#endif/*DETAILEDTRACKTRUTHBUILDER_H*/
