/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_DBMDET_H
#define PIXELGEOMODEL_DBMDET_H

/**
 * @class DBM_Det
 * @brief Diamond Beam Monitor detector builder
 **/

#include "GeoVPixelFactory.h"

class DBM_Det : public GeoVPixelFactory {
 public:
  DBM_Det(InDetDD::PixelDetectorManager* ddmgr,
	   PixelGeometryManager* mgr,
	   GeoModelIO::ReadGeoModel* sqliteReader,
           std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
           std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual  GeoVPhysVol* Build() override;
  
 private:
  std::vector<double> m_module[4];
};

#endif
