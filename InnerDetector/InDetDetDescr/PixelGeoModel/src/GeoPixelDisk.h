/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELDISK_H
#define PIXELGEOMODEL_GEOPIXELDISK_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelDisk : public GeoVPixelFactory {
 public:
  GeoPixelDisk(InDetDD::PixelDetectorManager* ddmgr,
	       PixelGeometryManager* mgr,
	       GeoModelIO::ReadGeoModel* sqliteReader,
               std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
               std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;
  double Thickness();
  double RMax();
  double RMin();
};

#endif
