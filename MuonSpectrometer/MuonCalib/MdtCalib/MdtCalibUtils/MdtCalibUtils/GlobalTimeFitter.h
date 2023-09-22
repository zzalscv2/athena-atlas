/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_GLOBALTIMEFITTER_H
#define MUONCALIB_GLOBALTIMEFITTER_H

#include <iostream>

#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibInterfaces/IMdtSegmentFitter.h"
#include "MuonCalibEvent/MdtCalibHit.h"
#include "MuonCalibEventBase/MuonCalibSegment.h"

namespace MuonCalib {

    /**
    @class GlobalTimeFitter
    Provides the operator to fit  ....
    */

    class GlobalTimeFitter {
    public:
        GlobalTimeFitter(const IMdtSegmentFitter *fitter) : m_fitter(fitter) {}
        GlobalTimeFitter(const IMdtSegmentFitter *fitter, const IRtRelation *rtRel) : m_fitter(fitter), m_rtRel(rtRel) {}
        ~GlobalTimeFitter() = default;

        double GTFit(MuonCalibSegment *seg);
        double GTFit2(MuonCalibSegment *seg);

        IRtRelation *getDefaultRtRelation();

        void setRtRelation(const IRtRelation *rtRel) { m_rtRel = rtRel; };

        double getDefaultResolution(double r) {
            double resolH8 = 0.164 * std::exp(-r / 4.43) + 0.043;  //  resolution from H8 TestBeam
            return 2. * resolH8;                                   //  resolution from Cosmics in the pit
                                                                   // return 2.0 ; //  FLAT 2 mm resolution
        };
        bool fit(MuonCalibSegment &seg) const { return m_fitter->fit(seg); }
        bool fit(MuonCalibSegment &seg, const MuonCalib::IMdtSegmentFitter::HitSelection& selection) const {
            return m_fitter->fit(seg, selection);
        }

    private:
        const IMdtSegmentFitter *m_fitter{nullptr};
        const IRtRelation *m_rtRel{nullptr};
    };

}  // namespace MuonCalib

#endif
