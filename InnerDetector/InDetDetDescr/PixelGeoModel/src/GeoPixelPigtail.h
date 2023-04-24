/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELPIGTAIL_H
#define GEOPIXELPIGTAIL_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelPigtail : public GeoVPixelFactory {
 public:
  GeoPixelPigtail(InDetDD::PixelDetectorManager* ddmgr,
                  PixelGeometryManager* mgr,
		  GeoModelIO::ReadGeoModel* sqliteReader, 
                  std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                  std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;

 private:
  
};

#endif
