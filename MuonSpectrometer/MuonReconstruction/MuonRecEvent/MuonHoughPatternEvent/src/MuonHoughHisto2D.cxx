/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughHisto2D.h"

#include "TFile.h"
#include "TH2F.h"

MuonHoughHisto2D::MuonHoughHisto2D(int nbinsx, double xmin, double xmax, int nbinsy, double ymin, double ymax, int number_of_maxima) :
    AthMessaging("MuonHoughHisto2D"),
    m_nbinsx(nbinsx),
    m_xmin(xmin),
    m_xmax(xmax),
    m_nbinsy(nbinsy),
    m_ymin(ymin),
    m_ymax(ymax),
    m_number_of_maxima(number_of_maxima),
    m_scale(10000),
    m_threshold(2.1 * m_scale),
    m_distance_to_next_maximum(100) {
    m_nbinsx_plus2 = nbinsx + 2;
    m_nbinsy_plus2 = nbinsy + 2;
    m_size = m_nbinsx_plus2 * m_nbinsy_plus2;

    m_binwidthx = (m_xmax - m_xmin) / (m_nbinsx + 0.);
    m_binwidthy = (m_ymax - m_ymin) / (m_nbinsy + 0.);

    m_invbinwidthx = 1. / m_binwidthx;
    m_invbinwidthy = 1. / m_binwidthy;
    init();
}

void MuonHoughHisto2D::init() { m_histBuffer.reset( new unsigned int[m_size]); }

void MuonHoughHisto2D::reset() {
    resetHisto();

    m_bins_above_threshold.clear();
    m_maximumbins.clear();  // obsolete?
    m_maxima_found = 0;
    m_maximumBin = -1;
    m_maximum = 0;
    m_maximumIsValid = true;
}

void MuonHoughHisto2D::findMaxima() {
    m_maximumbins.clear();  // clear old m_maxima_vector and start searching again

    int maximum_number = 0;

    while (maximum_number < m_number_of_maxima) {
        double maximum = -1.;
        //coverity defect 13671: max bin was set to -1 initially, but this is sued as an index into the array
        int maxbin = 0;

        ATH_MSG_VERBOSE("MuonHoughHisto2D::size bins above threshold: " << m_bins_above_threshold.size());
      
        for (const int bin_num :m_bins_above_threshold) {
            if (!checkIfMaximumAlreadyUsed(bin_num)) {
                checkIfMaximum(bin_num, maximum, maxbin);  // call by ref
            }
        }

        if (maximum < m_threshold) {
            ATH_MSG_DEBUG("MuonHoughHisto2D:: no maximum found");
            break;
        } else {
            ATH_MSG_DEBUG("MuonHoughHisto2D:: Maximum found: " << maximum << " binnumber: " << maxbin << " R (z) " << binnumberToCoords(maxbin).first
                    << " angle " << binnumberToCoords(maxbin).second);
            m_maximumbins.push_back(maxbin);
        }  // maxbin <> m_threshold

        if (maximum == -1.) {
            ATH_MSG_VERBOSE("MuonHoughHisto2D::No Bins Above Threshold");
        } else {
            std::pair<double, double> coords = binnumberToCoords(maxbin);
            ATH_MSG_VERBOSE("MuonHoughHisto2D::Maximum Number: " << maximum_number << " Maximum: " << maximum
                << " binnumber: " << maxbin << " x: " << coords.first << " y: " << coords.second);
        }    
        maximum_number++;
    }  // number_of_maxima

    m_maxima_found = 1;
}

std::pair<int, double> MuonHoughHisto2D::getMax() const {
    std::pair<int, double> maxpair;
    int maxbin = -1;  // convention! for no bin above threshold
    unsigned int maximum = m_threshold;

    if (m_maximumIsValid) {
        if (m_maximum > m_threshold) {
            maxbin = m_maximumBin;
            maximum = m_maximum;
        }
    } else {
        for (unsigned int i = 0; i < m_size; i++) {
            if (m_histBuffer[i] > maximum) {
                maximum = m_histBuffer[i];
                maxbin = i;
            }
        }
    }
    maxpair.first = maxbin;
    maxpair.second = maximum;

    return maxpair;
}

std::pair<int, double> MuonHoughHisto2D::getMaximumBin(unsigned int maximum_number) {
    if (m_maxima_found == 0) {
        findMaxima();  // fills m_maximumbins
    }

    int maxbin = -1;
    double maximum = -1.;

    ATH_MSG_VERBOSE("MuonHoughHisto2D:: m_maximumbins.size: " << m_maximumbins.size());
    

    if (m_maximumbins.size() > maximum_number) {
        maxbin = m_maximumbins[maximum_number];
        maximum = content_Bin_Area(maxbin);
    }
    return std::make_pair(maxbin, maximum);

}  // getMaximumBin

std::pair<double, double> MuonHoughHisto2D::getCoordsMaximum(unsigned int maximum_number) {
    std::pair<double, double> coordsmaximum;
    int binnumber = getMaxBin(maximum_number);

    if (binnumber != -1) {
        coordsmaximum = binnumberToCoords(binnumber);
    } else {
        ATH_MSG_WARNING("HoughTransform::No Maximum Found");
        coordsmaximum.first = 99999.;
        coordsmaximum.second = 99999.;
    }
    return coordsmaximum;
}

bool MuonHoughHisto2D::checkIfMaximumAlreadyUsed(int binnumber) const {
    bool check = false;

    for (unsigned int i = 0; i < m_maximumbins.size(); i++) {
        if (distanceBins(binnumber, m_maximumbins[i]) < m_distance_to_next_maximum) {
            check = true;
            return check;
        }
    }

    return check;
}

bool MuonHoughHisto2D::checkIfMaximum(int binnumber, double& maximum, int& maxbin) const {
    bool check = false;

    double content_bin_area = content_Bin_Area(binnumber);  // now no area anymore == getBinContent

    // when using negative weights the following can happen:
    if (content_bin_area < m_threshold) return check;

    if (content_bin_area == maximum) {
        if (getBinContent(maxbin) > getBinContent(binnumber))  // give preference to maximum with peak ( _U_ )
        {
            check = true;
            maximum = content_bin_area;
            maxbin = binnumber;
        }
    } else if (content_bin_area > maximum) {
        check = true;
        maximum = content_bin_area;
        maxbin = binnumber;
    }
    return check;
}  // checkIfMaximum

int MuonHoughHisto2D::distanceBins(int binnumber1, int binnumber2) const {
    int binnumberdistance = std::abs(binnumber1 - binnumber2);

    // Manhattan metric:
    int distance = (binnumberdistance % m_nbinsx_plus2) + (binnumberdistance / m_nbinsx_plus2);

    return distance;
}

std::pair<double, double> MuonHoughHisto2D::binnumberToCoords(int binnumber, int /*printLevel*/) const {
    std::pair<double, double> coordsbin;
    if (binnumber < 0) {
        ATH_MSG_WARNING("MuonHoughHisto2D::ERROR: negativebinnumber: " << binnumber);
        coordsbin.first = 99999.;
        coordsbin.second = 99999.;
        return coordsbin;
    }

    double xcoord = binnumberToXCoord(binnumber);
    double ycoord = binnumberToYCoord(binnumber);

    ATH_MSG_VERBOSE("MuonHoughHisto2D::Maximum: " << getBinContent(binnumber) << " binnumber: " << binnumber << " x: " << xcoord
                    << " y: " << ycoord);
    

    coordsbin.first = xcoord;
    coordsbin.second = ycoord;
    return coordsbin;
}

int MuonHoughHisto2D::binInHistogram(unsigned int binnumber) const {
    int bininhisto = 0;

    if ((binnumber) % m_nbinsx_plus2 == 0) {
        bininhisto = 1;
    } else if ((binnumber + 1) % m_nbinsx_plus2 == 0) {
        bininhisto = 2;
    } else if (binnumber <= m_nbinsx_plus2) {
        bininhisto = 3;
    } else if (binnumber >= m_nbinsx_plus2 * (getNbinsY() + 1)) {
        bininhisto = 4;
    }

    return bininhisto;
}

std::unique_ptr<TH2F> MuonHoughHisto2D::bookAndFillRootHistogram(const std::string& hname) const {
    std::unique_ptr<TH2F> histogram = std::make_unique<TH2F>(hname.c_str(), hname.c_str(), m_nbinsx, m_xmin, m_xmax, m_nbinsy, m_ymin, m_ymax);
    for (unsigned int i = 0; i < m_size; i++) {
        int ix = i % m_nbinsx_plus2;
        int iy = i / m_nbinsx_plus2;
        histogram->SetBinContent(ix, iy, m_histBuffer[i] / (double)m_scale);
    }
    return histogram;
}
