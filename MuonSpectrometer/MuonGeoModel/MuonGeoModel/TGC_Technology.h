/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGC_H
#define TGC_H

#include "MuonGeoModel/Technology.h"

#include <vector>
namespace MuonGM {
    class MYSQL;

    class TGC : public Technology {
      public:
        //	double thickness;
        int nlayers{0};
        double frame_h{0.};
        double frame_ab{0.};
        std::vector<std::string> materials{};
        std::vector<double> positions{};
        std::vector<double> tck{};

        // inner structure parameters from GGLN
        // For wire supports
        double widthWireSupport{0.};     // width of wire support, GGLN/S1PP
        double widthGasChannel{0.};      // not used, GGLN/S2PP
        double distanceWireSupport{0.};  // distance between two neigbouring wire supports, GGLN/WSEP
       std::array<double, 3> offsetWireSupport{}; // offset w.r.t the chamber centre axis for each layer, GGLN/SP[1-3]WI
        double angleTilt{0.};            // tilt angle of wire support, GGLN/TILT
        // For button supports
        double radiusButton{0.};   // radius of a button support, GGLN/SP1BU
        std::array<double, 2> pitchButton{}; // pitch in y and z axies, GGLN/SP[2,3]BU
        double angleButton{0.};    // tilt angle in trapezoid regions, GGLN/SP4BU

        inline TGC(MYSQL& mysql, const std::string& s);
    };

    TGC::TGC(MYSQL& mysql, const std::string& s)
        : Technology(mysql, s) {}
} // namespace MuonGM

#endif
