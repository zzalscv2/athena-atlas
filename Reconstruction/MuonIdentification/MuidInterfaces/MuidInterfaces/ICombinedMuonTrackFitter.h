/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ICombinedMuonTrackFitter
//  interface to build and fit a combined muon from input track(s)
//  and/or MeasurementSet, gathering material effects along the
//  track (in particular for the calorimeter).
//
///////////////////////////////////////////////////////////////////

#ifndef MUIDINTERFACES_ICOMBINEDMUONTRACKFITTER_H
#define MUIDINTERFACES_ICOMBINEDMUONTRACKFITTER_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkParameters/TrackParameters.h"

namespace Trk {
    class Track;
}  // namespace Trk

namespace Rec {

    /** Interface ID for ICombinedMuonTrackFitter*/

    /**@class ICombinedMuonTrackFitter

    Base class for CombinedMuonTrackBuilder AlgTool

    @author Alan.Poppleton@cern.ch
    */
    class ICombinedMuonTrackFitter : virtual public IAlgTool {
    public:
        /**Virtual destructor*/
        virtual ~ICombinedMuonTrackFitter() = default;

        /** AlgTool and IAlgTool interface methods */
        static const InterfaceID& interfaceID() {
            static const InterfaceID IID_ICombinedMuonTrackFitter("ICombinedMuonTrackFitter", 1, 0);
            return IID_ICombinedMuonTrackFitter;
        }

        /*refit a track*/
        virtual std::unique_ptr<Trk::Track> fit(const EventContext& ctx, const Trk::Track& track, const Trk::RunOutlierRemoval runOutlier = false,
                                                const Trk::ParticleHypothesis particleHypothesis = Trk::muon) const = 0;
    };

}  // namespace Rec

#endif  // MUIDINTERFACES_ICOMBINEDMUONTRACKBUILDER_H
