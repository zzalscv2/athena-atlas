/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMSTEERING_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMSTEERING_H

#include "MuonHoughPatternEvent/MuonHoughHisto2DContainer.h"
#include "MuonHoughPatternEvent/MuonHoughPatternCollection.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer.h"
#include <AthenaBaseComps/AthMessaging.h>

class MuonHoughPattern;

class MuonHoughTransformSteering: public AthMessaging {
    /** Class is build as Strategy, Context pattern */

public:
    /** constructor */
    MuonHoughTransformSteering(std::unique_ptr<MuonHoughTransformer>);
    /** destructor */
    ~MuonHoughTransformSteering();

    /** construct hough patterns
     * @param[in] event Hitcontainer
     * @param[in] residu_mm maximum residu for hit to be associated to pattern
     * @param[in] residu_grad maximum residu for hit to be associated to pattern
     * @param[in] max_patterns maximum number of patterns to be built
     * @param[in] which_segment upper (1) or lower (0) segment, this option is off by default
     * @param[out] HoughPatternCollection
     */
    MuonHoughPatternCollection constructHoughPatterns(const MuonHoughHitContainer& event, double residu_mm, double residu_grad,
                                                      int max_patterns) const;
    /** construct hough pattern on a certain maxima number of histogram */
    std::unique_ptr<MuonHoughPattern> constructHoughPattern(const MuonHoughHitContainer& event, double residu_mm, double residu_grad,
                                            int maximum_number) const;
    /** construct hough pattern at a certain coordinate (maximum) in certain sector*/
    std::unique_ptr<MuonHoughPattern> constructHoughPattern(const MuonHoughHitContainer& event, std::pair<double, double> coordsmaximum, double residu_mm,
                                            double residu_grad, int sector) const;
    /** construct hough pattern at a certain binnumber (maximum) in certain sector*/
    std::unique_ptr<MuonHoughPattern> constructHoughPattern(const MuonHoughHitContainer& event, int binnumber, double residu_mm, double residu_grad,
                                            int sector) const;

    /** fill histograms */
    void fill(const MuonHoughHitContainer& event);

    /** reset histograms */
    void resetHisto();

    /** access to histograms */
    const MuonHoughHisto2DContainer& histos() const { return m_houghtransformer->histos(); }

    const MuonHoughTransformer& transformer() const { return *m_houghtransformer; }

private:
    /** the actual houghtransform */
    std::unique_ptr<MuonHoughTransformer> m_houghtransformer{};

};

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHTRANSFORMSTEERING_H
