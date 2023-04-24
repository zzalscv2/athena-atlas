/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELFRAME_H
#define GEOPIXELFRAME_H

#include "GeoVPixelFactory.h"

class GeoPixelFrame : public GeoVPixelFactory {

public:  
  GeoPixelFrame(InDetDD::PixelDetectorManager* ddmgr,
                PixelGeometryManager* mgr, GeoModelIO::ReadGeoModel* sqliteReader,
                std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  void BuildAndPlace(GeoFullPhysVol * parent, int section);

private:
  virtual GeoVPhysVol* Build() override {return 0;} // unused - but satisfy interface
  bool same(double v1, double v2);
};

#endif

