/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_XY_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_XY_H

#include "MuonHoughPatternEvent/MuonHoughTransformer_xyz.h"

class MuonHoughTransformer_xy : public MuonHoughTransformer_xyz {
public:
    /** constructor */
    MuonHoughTransformer_xy(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle, double threshold_histo,
                            int number_of_sectors = 1);
    /** destructor */
    virtual ~MuonHoughTransformer_xy() = default;

    /** returns the hit position in xy frame */
    std::pair<double, double> getHitPos(const MuonHoughHitContainer& event, int hitid) const override;
    /** build new houghpattern */
    std::unique_ptr<MuonHoughPattern> initialiseHoughPattern() const override final;   
};

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_XY_H
