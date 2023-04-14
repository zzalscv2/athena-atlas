/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_IMUONNSWSEGMENTFINDERTOOL_H
#define MUON_IMUONNSWSEGMENTFINDERTOOL_H

#include <vector>

#include "GaudiKernel/IAlgTool.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonSegment/MuonSegment.h"
#include "TrkSegment/SegmentCollection.h"


namespace Muon {

    class IMuonNSWSegmentFinderTool : virtual public IAlgTool {
    public:
        /** access to tool interface */
        static const InterfaceID& interfaceID() {
            static const InterfaceID IID_IMuonNSWSegmentFinderTool("Muon::IMuonNSWSegmentFinderTool", 1, 0);
            return IID_IMuonNSWSegmentFinderTool;
        }
        
        using MuonClusterPtr = std::unique_ptr<const Muon::MuonClusterOnTrack>;
        using MuonClusterVec = std::vector<MuonClusterPtr>;
        
        /// Helper struct to parse the data around
        struct SegmentMakingCache{
            /// Output vector to which the constructed segments are pushed_back
            std::vector<std::unique_ptr<Muon::MuonSegment>> constructedSegs{}; 
            /// Toggle whether quad segments should be built or not
            bool buildQuads{false};
            /// Output vector to which the quadruplet segments are pushed back
            std::vector<std::unique_ptr<Muon::MuonSegment>> quadSegs{};
            /// Input vector containing all muon cluster on track
            std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> inputClust{};
            /// Set of all hits used in the segment making
            std::set<Identifier> usedHits{};

        };
        virtual void find(const EventContext& ctx, SegmentMakingCache& cache) const = 0;

        virtual ~IMuonNSWSegmentFinderTool() = default;
    };

}  // namespace Muon

#endif
