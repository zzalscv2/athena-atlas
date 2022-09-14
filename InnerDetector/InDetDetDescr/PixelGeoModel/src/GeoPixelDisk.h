/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELDISK_H
#define PIXELGEOMODEL_GEOPIXELDISK_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelDisk : public GeoVPixelFactory {
 public:
  GeoPixelDisk(InDetDD::PixelDetectorManager* ddmgr
	       , PixelGeometryManager* mgr
	       , GeoModelIO::ReadGeoModel* sqliteReader);
  virtual GeoVPhysVol* Build() override;
  double Thickness();
  double RMax();
  double RMin();
};

#endif
