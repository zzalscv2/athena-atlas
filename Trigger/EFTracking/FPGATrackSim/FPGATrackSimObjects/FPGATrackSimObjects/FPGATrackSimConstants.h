/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_CONSTANTS_H
#define TRIGFPGATrackSimOBJECTS_CONSTANTS_H


#include <array>
#define MTX_TOLERANCE 3e-16

namespace htt {

    // "Idealised" radius values for each of the detector barrel layers
    constexpr std::array< double, 8 >  TARGET_R_1STAGE = { 290.516, 396.066, 558.552, 564.953, 758.321, 764.665, 996.384, 1002.72 };
    constexpr std::array< double, 13 > TARGET_R_2STAGE = { 33.3024, 99.1959, 159.543, 227.638, 290.516, 396.066, 402.463, 558.552, 564.953, 758.321, 764.665, 996.384, 1002.72 };

    constexpr std::array< double, 16 > QOVERPT_BINS = { -0.001, -0.0009, -0.00075, -0.0006, -0.00045, -0.0003, -0.00015, -0.000075, 0.000075, 0.00015, 0.0003, 0.00045, 0.0006, 0.00075, 0.0009, 0.001};

    // --- This is the current FPGATrackSimCluster to FPGATrackSimHit scaling factor --- //
    constexpr float scaleHitFactor = 2;
    constexpr float DEG_TO_RAD = 0.017453292519943295;

    static constexpr double A = 0.0003; // for Hough Transform

}

#endif // TRIGFPGATrackSimOBJECTS_CONSTANTS_H
