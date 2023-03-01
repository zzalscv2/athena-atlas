/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughTransformer.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "MuonHoughPatternEvent/MuonHoughHitContainer.h"

MuonHoughTransformer::MuonHoughTransformer(const std::string& tr_name,int nbins, int nbins_angle, double detectorsize, double detectorsize_angle,
                                           double threshold_histo, int number_of_sectors) :
    AthMessaging(tr_name),
    m_threshold_histo(threshold_histo),
    m_eventsize(0),
    m_eventsize_weightfactor(20.),
    m_nbins(nbins),
    m_nbins_plus3(m_nbins + 3),
    m_nbins_angle(nbins_angle),
    m_detectorsize(detectorsize),
    m_detectorsize_angle(detectorsize_angle),
    m_number_of_sectors(number_of_sectors) {
    m_add_weight_angle = false;
    m_weight_constant_angle = 1.;
    m_weight_constant_radius = 1.;

    m_use_negative_weights = false;
    m_ip_setting = true;

    m_stepsize = 2 * detectorsize / (nbins + 0.);
    m_stepsize_per_angle = detectorsize_angle / (nbins_angle + 0.);

    m_add_weight_radius = 1.;
    m_histos.reserve(m_number_of_sectors);
    for (int i = 0; i < m_number_of_sectors; i++) {
        std::unique_ptr<MuonHoughHisto2D> histo = std::make_unique<MuonHoughHisto2D>(nbins, -detectorsize, detectorsize, nbins_angle, 0., detectorsize_angle);
        histo->setThreshold(m_threshold_histo);
        m_histos.push_back(std::move(histo));
    }

    m_binwidthx = m_histos.getHisto(0)->getBinWidthX();
    m_binwidthy = m_histos.getHisto(0)->getBinWidthY();
}

MuonHoughTransformer::~MuonHoughTransformer() = default;

void MuonHoughTransformer::fill(const MuonHoughHitContainer& event, bool subtract) {
    m_eventsize = event.size();
    m_eventsize_weightfactor = 20. * std::sqrt(m_eventsize) / std::sqrt(7000.);
    if (subtract) {
        // invalidate maxima
        for (int i = 0; i < m_histos.size(); ++i) m_histos.getHisto(i)->setMaximumIsValid(false);

        for (unsigned int hitid = 0; hitid < m_eventsize; hitid++) {
            std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
            if (hit->getAssociated()) fillHit(hit, -1. * hit->getWeight());
        }
    } else {
        for (unsigned int hitid = 0; hitid < m_eventsize; hitid++) { fillHit(event.getHit(hitid), event.getHit(hitid)->getWeight()); }
    }
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformer::associateHitsToMaximum(const MuonHoughHitContainer& event, double maximum_residu_mm,
                                                               double maximum_residu_grad, int maximum_number) const {
    std::unique_ptr<MuonHoughPattern> houghpattern{};
    std::pair<double, double> coordsmaximum;
    std::pair<int, int> maximumbin;
    maximumbin = m_histos.getMaximumBinnumber();

    int sector = maximumbin.first;

    if (sector != -1) {
       coordsmaximum = m_histos.getHisto(sector)->getCoordsMaximum(maximum_number);

       ATH_MSG_VERBOSE("maximum binnumber of histogram: " << maximumbin.second
                        << " value: " << m_histos.getHisto(sector)->getBinContent(maximumbin.second));
       ATH_MSG_VERBOSE(" coordinates: " << coordsmaximum.first << " second " << coordsmaximum.second<<" sector: "<<sector);


        if (maximumbin.second == -1)  // no maximum, no bin above threshold
        {
            ATH_MSG_VERBOSE("No Maximum Found");
            return nullptr;
        } else {
            houghpattern = hookAssociateHitsToMaximum(event, coordsmaximum, maximum_residu_mm, maximum_residu_grad, sector);
            if (houghpattern) { houghpattern->setMaximumHistogram(m_histos.getHisto(sector)->getBinContent(maximumbin.second)); }
        }
    } else {
        ATH_MSG_VERBOSE("No Maximum Found sector is -1");
        return nullptr;
    }

    return houghpattern;
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformer::associateHitsToCoords(const MuonHoughHitContainer& event, std::pair<double, double> coordsmaximum,
                                                              double maximum_residu_mm, double maximum_residu_angle, int sector) const {
    return hookAssociateHitsToMaximum(event, coordsmaximum, maximum_residu_mm, maximum_residu_angle, sector);   
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformer::associateHitsToBinnumber(const MuonHoughHitContainer& event, int binnumber,
                                                                 double maximum_residu_mm, double maximum_residu_angle, int sector) const {
    ATH_MSG_VERBOSE("associateHitsToBinnumber() -- sector "<<sector<<",binnumber "<<binnumber<<
                    " maximum of histogram: " << m_histos.getHisto(sector)->getBinContent(binnumber));
   
    std::pair<double, double> coordsmaximum = m_histos.getHisto(sector)->binnumberToCoords(binnumber);
    std::unique_ptr<MuonHoughPattern> houghpattern = hookAssociateHitsToMaximum(event, coordsmaximum, 
                                                     maximum_residu_mm, maximum_residu_angle, sector);
    houghpattern->setMaximumHistogram(m_histos.getHisto(sector)->getBinContent(binnumber));
    return houghpattern;
}

std::pair<double, double> MuonHoughTransformer::getEndPointsFillLoop(double radius, double stepsize, int sector) const {
    std::pair<double, double> endpoints(-radius + 0.00001, radius);  // why +0.00001?

    if (-radius < m_histos.getHisto(sector)->getXmin())  // randomizer to avoid binning effects
    {
        endpoints.first = m_histos.getHisto(sector)->getXmin() + 0.5 * stepsize;  // no randomizer! no radius constraint
    }

    if (radius > m_histos.getHisto(sector)->getXmax()) { endpoints.second = m_histos.getHisto(sector)->getXmax(); }
    return endpoints;
}

void MuonHoughTransformer::resetHisto() { m_histos.reset(); }

std::vector<std::pair<int, int> > MuonHoughTransformer::getMaxima(int max_patterns) const {
    std::vector<std::pair<int, int> > maximumbins;  // sorted

    std::vector<std::pair<std::pair<int, int>, double> > maxima;

    for (int sector = 0; sector < m_number_of_sectors; sector++)  // to be made more general when m_number_of_sectors ==1 e.g.
    {
        std::pair<int, double> maximumbin = m_histos.getHisto(sector)->getMax();
        std::pair<std::pair<int, int>, double> maximum;
        maximum.first.first = sector;
        maximum.first.second = maximumbin.first;
        maximum.second = maximumbin.second;
        maxima.push_back(maximum);
    }

    sort(maxima.begin(), maxima.end(), maximaCompare());

    unsigned int count_maxima = 0;  // should be iterator
    int number_of_patterns = 0;
    std::set<int> sectors;  // sectors that are already used
    const unsigned int size = maxima.size();
    while (count_maxima != size && number_of_patterns != max_patterns) {
        std::pair<int, int> maximumbin = maxima[count_maxima].first;

        bool check = true;  // check if sector is not nearby a sector already chosen
        int sector = maximumbin.first;

        if (sectors.find(sector) != sectors.end()) { check = false; }

        if (check) {
            maximumbins.push_back(maximumbin);
            sectors.insert(maximumbin.first);

            int sectormin = sector - 1;
            int sectorplus = sector + 1;
            if (sectormin < 0) { sectormin = m_number_of_sectors; }
            if (sectorplus > m_number_of_sectors) { sectorplus = 0; }

            sectors.insert(sectormin);
            sectors.insert(sectorplus);

            if (m_number_of_sectors > 20 && maximumbin.first % 2 == 1)  // hack for new single and overlap filling curved transform!
            {
                int sectorminmin = sectormin - 1;
                int sectorplusplus = sectorplus + 1;
                sectors.insert(sectorminmin);
                sectors.insert(sectorplusplus);
            }

            count_maxima++;
            number_of_patterns++;
        } else {
            count_maxima++;
        }
    }

    return maximumbins;
}
