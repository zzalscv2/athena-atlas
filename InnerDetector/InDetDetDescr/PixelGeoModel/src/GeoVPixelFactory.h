/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOVPIXELFACTORY_H
#define PIXELGEOMODEL_GEOVPIXELFACTORY_H

/**
 * @class GeoVPixelFactory
 * @brief This is the base class for all the pieces of the Pixel detector.
 **/

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "PixelGeometryManager.h"
#include "InDetGeoModelUtils/InDetMaterialManager.h"
#include "CxxUtils/checker_macros.h"

// fwd declaration
namespace InDetDD {class PixelDetectorManager;}

class GeoVPixelFactory {
 public:
  GeoVPixelFactory(InDetDD::PixelDetectorManager* ddmgr
		   , PixelGeometryManager* mgr
		   , GeoModelIO::ReadGeoModel* sqliteReader);
  virtual ~GeoVPixelFactory();
  virtual GeoVPhysVol* Build()=0;
     
 protected:
  PixelGeometryManager* m_gmt_mgr;
  InDetMaterialManager* m_mat_mgr;
  InDetDD::PixelDetectorManager* m_DDmgr;
  GeoModelIO::ReadGeoModel* m_sqliteReader;
  const double m_epsilon;
};


#endif
