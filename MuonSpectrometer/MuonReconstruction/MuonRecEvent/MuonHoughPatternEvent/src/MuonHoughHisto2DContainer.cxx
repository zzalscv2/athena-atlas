/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughHisto2DContainer.h"

MuonHoughHisto2DContainer::MuonHoughHisto2DContainer() = default;

std::pair<int, int> MuonHoughHisto2DContainer::getMaximumBinnumber() const {
   
    double maximum{0.};
    int maxid{-1}, maxbin{-1};
    for (int histoid = 0; histoid < size(); histoid++) {
        std::pair<int, double> histomax =
            getHisto(histoid)->getMaximumBin(0);  // binnumber and value of the area of the bin

        if (histomax.second > maximum) {
            maximum = histomax.second;
            maxid = histoid;
            maxbin = histomax.first;
        }
    }
    return std::make_pair(maxid, maxbin);
}

void MuonHoughHisto2DContainer::reset() const {
    for (int histoid = 0; histoid < size(); histoid++) { getHisto(histoid)->reset(); }
}
