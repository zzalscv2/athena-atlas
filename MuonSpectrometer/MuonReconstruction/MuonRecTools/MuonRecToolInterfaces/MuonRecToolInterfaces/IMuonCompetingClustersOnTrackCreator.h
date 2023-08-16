/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_IMUONCOMPETINGCLUSTERSONTRACKCREATOR_H
#define MUON_IMUONCOMPETINGCLUSTERSONTRACKCREATOR_H

#include <list>
#include <memory>

#include "GaudiKernel/IAlgTool.h"

static const InterfaceID IID_IMuonCompetingClustersOnTrackCreator("Muon::IMuonCompetingClustersOnTrackCreator", 1, 0);

namespace Trk {
    class PrepRawData;
}

namespace Muon {

    class CompetingMuonClustersOnTrack;

    /** @brief Interface for tools creating CompetingMuonClustersOnTrack objects
     */
    class IMuonCompetingClustersOnTrackCreator : virtual public IAlgTool {
    public:
        static const InterfaceID& interfaceID();

        /** @brief method to create a CompetingMuonClustersOnTrack using the average position of the hits and
                   the distance between the clusters as error
            @param prds list of Trk::PrepRawData objects
            @param errorScaleFactor error scale factor
            @return a pointer to a new CompetingMuonClustersOnTrack, zero if creation failed.
                    The ownership of the new object is passed to the client calling the tool
        */
        virtual std::unique_ptr<CompetingMuonClustersOnTrack> 
        createBroadCluster(const std::list<const Trk::PrepRawData*>& prds,
            const double errorScaleFactor) const = 0;
    };

    inline const InterfaceID& IMuonCompetingClustersOnTrackCreator::interfaceID() { return IID_IMuonCompetingClustersOnTrackCreator; }
}  // namespace Muon

#endif  // MUON_IMUONCOMPETINGCLUSTERSONTRACKCREATOR_H
