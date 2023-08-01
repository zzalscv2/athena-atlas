/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Spacer_H
#define Spacer_H

#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/SpaComponent.h"

class GeoVPhysVol;

namespace MuonGM {
    class MYSQL;

    class Spacer : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers

        Spacer(const MYSQL& mysql, Component *s);
        GeoVPhysVol *build(StoredMaterialManager& matManager);
        GeoVPhysVol *build(StoredMaterialManager& matManager, int cutoutson);
        virtual void print() const override;

      private:
        SpaComponent m_component{};
    };

} // namespace MuonGM

#endif
