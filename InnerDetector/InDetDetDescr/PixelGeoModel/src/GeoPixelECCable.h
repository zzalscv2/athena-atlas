/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELECCABLE_H
#define GEOPIXELECCABLE_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelECCable : public GeoVPixelFactory {
 public:
  GeoPixelECCable(InDetDD::PixelDetectorManager* ddmgr,
                  PixelGeometryManager* mgr,
		  GeoModelIO::ReadGeoModel* sqliteReader,
                  std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                  std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;
};

#endif
