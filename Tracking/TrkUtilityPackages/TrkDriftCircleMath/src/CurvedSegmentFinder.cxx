/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkDriftCircleMath/CurvedSegmentFinder.h"
#define sq(x) ((x) * (x))

namespace TrkDriftCircleMath {

    void CurvedSegmentFinder::curvedSegments(const ChamberGeometry& mdtGeo, SegVec& segs) const {
        int nCurved = 0;
        // collect all the ML1 and ML2 only segments
        SegVec ml1segs, ml2segs;
        for (auto & seg : segs) {
            int isBarrel = seg.dcs()[0].id().isBarrel();
            if (!isBarrel) continue;

            if (seg.hitsMl2() == 0) {
                ml1segs.push_back(seg);
            } else if (seg.hitsMl1() == 0) {
                ml2segs.push_back(seg);
            }
        }
        // check that both ML have segments
        if (ml1segs.empty() || ml2segs.empty()) return;
        // Chamber information needed to calculate Delta b
        const LocVec2D& ml1LocVec2D = mdtGeo.tubePosition(0, mdtGeo.nlay(), 0);
        const LocVec2D& ml2LocVec2D = mdtGeo.tubePosition(1, 1, 0);
        double chamberMidPtY = (ml1LocVec2D.y() + ml2LocVec2D.y()) / 2.0;
        // loop over the ML segments and find matches
        if (m_debugLevel >= 10)
            std::cout << "CurvedSegmentsFinder begining match with " << ml1segs.size() << " ML1 segments and " << ml2segs.size()
                      << " ML2 segments" << std::endl;
        for (auto & ml1seg : ml1segs) {
            // bool foundCurvedSeg = false;
            const double tanML1 = std::tan(ml1seg.line().phi());
            const double mid1 = (chamberMidPtY - ml1seg.line().position().y()) / tanML1 + ml1seg.line().position().x();
            const double y01 = ml1seg.line().position().y() - ml1seg.line().position().x() * tanML1;
            for (auto & ml2seg : ml2segs) {
                // angle between the 2 segments
                const double deltaAlpha = ml1seg.line().phi() - ml2seg.line().phi();
                // distance of closest approach between the 2 segments at the middle of chamber
                                const double tanML2 = std::tan(ml2seg.line().phi());
                
                const double mid2 = (chamberMidPtY - ml2seg.line().position().y()) / tanML2 + ml2seg.line().position().x();
                const double y02 = ml2seg.line().position().y() - ml2seg.line().position().x() * tanML2;
                double deltab =  (mid2 * tanML1 - chamberMidPtY + y01) / std::hypot(1, tanML1);
                const double deltab2 = (mid1 * tanML2 - chamberMidPtY + y02) / std::hypot(1, tanML2);
                if (std::abs(deltab2) < std::abs(deltab)) deltab = deltab2;
                if (std::abs(deltaAlpha) >= m_maxDeltaAlpha  ||  std::abs(deltab) >= m_maxDeltab) continue;
                ++nCurved;
                
                // foundCurvedSeg = true;
                if (m_debugLevel >= 10)
                    std::cout << "CurvedSegment combination found with parameters (DeltaAlpha,Deltab) = (" << deltaAlpha << ", "
                                << deltab << ")" << std::endl;
                // build the curved segment
                DCOnTrackVec dcs = ml1seg.dcs();
                dcs.insert(dcs.end(), ml2seg.dcs().begin(), ml2seg.dcs().end());
                double segChi2 = ml1seg.chi2() + ml2seg.chi2();
                double segNDoF = ml1seg.ndof() + ml2seg.ndof();
                Segment seg(ml1seg.line(), dcs, segChi2, segNDoF, ml1seg.dtheta(), ml1seg.dy0());
                // find the x coordinate of at the middle of the chamber for ml2 segment
                const double xb = ml2seg.line().position().x() - (ml2seg.line().position().y() - chamberMidPtY) / tan(ml2seg.line().phi());
                // set the curvature parameters
                seg.setCurvatureParameters(deltaAlpha, xb);
                // hit information
                seg.deltas(ml1seg.deltas() + ml2seg.deltas());
                seg.hitsOutOfTime(ml1seg.hitsOutOfTime() + ml2seg.hitsOutOfTime());
                seg.hitsOnTrack(ml1seg.hitsOnTrack() + ml2seg.hitsOnTrack());
                seg.hitsPerMl(ml1seg.hitsMl1(), ml2seg.hitsMl2());
                seg.closeHits(ml1seg.closeHits() + ml2seg.closeHits());
                
                // store the new segment
                segs.push_back(seg);
                // remove the ML2 segment from the list
                // ml2segs.erase( ml2 );
                // break;
            }
        }  // end loop on ml2
        if (m_debugLevel >= 5) std::cout << "Finished CurvedSegments Finding, and found " << nCurved << " CurvedSegments" << std::endl;
   }

}  // end namespace TrkDriftCircleMath
