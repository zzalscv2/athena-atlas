/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughTransformer_xyz.h"
#include "CxxUtils/sincos.h"

MuonHoughTransformer_xyz::MuonHoughTransformer_xyz(int nbins, int nbins_angle, double detectorsize, double detectorsize_angle,
                                                   double threshold_histo, int number_of_sectors) :
    MuonHoughTransformer("MuonHoughTransformer_xyz", nbins, nbins_angle, detectorsize, detectorsize_angle, threshold_histo, number_of_sectors) {
    m_add_weight_radius = true;
    m_weight_constant_radius = 3000;
}

void MuonHoughTransformer_xyz::fillHit(const std::shared_ptr<MuonHoughHit>& hit, double weight) {
    double radius = hit->getRadius();
    double hitx = hit->getHitx();
    double hity = hit->getHity();
    int sectorhit = sector(hit);
    double dotprod{0.};

    if (m_ip_setting) {
        std::pair<double, double> endpoints = getEndPointsFillLoop(radius, m_stepsize, sectorhit);
        for (double r0 = endpoints.first; r0 < endpoints.second; r0 += m_stepsize) {
            double phi = calculateAngle(hitx, hity, r0);
            CxxUtils::sincos scphi(phi);
            dotprod = scphi.apply(hity, hitx);  // hity * sincosphi[0] + hitx * sincosphi[1];
            if (dotprod >= 0) {
                double phi_in_grad = phi * MuonHough::rad_degree_conversion_factor;
                fillHisto(r0, phi_in_grad, weight, sectorhit);
            }
        }
    }  // m_ip_setting

    else  // m_ip_setting == false
    {
        for (double phi = m_stepsize_per_angle / 2.; phi < m_detectorsize_angle; phi += m_stepsize_per_angle) {
            double phi_in_rad = MuonHough::degree_rad_conversion_factor * phi;
            CxxUtils::sincos scphi(phi_in_rad);
            double r0 = scphi.apply(hitx, -hity);
            fillHisto(r0, phi, weight, sectorhit);
        }
    }
}

int MuonHoughTransformer_xyz::fillHisto(double r0, double phi, double weight, int sector)  // phi in grad!
{
    MuonHoughHisto2D* histo = m_histos.getHisto(sector);

    const int filled_binnumber = histo->fill(r0, phi, weight);

    // applying a 'butterfly' weighting effect:
    bool butterfly = true;
    if (butterfly) {
        // nearby sectors:
        if (m_number_of_sectors >= 3) {
            double third_weight = weight / 3.;
            if (sector != 0 && sector != m_number_of_sectors - 1) {
                m_histos.getHisto(sector + 1)->fill(filled_binnumber, third_weight);
                m_histos.getHisto(sector - 1)->fill(filled_binnumber, third_weight);
            } else if (sector == 0) {
                m_histos.getHisto(sector + 1)->fill(filled_binnumber, third_weight);
            } else  // sector == m_number_of_sectors - 1
            {
                m_histos.getHisto(sector - 1)->fill(filled_binnumber, third_weight);
            }
        }

        double half_weight = 0.5 * weight;

        histo->fill(filled_binnumber - 1, half_weight);
        histo->fill(filled_binnumber + 1, half_weight);

        const int upperright = filled_binnumber + m_nbins_plus3;
        const int lowerleft = filled_binnumber - m_nbins_plus3;

        if (phi - m_binwidthy < 0) {
            histo->fill(r0 - m_binwidthx, phi - m_binwidthy + m_detectorsize_angle, half_weight);  // should calculate binnumber..
            histo->fill(upperright, half_weight);
            if (m_use_negative_weights) {
                histo->fill(r0 + m_binwidthx, phi - m_binwidthy + m_detectorsize_angle, -half_weight);
                histo->fill(upperright - 2, -half_weight);
            }
        } else if (phi + m_binwidthy > m_detectorsize_angle) {
            histo->fill(lowerleft, half_weight);
            histo->fill(r0 + m_binwidthx, phi + m_binwidthy - m_detectorsize_angle, half_weight);
            if (m_use_negative_weights) {
                histo->fill(lowerleft + 2, -half_weight);
                histo->fill(r0 - m_binwidthx, phi + m_binwidthy - m_detectorsize_angle, -half_weight);
            }
        } else {
            histo->fill(lowerleft, half_weight);
            histo->fill(upperright, half_weight);
            if (m_use_negative_weights) {
                histo->fill(lowerleft + 2, -half_weight);
                histo->fill(upperright - 2, -half_weight);
            }
        }
    }
    return filled_binnumber;
}

double MuonHoughTransformer_xyz::calculateAngle(double hitx, double hity, double r0) {
    double phi = 0;
    double height_squared = hitx * hitx + hity * hity - r0 * r0;
    if (height_squared >= 0) {
        double height = std::sqrt(height_squared);
        phi = std::atan2(hity, hitx) + std::atan2(r0, height);
    }

    else {
        phi = std::atan2(hity, hitx);
    }

    if (phi < 0) { phi += MuonHough::two_Pi; }
    if (phi > MuonHough::two_Pi) { phi -= MuonHough::two_Pi; }

    return phi;
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformer_xyz::hookAssociateHitsToMaximum(const MuonHoughHitContainer& event,
                                                                       std::pair<double, double> coordsmaximum, double max_residu_mm,
                                                                       double /*max_residu_angle*/, int max_sector) const {
    std::unique_ptr<MuonHoughPattern> houghpattern{initialiseHoughPattern()};
    ATH_MSG_DEBUG("MuonHoughTransformer_xyz::hookAssociateHitsToMaximum  (start)");
    

    double ephi{0.}, eradius{0.}, sin_phi{0.}, cos_phi{0.};
    double dotprod{0.}, etheta{0.}, residu_distance{0.};

    ATH_MSG_DEBUG("MuonHoughTransformer_xyz::size_event: " << event.size() );
    ATH_MSG_DEBUG("MuonHoughTransformer_xyz::found_maximum: r: " << coordsmaximum.first << " phi: " << coordsmaximum.second
                                                                 << " sector: " << max_sector);
           
    

    double phimax = m_muonhoughmathutils.angleFromGradToRadial(coordsmaximum.second);
    CxxUtils::sincos scphimax(phimax);

    int max_secmax = max_sector + 1;
    int max_secmin = max_sector - 1;
    if (max_secmin < 0) max_secmin = max_sector;
    if (max_secmax > m_number_of_sectors - 1) max_secmax = max_sector;

    for (unsigned int i = 0; i < event.size(); i++) {
        double hitx = event.getHitx(i);
        double hity = event.getHity(i);
        double hitz = event.getHitz(i);

        double radiushit = std::sqrt(hitx * hitx + hity * hity);
        int sectorhit = sector(event.getHit(i));

        if (sectorhit == max_sector || sectorhit == max_secmin || sectorhit == max_secmax) {
            if (!m_ip_setting) {
                dotprod = 1.;
            } else {
                dotprod = scphimax.apply(getHitPos(event, i).second,
                                         getHitPos(event, i).first);  
            }
            if (dotprod >= 0) {
                residu_distance = -coordsmaximum.first +
                                   scphimax.apply(getHitPos(event, i).first,
                                                  -getHitPos(event, i).second); 
                ATH_MSG_VERBOSE("MuonHoughTransformer_xyz::hitx: " << getHitPos(event, i).first
                                << " hity: " << getHitPos(event, i).second << " dotprod: " << dotprod << " sector: " 
                                << sectorhit<<",residu_distance: " << residu_distance);                

                if (std::abs(residu_distance) < max_residu_mm) {
                    houghpattern->addHit(event.getHit(i));

                    ATH_MSG_VERBOSE("MuonHoughTransformer_xyz::hit added to houghpattern!");
                    ATH_MSG_VERBOSE(" hit already earlier associated to pattern!");

                    event.getHit(i)->setAssociated(true);

                    double phi = calculateAngle(hitx, hity, coordsmaximum.first);

                    double thetah = std::atan2(radiushit, hitz);
                    ATH_MSG_VERBOSE(" X Y hit added to hough pattern phi hit " << std::atan2(event.getHity(i), event.getHitx(i))
                                     << " phi_calc: " << phi << " phi all " << phimax << " theta hit " << thetah );
                    

                    etheta += thetah;
                    CxxUtils::sincos scphi(phi);

                    sin_phi += scphi.sn;
                    cos_phi += scphi.cs;

                    double radius = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(event.getHitx(i), event.getHity(i), phimax);
                    eradius += radius;
                    ATH_MSG_VERBOSE(" calculateAngle: " << phi << " calculateradius: " << radius);                    
                }
            }  // dotprod >=0
        }      // sector requirement
    }          // size

    eradius = eradius / (houghpattern->size() + 0.0001);
    etheta = etheta / (houghpattern->size() + 0.0001);
    ephi = std::atan2(sin_phi, cos_phi);

    ATH_MSG_VERBOSE("ephi: " << ephi << " eradius: " << eradius << " etheta " << etheta);

    if (m_ip_setting)
        houghpattern->setEPhi(ephi);
    else {
        houghpattern->setEPhi(phimax);
    }

    houghpattern->setERPhi(coordsmaximum.first);
    houghpattern->setETheta(etheta);

    if (!houghpattern->empty() && std::abs(std::sin(houghpattern->getEPhi() - phimax)) > 0.05) {
        ATH_MSG_WARNING("MuonHoughTransformer_xyz:: Ephi calculation went wrong -- histo radius: " 
                        << coordsmaximum.first << " phi: " << phimax <<", ephi: " << ephi);
        houghpattern->setEPhi(MuonHoughMathUtils::angleFromMinusPiToPi(phimax));
        houghpattern->setERPhi(coordsmaximum.first);
    }

    if (!m_ip_setting) {
        houghpattern->updateParametersRPhi(true);  // switch off ip constraint! (cosmics==true), on by default (cosmics== false)

        ATH_MSG_VERBOSE("updateParameterstheta new phi (phi flipped Pi for cosmics): " << houghpattern->getEPhi()
                        << " old phi: " << phimax <<" new r0: " << houghpattern->getERPhi()
                        << " old r0: " << coordsmaximum.first);
    }

    return houghpattern;
}

float MuonHoughTransformer_xyz::weightHoughTransform(double r0) const {
    if (m_add_weight_radius) {
        return (1. / (std::abs(r0 / m_weight_constant_radius) + 1.));
    } else {
        return 1;
    }  // weight function, to give more importance to patterns close to origin
}

int MuonHoughTransformer_xyz::sector(const std::shared_ptr<MuonHoughHit>& hit) const {
    double radius = hit->getRadius();
    double hitz = hit->getHitz();

    //  returns  the sector number of the hit 0..m_number_of_sectors-1

    // Peter Kluit correction
    double theta = std::atan2(radius, hitz);  // radius>0 : theta: [0,Pi]

    int sectorhit = static_cast<int>(theta * m_number_of_sectors / M_PI);
    if (sectorhit == m_number_of_sectors) sectorhit += -1;  // could happen in rare cases
    return sectorhit;                                       // only valid for xy!! yz to be done (or to be abondoned)
}
