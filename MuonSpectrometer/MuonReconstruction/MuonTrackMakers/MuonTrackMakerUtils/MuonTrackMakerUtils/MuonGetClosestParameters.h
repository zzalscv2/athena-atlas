/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGETCLOSESTPARAMETERS_H
#define MUONGETCLOSESTPARAMETERS_H

#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/Track.h"

namespace Muon {

    class MuonGetClosestParameters {
    public:
        static double distance(const Trk::TrackParameters& pars, const Amg::Vector3D& pos) {
            return (pars.position() - pos).dot(pars.momentum().unit());
        }
        static std::unique_ptr<Trk::TrackParameters> closestParameters(const Trk::Track& track, const Amg::Vector3D& pos, bool onlyUseMeasured = false) {
            const DataVector<const Trk::TrackParameters>* pars = track.trackParameters();
            if (!pars || pars->empty()) { return nullptr; }

            bool firstOk = onlyUseMeasured ? pars->front()->covariance() != nullptr : true;
	        bool lastOk = onlyUseMeasured ? pars->back()->covariance() != nullptr : true;

            const double distFront = distance(*pars->front(), pos);
            if (distFront > 0. && firstOk) { return pars->front()->uniqueClone(); }

            const double distBack = distance(*pars->back(), pos);
            if (distBack < 0. && lastOk) { return pars->back()->uniqueClone(); }

            bool startFront = std::abs(distFront) < distBack;

            const Trk::TrackParameters* result{nullptr}, *prevresult{nullptr};
            if (startFront) {
                double prevDist = distFront - 1.;
                // loop over parameters, calculate distance
                DataVector<const Trk::TrackParameters>::const_iterator it = pars->begin();
                DataVector<const Trk::TrackParameters>::const_iterator it_end = pars->end();
                for (; it != it_end; ++it) {
                    if (onlyUseMeasured && !(*it)->covariance()) continue;

                    double dist = distance(**it, pos);
                    // check whether dist flips sign, if this happens select either the current hit or the previous
                    if (dist > 0.) {
                        if (std::abs(dist) < std::abs(prevDist))
                            result = *it;
                        else if (it == pars->begin())
                            result = *it;
                        else
                            result = prevresult;
                        break;
                    }
                    prevDist = dist;
                    prevresult = *it;
                }
            } else {
                double prevDist = distBack + 1.;
                // loop over parameters, calculate distance
                DataVector<const Trk::TrackParameters>::const_reverse_iterator it = pars->rbegin();
                DataVector<const Trk::TrackParameters>::const_reverse_iterator it_end = pars->rend();
                for (; it != it_end; ++it) {
                    if (onlyUseMeasured && !(*it)->covariance()) continue;

                    const double dist = distance(**it, pos);
                    // check whether dist flips sign, if this happens select either the current hit or the previous
                    if (dist < 0.) {
                        if (std::abs(dist) < std::abs(prevDist))
                            result = *it;
                        else if (it == pars->rbegin())
                            result = *it;
                        else
                            result = prevresult;
                        break;
                    }
                    prevDist = dist;
                    prevresult = *it;
                }
            }

            return result ? result->uniqueClone() : nullptr;
        }

        static std::unique_ptr<Trk::TrackParameters> closestParameters(const Trk::Track& track, 
                                                                       const Trk::Surface& surf, 
                                                                       bool onlyUseMeasured = false) {
            return closestParameters(track, surf.center(), onlyUseMeasured);
        }

        static std::unique_ptr<Trk::TrackParameters> closestParameters(const Trk::Track& track, 
                                                                       const Trk::TrackParameters& pars,
                                                                       bool onlyUseMeasured = false) {
            return closestParameters(track, pars.position(), onlyUseMeasured);
        }
    };

}  // namespace Muon

#endif
