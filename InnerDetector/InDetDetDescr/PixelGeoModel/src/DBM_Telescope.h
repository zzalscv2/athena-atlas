/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef PIXELGEOMODEL_DBMTELESCOPE_H
#define PIXELGEOMODEL_DBMTELESCOPE_H

/**
 * @class DBM_Telescope
 * @brief Diamond Beam Monitor telescope builder
 */

#include "GeoVPixelFactory.h"

class DBM_Telescope : public GeoVPixelFactory {
 public:
  using GeoVPixelFactory::GeoVPixelFactory;
  virtual GeoVPhysVol* Build() override;
};

#endif
