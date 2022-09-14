/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMATERIAL2G4_Geo2G4MatPropTableFactory_h
#define GEOMATERIAL2G4_Geo2G4MatPropTableFactory_h

class G4MaterialPropertiesTable;
class GeoMaterialPropertiesTable;

class Geo2G4MatPropTableFactory {
public:
  G4MaterialPropertiesTable* Build(const GeoMaterialPropertiesTable*);
};

#endif
