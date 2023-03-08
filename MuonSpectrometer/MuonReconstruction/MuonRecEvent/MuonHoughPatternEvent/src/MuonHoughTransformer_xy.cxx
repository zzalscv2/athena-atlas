/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughTransformer_xy.h"

MuonHoughTransformer_xy::MuonHoughTransformer_xy(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle,
                                                 double threshold_histo, int number_of_sectors) :
    MuonHoughTransformer_xyz(nbins, nbins_angle, detectorsize, detectorsize_angle, threshold_histo, number_of_sectors) {}

std::pair<double, double> MuonHoughTransformer_xy::getHitPos(const MuonHoughHitContainer& event, int hitid)
    const  // returns the relevant position of the hit (xy-RPC in case of id_number==id_xy_rpc etc.)
{
    return {event.getHitx(hitid),event.getHity(hitid)};    
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformer_xy::initialiseHoughPattern() const {
    return std::make_unique<MuonHoughPattern>(MuonHough::hough_xy);   
}
