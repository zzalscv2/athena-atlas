/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_YZ_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_YZ_H

#include "MuonHoughPatternEvent/MuonHoughTransformer_xyz.h"

class MuonHoughTransformer_yz : public MuonHoughTransformer_xyz {
public:
    /** constructor */
    MuonHoughTransformer_yz(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle, double threshold_histo,
                            int number_of_sectors = 1);
    /** destructor */
    virtual ~MuonHoughTransformer_yz() = default;

    /** returns the hit position in yz frame */
    std::pair<double, double> getHitPos(const MuonHoughHitContainer& event, int hitid)
        const override final;  // returns the relevant position of the hit (xy-RPC in case of id==id_xy_rpc etc.)
    /** build new houghpattern */
    std::unique_ptr<MuonHoughPattern> initialiseHoughPattern() const override final;
   
};

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_YZ_H
