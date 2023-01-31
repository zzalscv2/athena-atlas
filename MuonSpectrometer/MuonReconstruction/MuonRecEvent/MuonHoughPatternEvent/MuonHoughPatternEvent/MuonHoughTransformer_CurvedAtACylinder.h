/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_CURVEDATACYLINDER_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_CURVEDATACYLINDER_H

#include "MuonHoughPatternEvent/MuonHoughTransformer.h"

class MuonHoughTransformer_CurvedAtACylinder : public MuonHoughTransformer {
public:
    /** constructor */
    MuonHoughTransformer_CurvedAtACylinder(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle,
                                           double threshold_histo, int number_of_sectors = 1);
    /** destructor */
    ~MuonHoughTransformer_CurvedAtACylinder() = default;

    /** fill hit in histogram */
    void fillHit(const std::shared_ptr<MuonHoughHit>& hit, double weight = 1.) override final;
    /** fill transformed values in histogram */
    int fillHisto(double xbin, double theta, double weight = 1., int sector = 0)  override final;

    /** associate hits to maximum found */
    std::unique_ptr<MuonHoughPattern> hookAssociateHitsToMaximum(const MuonHoughHitContainer* event, std::pair<double, double> coordsmaximum,
                                                         double residu_mm, double residu_grad, int sector = 0, bool which_segment = 0,
                                                         int printlevel = 999) const override final;

    /** returns the phi sector */
    int sector(const std::shared_ptr<MuonHoughHit>& hit) const override final;
    /** not implemented for this transform */
    float weightHoughTransform(double r0) const  override final;

private:
    /** array that stores the inverse curvatures that are scanned */
    std::unique_ptr<double[]> m_invcurvature;
    std::unique_ptr<double[]> m_weightcurvature;
   
};

inline int MuonHoughTransformer_CurvedAtACylinder::sector(const std::shared_ptr<MuonHoughHit>& hit) const { return hit->phiSector(); }

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_CURVEDATACYLINDER_H
