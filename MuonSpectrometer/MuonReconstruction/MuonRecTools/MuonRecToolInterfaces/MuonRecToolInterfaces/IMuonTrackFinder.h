/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_IMUONTRACKFINDER_H
#define MUON_IMUONTRACKFINDER_H

#include <vector>

#include "GaudiKernel/IAlgTool.h"
#include "TrkTrack/TrackCollection.h"


namespace Muon {

    class MuonSegment;

    /** @brief The IMuonTrackFinder is a pure virtual interface for tools to find track in the muon system
        starting from a vector of segments

        The following interface is available.
        @code
          std::vector<const Trk::Track*>* find( const std::vector<const MuonSegment*>& segments );
        @endcode

    */
    class IMuonTrackFinder : virtual public IAlgTool {
    public:
        /** access to tool interface */
        static const InterfaceID& interfaceID() {
            static const InterfaceID IID_IMuonTrackFinder("Muon::IMuonTrackFinder", 1, 0);
            return IID_IMuonTrackFinder; 
        }

        /** @brief interface for tools to find track in the muon system starting from a vector of segments
            @param segments a vector of input segments
            @return a pointer to a vector of Trk::Track objects, zero if no tracks are found.
                    The ownership of the tracks is passed to the client calling the tool.

        */
        virtual std::unique_ptr<TrackCollection> find(const EventContext& ctx, const std::vector<const MuonSegment*>& segments) const = 0;
    };
   
}  // namespace Muon

#endif  // IMuonTrackFinder_H
