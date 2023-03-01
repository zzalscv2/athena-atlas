/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughTransformSteering.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "MuonHoughPatternEvent/MuonHoughPattern.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer.h"

MuonHoughTransformSteering::MuonHoughTransformSteering(std::unique_ptr<MuonHoughTransformer> houghtransformer) :
    AthMessaging{"MuonHoughTransformSteering"},
    m_houghtransformer{std::move(houghtransformer)} {}

MuonHoughTransformSteering::~MuonHoughTransformSteering() = default;

MuonHoughPatternCollection MuonHoughTransformSteering::constructHoughPatterns(const MuonHoughHitContainer& event, double residu_mm,
                                                                              double residu_grad, int max_patterns) const {
    MuonHoughPatternCollection houghpatterns;
    houghpatterns.reserve(max_patterns);
    std::vector<std::pair<int, int> > maxima = m_houghtransformer->getMaxima(max_patterns);  // sector,binnumber , sorted vector

    for (const auto& [sector, binnumber] : maxima) {
        if (binnumber == -1) {
            ATH_MSG_VERBOSE("binnumber == -1 (no max found), max patterns = " << max_patterns );            
            break;
        }
        std::unique_ptr<MuonHoughPattern> houghpattern = constructHoughPattern(event, binnumber, residu_mm, 
                                                                               residu_grad, sector);
        houghpatterns.emplace_back(std::move(houghpattern));
    }

    // subtract all hits that were just added to a pattern
    // m_houghtransformer->fill(event,true);

    return houghpatterns;
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformSteering::constructHoughPattern(const MuonHoughHitContainer& event, double residu_mm,
                                                                    double residu_grad, int maximum_number) const {
    ATH_MSG_DEBUG("MuonHoughTransformSteering::constructHoughPattern (start) ");
   
    std::unique_ptr<MuonHoughPattern> houghpattern = m_houghtransformer->associateHitsToMaximum(event, residu_mm, residu_grad, 
                                                                                                maximum_number);

    ATH_MSG_DEBUG("MuonHoughTransformSteering::constructHoughPattern (end) ");   
    
    return houghpattern;
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformSteering::constructHoughPattern(const MuonHoughHitContainer& event, std::pair<double, double> coords,
                                                                    double residu_mm, double  residu_grad, int sector) const {
    return m_houghtransformer->associateHitsToCoords(event, coords, residu_mm, residu_grad, sector);   
}

std::unique_ptr<MuonHoughPattern> MuonHoughTransformSteering::constructHoughPattern(const MuonHoughHitContainer& event, int binnumber, double residu_mm,
                                                                    double residu_grad, int sector) const {
    return m_houghtransformer->associateHitsToBinnumber(event, binnumber, residu_mm, residu_grad, sector);
}   

void MuonHoughTransformSteering::fill(const MuonHoughHitContainer& event) { m_houghtransformer->fill(event); }

void MuonHoughTransformSteering::resetHisto() { m_houghtransformer->resetHisto(); }
