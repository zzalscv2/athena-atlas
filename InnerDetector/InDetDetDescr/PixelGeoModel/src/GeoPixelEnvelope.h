/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELENVELOPE_H
#define PIXELGEOMODEL_GEOPIXELENVELOPE_H

#include "GeoVPixelFactory.h"

class GeoPixelEnvelope : public GeoVPixelFactory {
 public:
  using GeoVPixelFactory::GeoVPixelFactory;
  virtual GeoVPhysVol* Build() override;
};

#endif
