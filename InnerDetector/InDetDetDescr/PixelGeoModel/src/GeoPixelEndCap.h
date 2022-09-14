/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELENDCAP_H
#define PIXELGEOMODEL_GEOPIXELENDCAP_H

#include "GeoVPixelFactory.h"
class GeoPixelServices;

class GeoPixelEndCap : public GeoVPixelFactory {
 public:
  GeoPixelEndCap(InDetDD::PixelDetectorManager* ddmgr
                 , PixelGeometryManager* mgr
		 , GeoModelIO::ReadGeoModel* sqliteReader
                 , GeoPixelServices * pixServices);
  virtual GeoVPhysVol* Build() override;
 private:
  GeoPixelServices * m_pixServices;
};

#endif
