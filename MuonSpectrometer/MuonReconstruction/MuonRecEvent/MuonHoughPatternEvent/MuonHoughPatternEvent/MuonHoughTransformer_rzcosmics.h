/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_RZCOSMICS_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_RZCOSMICS_H

#include "MuonHoughPatternEvent/MuonHoughTransformer.h"

class MuonHoughTransformer_rzcosmics : public MuonHoughTransformer {
public:
    MuonHoughTransformer_rzcosmics(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle, double threshold_histo,
                                   int number_of_sectors = 1);
    ~MuonHoughTransformer_rzcosmics() = default;

    void fillHit(const std::shared_ptr<MuonHoughHit>&  hit, double weight = 1.) override final;
    int fillHisto(double rz0, double theta, double weight = 1., int sector = 0) override final;

    float weightHoughTransform(double r0) const override final;

    int sector(const std::shared_ptr<MuonHoughHit>&  hit) const override final;  // 0..15 same as atlas sector 1..16 // returns 0 if number_of_sectors == 0
protected:
    std::unique_ptr<MuonHoughPattern> hookAssociateHitsToMaximum(const MuonHoughHitContainer* event, std::pair<double, double> coordsmaximum,
                                                         double residu_mm, double residu_grad, int sector = 0, bool which_segment = 0,
                                                         int printlevel = 999) const override final;

private:
    /** recalculate trackparameters of pattern */
    static void updateParameters(MuonHoughPattern*);

    /** weight in transform, dotprod is the phi angle between the normal and the phisector */
    float weightHoughTransform(double r0, double sintheta, double sinphi, double dotprod) const;

    /** arrays that store values of transform */
    std::unique_ptr<double[]> m_phisec{};
    std::unique_ptr<double[]> m_sinphisec{};
    std::unique_ptr<double[]> m_cosphisec{};
    std::unique_ptr<double[]> m_theta_in_grad{};
    std::unique_ptr<double[]> m_sintheta{};
    std::unique_ptr<double[]> m_costheta{};  
};

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMER_RZCOSMICS_H
