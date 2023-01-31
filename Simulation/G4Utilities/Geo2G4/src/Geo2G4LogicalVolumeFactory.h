/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_Geo2G4LogicalVolumeFactory_h
#define GEO2G4_Geo2G4LogicalVolumeFactory_h

#include "CxxUtils/checker_macros.h"

class G4LogicalVolume;
class GeoLogVol;

class Geo2G4LogicalVolumeFactory {
public:
  Geo2G4LogicalVolumeFactory();
  G4LogicalVolume* Build ATLAS_NOT_THREAD_SAFE (const GeoLogVol*);
};

#endif
