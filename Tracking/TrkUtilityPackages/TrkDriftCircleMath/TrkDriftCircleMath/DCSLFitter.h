/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DCMATH_DCSLFITTER_H
#define DCMATH_DCSLFITTER_H

#include <vector>

#include "TrkDriftCircleMath/DCOnTrack.h"
#include "TrkDriftCircleMath/HitSelection.h"
#include "TrkDriftCircleMath/Line.h"
#include "TrkDriftCircleMath/Segment.h"

namespace TrkDriftCircleMath {

class DCSLFitter {
 public:
  struct FitData {
    double y{0};    // local y position
    double z{0};    // local z position
    double r{0};    // drift radius
    double w{0};    // weight
    double rw{0};   // weighted radius
    double ryw{0};  // weighted y position
    double rzw{0};  // weighted z position
  };

  DCSLFitter() = default;
  virtual ~DCSLFitter() = default;
  virtual bool fit(Segment& result, const Line& line, const DCOnTrackVec& dcs,
                   double t0Seed = -99999.) const;
  virtual bool fit(Segment& result, const Line& line, const DCOnTrackVec& dcs,
                   const HitSelection& selection,
                   double t0Seed = -99999.) const;
};

inline bool DCSLFitter::fit(Segment& result, const Line& line,
                            const DCOnTrackVec& dcs, double) const {
  HitSelection selection(dcs.size(), 0);
  return fit(result, line, dcs, selection);
}
}  // namespace TrkDriftCircleMath

#endif
