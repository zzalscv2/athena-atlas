/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELOLDFRAME_H
#define GEOPIXELOLDFRAME_H

#include "GeoVPixelFactory.h"
class GeoLogVol;
class GeoFullPhysVol;
class GeoVPhysVol;
class PixelLegacyManager;

class GeoPixelOldFrame : public GeoVPixelFactory {
 public:
  GeoPixelOldFrame(InDetDD::PixelDetectorManager* ddmgr,
                   PixelGeometryManager* mgr,
		   GeoModelIO::ReadGeoModel* sqliteReader,
                   std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                   std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;

  void BuildInBarrel(GeoFullPhysVol * parent);
  void BuildOutBarrel(GeoFullPhysVol * parent);

private:  
  GeoVPhysVol* BuildBox();
  GeoVPhysVol* BuildTrap();

  PixelLegacyManager * m_legacyManager;

};

#endif
