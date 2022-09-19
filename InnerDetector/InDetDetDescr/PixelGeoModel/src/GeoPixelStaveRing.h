/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELSTAVERING_H
#define PIXELGEOMODEL_GEOPIXELSTAVERING_H

#include "GeoVPixelFactory.h"

class GeoPixelStaveRing :  public GeoVPixelFactory {

public:  
  GeoPixelStaveRing(InDetDD::PixelDetectorManager* ddmgr,
                    PixelGeometryManager* mgr,
		    GeoModelIO::ReadGeoModel* sqliteReader);
  virtual GeoVPhysVol* Build() override;

  GeoVPhysVol* SetParametersAndBuild(const std::string&,const std::string&);
  double GetPositionAlongZAxis() const { return m_zPosition; }

  double GetInnerRadius() const { return m_innerRadius; }
  double GetOuterRadius() const { return m_outerRadius; }

private:
  GeoVPhysVol* m_physVol;
  double m_zPosition;
  double m_innerRadius, m_outerRadius;
  std::string m_ringPosition;
  std::string m_ringName;

};

#endif

