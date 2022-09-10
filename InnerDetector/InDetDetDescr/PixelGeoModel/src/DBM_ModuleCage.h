/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_DBMMODULECAGE_H
#define PIXELGEOMODEL_DBMMODULECAGE_H

/**
 * @class DBM_ModuleCage
 * @brief The module cage is the structure forming of
 *        the three aluminium plates and rods,
 *        which support the DBM module.
 **/

#include "GeoVPixelFactory.h"

class DBM_ModuleCage : public GeoVPixelFactory {
 public:
  using GeoVPixelFactory::GeoVPixelFactory;
  virtual GeoVPhysVol* Build() override;
};

#endif
