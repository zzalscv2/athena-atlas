/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughTransformer_rz.h"

MuonHoughTransformer_rz::MuonHoughTransformer_rz(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle,
                                                 double threshold_histo, int number_of_sectors) :
    MuonHoughTransformer("MuonHoughTransformer_rz", nbins, nbins_angle, detectorsize, detectorsize_angle, threshold_histo, number_of_sectors),
    m_use_residu_grad(false) {
    m_add_weight_radius = false;
    m_weight_constant_radius = 0.;
    m_add_weight_angle = false;
}

void MuonHoughTransformer_rz::fillHit(const std::shared_ptr<MuonHoughHit>& hit, double weight) {
    const double hitz = hit->getHitz();

    if (m_ip_setting)  // niels and peter alg, butterfly to be added, and to be looked into the method
    {
        const int sectorhit = sector(hit);
        const double perp = hit->getRadius();
        const double radius = hit->getAbs();

        double dotprod = 0;
        MuonHoughHisto2D* histo = m_histos.getHisto(sectorhit);
        for (double rz0 = histo->getXmin() + m_stepsize / 2.; rz0 < histo->getXmax(); rz0 += m_stepsize) {
            if (std::abs(rz0) > radius) continue;

            double height = std::sqrt(radius * radius - rz0 * rz0);
            double theta = std::atan2(perp, hitz) + std::atan2(rz0, height);
            double theta_in_grad = (theta / M_PI) * 180.;

            dotprod = perp * std::sin(theta) + hitz * std::cos(theta);

            if (theta_in_grad > 180.) continue;  // to keep the angle physical
            if (theta_in_grad < 0.) continue;    // idem

            if (dotprod >= 0) { fillHisto(rz0, theta_in_grad, weight, sectorhit); }
        }
    } else {
        int sectorhit = 0;
        const double radius = hit->getRadius();

        for (double theta = m_stepsize_per_angle / 2.; theta < m_detectorsize_angle; theta += m_stepsize_per_angle) {
            double theta_in_rad = M_PI * theta / 180.;
            double rz0 = hitz * std::sin(theta_in_rad) - radius * std::cos(theta_in_rad);
            double dotprod = 0;

            dotprod = radius * std::sin(theta_in_rad) + hitz * std::cos(theta_in_rad);
            if (dotprod >= 0) { fillHisto(rz0, theta, weight, sectorhit); }  // dotprod
        }
    }  // m_atlas_setting
}

int MuonHoughTransformer_rz::fillHisto(double rz0, double theta_in_grad, double weight, int sector) {
    MuonHoughHisto2D* histo = m_histos.getHisto(sector);

    double binwidthx = histo->getBinWidthX();
    double binwidthy = histo->getBinWidthY();

    int filled_binnumber = histo->fill(rz0, theta_in_grad, weight);
    // butterfly:

    // nearby sectors:
    if (m_number_of_sectors >= 3) {
        double third_weight = weight / 3.;
        if (sector != 0 && sector != m_number_of_sectors - 1) {
            m_histos.getHisto(sector + 1)->fill(rz0, theta_in_grad, third_weight);
            m_histos.getHisto(sector - 1)->fill(rz0, theta_in_grad, third_weight);
        } else if (sector == 0) {
            m_histos.getHisto(sector + 1)->fill(rz0, theta_in_grad, third_weight);
            m_histos.getHisto(m_number_of_sectors - 1)->fill(rz0, theta_in_grad, third_weight);
        } else {
            m_histos.getHisto(sector - 1)->fill(rz0, theta_in_grad, third_weight);
            m_histos.getHisto(0)->fill(rz0, theta_in_grad, third_weight);
        }
    }

    double half_weight = 0.5 * weight;

    if (theta_in_grad - binwidthy < 0) {
        histo->fill(rz0 + binwidthx, theta_in_grad + binwidthy, half_weight);
        if (m_use_negative_weights) { histo->fill(rz0 - binwidthx, theta_in_grad + binwidthy, -half_weight); }

    } else if (theta_in_grad + binwidthy > 180.) {
        histo->fill(rz0 - binwidthx, theta_in_grad - binwidthy, half_weight);
        if (m_use_negative_weights) { histo->fill(rz0 + binwidthx, theta_in_grad - binwidthy, -half_weight); }
    } else {
        histo->fill(rz0 + binwidthx, theta_in_grad + binwidthy, half_weight);
        histo->fill(rz0 - binwidthx, theta_in_grad - binwidthy, half_weight);
        if (m_use_negative_weights) {
            histo->fill(rz0 - binwidthx, theta_in_grad + binwidthy, -half_weight);
            histo->fill(rz0 + binwidthx, theta_in_grad - binwidthy, -half_weight);
        }
    }
    return filled_binnumber;
}

double MuonHoughTransformer_rz::calculateAngle(double hitx, double hity, double hitz, double z0) {
    // z0 is cartesian coordinate where track goes through z axis

    // analog to xyz:
    double theta = 0;
    double r = std::sqrt(hitx * hitx + hity * hity);

    theta = std::atan2(r, hitz - z0);

    return theta;
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformer_rz::hookAssociateHitsToMaximum(const MuonHoughHitContainer& event,
                                                                      std::pair<double, double> coordsmaximum, double maximum_residu_mm,
                                                                      double maximum_residu_angle, int maxsector) const {
    
    std::unique_ptr<MuonHoughPattern> houghpattern = std::make_unique<MuonHoughPattern>(MuonHough::hough_rz);

    double theta{0.}, residu_distance{0.}, maximum_residu{0.};
    double eradius{0.}, etheta{0.}, sin_theta{0.}, cos_theta{0.}, sin_phi{0.}, cos_phi{0.}, phi{0.}, ephi{0.};
    double coordsmaximumsecondinrad = m_muonhoughmathutils.angleFromGradToRadial(coordsmaximum.second);
    double rz0 = coordsmaximum.first;

    ATH_MSG_VERBOSE("MuonHoughTransformer_rz::hookAssociateHitsToMaximum() -- sector: " << maxsector
                    << "coordsmaximumfirst: " << rz0 << "coordsmaximumsecond: " << coordsmaximum.second 
                    << " coordsmaximumsecondinrad: " << coordsmaximumsecondinrad
                    <<" size of event: " << event.size());

    for (unsigned int i = 0; i < event.size(); i++) {
        double dotprod = 0;
        double hitx = event.getHitx(i);
        double hity = event.getHity(i);
        int sectorhit = sector(event.getHit(i));
        int maxsecmax = maxsector + 1;
        int maxsecmin = maxsector - 1;
        if (maxsecmin < 0) maxsecmin = m_number_of_sectors - 1;
        if (maxsecmax > m_number_of_sectors - 1) maxsecmax = 0;
        // select which hits could be in maximum:
        if (sectorhit == maxsector || sectorhit == maxsecmin || sectorhit == maxsecmax) {
            double hitz = event.getHitz(i);
            double radius = std::hypot(hitx, hity);

            dotprod = radius * std::sin(coordsmaximumsecondinrad) + hitz * std::cos(coordsmaximumsecondinrad);

            if (dotprod >= 0) {
                double residu_distance_mm = MuonHoughMathUtils::signedDistanceToLine(hitz, radius, rz0, coordsmaximumsecondinrad);

                // Use this code for rz scan and theta
                //
                double radius3 = std::hypot(hitx, hity, hitz);
                double height{0.};
                if (std::abs(rz0) < radius3) {
                    height = std::sqrt(radius3 * radius3 - rz0 * rz0);
                } else {
                    height = std::sqrt(rz0 * rz0 - radius3 * radius3);
                }

                theta = std::atan2(radius, hitz) + std::atan2(rz0, height);
                ATH_MSG_VERBOSE("theta: " << theta << " height: " << height << " radius3: " << radius3
                        << "  std::atan2(radius,hitz): " << std::atan2(radius, hitz)
                        << " +std::atan2(rz0,height): " << std::atan2(rz0, height) << " rz0: ");               
                if (m_use_residu_grad == 1) {
                    double residu_distance_grad = std::abs(std::sin(theta - coordsmaximumsecondinrad));
                    residu_distance = residu_distance_grad;
                    maximum_residu = maximum_residu_angle;
                } else {
                    residu_distance = residu_distance_mm;
                    maximum_residu = maximum_residu_mm;
                }

                ATH_MSG_VERBOSE("hitx: " << hitx << " hity: " << hity << " hitz: " << hitz 
                                << ", residu_distance: " << residu_distance);                
                bool inmax = false;
                if (std::abs(theta * 180. / M_PI - coordsmaximum.second) < 1.1) inmax = true;

                if (std::abs(residu_distance) < maximum_residu)  // here no circular effect
                {
                   ATH_MSG_VERBOSE(" hit added to houghpattern! -- sector number hit " << sectorhit << " max " << maxsector
                            << " detector: " << event.getHit(i)->getWhichDetector() <<  
                            (inmax ?  " MuonHoughTransformer_rz:: in maximum " : " OUTSIDE maximum" )<< " theta hit " << theta * 180. / M_PI << " max Hough theta "
                                << coordsmaximum.second );
                    houghpattern->addHit(event.getHit(i));

                    event.getHit(i)->setAssociated(true);

                    double rz0hit = residu_distance_mm + rz0;
                    eradius += rz0hit;
                    ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" calculateAngle: " << theta << " calculateradius: " << rz0hit
                                    << " R Z hit added to hough pattern theta hit " 
                                    << atan2(std::hypot(event.getHitx(i) ,event.getHity(i)), event.getHitz(i))
                                    << " theta all " << coordsmaximumsecondinrad);
                    sin_theta += std::sin(theta);
                    cos_theta += std::cos(theta);

                    phi = std::atan2(hity, hitx);

                    sin_phi += std::sin(phi);
                    cos_phi += std::cos(phi);
                    //}
                } else if (inmax)
                    ATH_MSG_WARNING("LOST hit in maximum distance " );
            }  // dotprod
        }      // sector constraint
    }          // hitno

    etheta = std::atan2(sin_theta, cos_theta);
    ephi = std::atan2(sin_phi, cos_phi);
    houghpattern->setEPhi(ephi);

    eradius = eradius / (houghpattern->size() + 0.0001);

    ATH_MSG_VERBOSE("Etheta : " << etheta << " Size houghpattern " << houghpattern->size() << " ephi "
                    << ephi );
    houghpattern->setETheta(etheta);
    houghpattern->setERTheta(eradius);
    houghpattern->setECurvature(1.);

    if (houghpattern->empty()) {
        ATH_MSG_VERBOSE("no hits found on pattern");
    }

    else if (std::abs(eradius - rz0) > 500. || std::sin(etheta - coordsmaximumsecondinrad) > 0.05) {
        ATH_MSG_VERBOSE("WARNING Eradius or Etheta calc. WRONG -- rz0: " << rz0 
                        << " etheta: " << coordsmaximumsecondinrad<< " eradius: " << eradius << " etheta: " << etheta );
        houghpattern->setETheta(coordsmaximumsecondinrad);  // coordsmaximumsecondinrad
        houghpattern->setERTheta(rz0);
    }

    return houghpattern;
}

float MuonHoughTransformer_rz::weightHoughTransform(double r0) const {
    if (m_add_weight_radius) {
        return 1. / (std::abs(r0 / (m_weight_constant_radius + 1.)));
    } else {
        return 1;
    }  // weight function, to give more importance to patterns close to origin
}

float MuonHoughTransformer_rz::weightHoughTransform(double r0, double theta) const  // theta in grad
{
    if (!m_add_weight_angle) {
        return weightHoughTransform(r0);
    } else {
        if (m_add_weight_radius) {
            double theta_rad = m_muonhoughmathutils.angleFromGradToRadial(theta);

            float r_theta_weight = std::abs(std::sin(theta_rad)) / (1. + std::abs((r0 / 6000.)));

            return r_theta_weight;
        }

        else {
            return 1;
        }  // weight function, to give more importance to patterns close to origin
    }
}
