/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_RZ_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_RZ_H

#include "MuonHoughPatternEvent/MuonHoughTransformer.h"

class MuonHoughTransformer_rz : public MuonHoughTransformer {
public:
    MuonHoughTransformer_rz(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle, double threshold_histo,
                            int number_of_sectors = 1);
    virtual ~MuonHoughTransformer_rz() = default;

    virtual void fillHit(const std::shared_ptr<MuonHoughHit>&  hit, double weight = 1.) override final;
    virtual int fillHisto(double rz0, double theta, double weight, int sector) override final;

    static double calculateAngle(double hitx, double hity, double hitz, double z0);  // in rad

    virtual float weightHoughTransform(double r0) const  override final;
    float weightHoughTransform(double r0, double angle) const;
    
    int sector(const std::shared_ptr<MuonHoughHit>& hit) const  override final;  // 0..15 same as atlas sector 1..16 // returns 0 if number_of_sectors == 0

protected:
    std::unique_ptr<MuonHoughPattern> hookAssociateHitsToMaximum(const MuonHoughHitContainer& event, std::pair<double, double> coordsmaximum,
                                                         double residu_mm, double residu_grad, int sector) const  override final;

    const bool m_use_residu_grad;  // 0 is advisable //only used for rz
};

inline int MuonHoughTransformer_rz::sector(const std::shared_ptr<MuonHoughHit>&  hit) const { return hit->phiSector(); }

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_RZ_H
