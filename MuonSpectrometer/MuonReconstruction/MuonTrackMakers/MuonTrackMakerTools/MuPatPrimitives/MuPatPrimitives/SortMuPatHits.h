/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SORTMUPATHITS_H
#define SORTMUPATHITS_H

#include "MuPatPrimitives/MuPatHit.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"

namespace Muon {

    class DistanceAlongParameters {
    public:
        double operator()(const Trk::TrackParameters& pars, const Amg::Vector3D& pos) const {
            Amg::Vector3D difPos = pos - pars.position();
            return difPos.dot(pars.momentum().unit());
        }

        double operator()(const Trk::TrackParameters& pars1, const Trk::TrackParameters& pars2) const {
            return this->operator()(pars1, pars2.position());
        }

        double operator()(const Trk::TrackParameters& pars, const Trk::MeasurementBase& meas) const {
            return this->operator()(pars, meas.globalPosition());
        }
        double operator()(const MuPatHitPtr& hit1, const MuPatHitPtr& hit2) const {
            return operator()(hit1->parameters(), hit2->parameters());
        }
        double operator()(const MuPatHit* hit1, const MuPatHit* hit2) const {
            return operator()(hit1->parameters(), hit2->parameters());
        }

    };

    //Not used atm (default is SortByDirectionMuPatHits)
    class SortByIPMuPatHits { 
    public:

        SortByIPMuPatHits(const IMuonIdHelperSvc* idh) : m_idh{idh} {}
        /// Sort the mu pat hits using their associated surfaces
        bool operator()(const MuPatHitPtr& hit1, const MuPatHitPtr& hit2) const {
            return operator()(hit1.get(), hit2.get());
        }
        bool operator()(const MuPatHit* hit1, const MuPatHit* hit2) const {
            // first, check if both hits are in the same chamber, and at least one is an RPC

            const MuPatHit::Info& info1 = hit1->info();
            const MuPatHit::Info& info2 = hit2->info();
            const Identifier& id1 = info1.id;
            const Identifier& id2 = info2.id;

            const Trk::Surface& surf1{hit1->measurement().associatedSurface()};
            const Trk::Surface& surf2{hit2->measurement().associatedSurface()};

            if (info1.isEndcap && info2.isEndcap) {
                const double absZ1 = std::abs(surf1.center().z());
                const double absZ2 = std::abs(surf2.center().z());
                if (std::abs(absZ1 - absZ2) > std::numeric_limits<float>::epsilon()) { return absZ1 < absZ2; }
            } else if (!info1.isEndcap && !info2.isEndcap) {
                const double perp1 = surf1.center().perp2();
                const double perp2 = surf2.center().perp2();
                if (std::abs(perp1 - perp2) > std::numeric_limits<float>::epsilon()) {return perp1 < perp2; }
            }
             /// large small overlap 
             else if (info1.isEndcap || info2.isEndcap) {
                    const MuonStationIndex::StIndex& st1 = info1.stIdx;
                    const MuonStationIndex::StIndex& st2 = info2.stIdx;
                    if (st1 != st2) return st1 < st2;                   
            }
            if (info1.type == info2.type) {
                if (info1.type == MuPatHit::sTGC) {
                    const sTgcIdHelper& id_helper = m_idh->stgcIdHelper();
                    const int type1 =id_helper.channelType(id1);
                    const int type2 =id_helper.channelType(id2);
                    if (type1 != type2) return type1 > type2;
                }
                if (info1.status != info2.status) {
                    return info1.status < info2.status;
                }
                if (info1.measuresPhi == info2.measuresPhi) {
                   return hit1->pull() < hit2->pull();
                }
            }
            // phi measurements larger than eta measurements
            return !info1.measuresPhi;
        }

        /// DistanceAlongParameters distanceCalculator;
        const IMuonIdHelperSvc* m_idh{nullptr};
    };

    /// Default for both collision and cosmic parameter sorting
    class SortByDirectionMuPatHits{
        public:
            SortByDirectionMuPatHits(const Trk::TrackParameters& refPars):
                m_ref(refPars) {}
        
        bool operator() (const Trk::MeasurementBase* meas1, const Trk::MeasurementBase* meas2) const  {
            return operator()(*meas1,*meas2);
        }
        bool operator() (const Trk::MeasurementBase& meas1, const Trk::MeasurementBase& meas2) const {
            static const DistanceAlongParameters parsSorter{};
            return parsSorter(m_ref, meas1) < parsSorter(m_ref, meas2);
        }

            bool operator()(const MuPatHitPtr& hit1, const MuPatHitPtr& hit2) const {
               return operator()(hit1->parameters(),hit2->parameters());
            }
            bool operator()(const MuPatHit* hit1, const MuPatHit* hit2) const {           
               return operator()(hit1->parameters(),hit2->parameters());
            }
            bool operator()(const Trk::TrackParameters* pars1, const Trk::TrackParameters* pars2) const {
               return operator()(*pars1,*pars2);
            }
            bool operator()(const Trk::TrackParameters& pars1, const Trk::TrackParameters& pars2) const {
               static const DistanceAlongParameters parsSorter{};
               return parsSorter(m_ref, pars1) < parsSorter(m_ref, pars2);
            }



        private:
            const Trk::TrackParameters& m_ref;

    };
    

}  // namespace Muon

#endif
