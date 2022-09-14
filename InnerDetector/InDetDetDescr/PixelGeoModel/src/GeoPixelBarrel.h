/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELBARREL_H
#define PIXELGEOMODEL_GEOPIXELBARREL_H

#include "GeoVPixelFactory.h"
class GeoPixelServices;

class GeoPixelBarrel : public GeoVPixelFactory {
 public:
  GeoPixelBarrel(InDetDD::PixelDetectorManager* ddmgr
                 , PixelGeometryManager* mgr
		 , GeoModelIO::ReadGeoModel* sqliteReader
		 , GeoPixelServices * pixServices);
  virtual GeoVPhysVol* Build() override;
 private:
  GeoPixelServices * m_pixServices;
};

#endif
