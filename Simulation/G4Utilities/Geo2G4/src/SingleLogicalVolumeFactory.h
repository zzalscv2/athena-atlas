/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_SingleLogicalVolumeFactory_h
#define GEO2G4_SingleLogicalVolumeFactory_h

#include "AthenaBaseComps/AthMessaging.h"
#include "CxxUtils/checker_macros.h"

class G4LogicalVolume;
class GeoLogVol;

#include <string>
#include <map>

class SingleLogicalVolumeFactory : public AthMessaging {
public:
  SingleLogicalVolumeFactory();
  G4LogicalVolume* Build ATLAS_NOT_THREAD_SAFE (const GeoLogVol*);
};

#endif
