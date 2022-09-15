/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
#include "GeoMaterial2G4/Geo2G4MatPropTableFactory.h"
#include "G4MaterialPropertiesTable.hh"
#include "G4MaterialPropertyVector.hh"

#include "GeoModelUtilities/GeoMaterialPropertiesTable.h"
#include "GeoModelUtilities/GeoMaterialPropertyVector.h"

#include <mutex>
#include <unordered_map>

namespace {
  typedef std::unordered_map<const GeoMaterialPropertiesTable*, G4MaterialPropertiesTable*> TableMap;
}


G4MaterialPropertiesTable* Geo2G4MatPropTableFactory::Build(const GeoMaterialPropertiesTable* thePropTable)
{
  static TableMap definedTables ATLAS_THREAD_SAFE;

  // For now just use a global lock. If this turns out to be a bottleneck
  // switch to a concurrent map or similar.
  static std::mutex tableMutex;
  std::scoped_lock lock(tableMutex);

  //
  // Check if this material has already been defined.
  //
  const auto itr = definedTables.find(thePropTable);
  if(itr != definedTables.end())
    return itr->second;

  G4MaterialPropertiesTable* newTable = new G4MaterialPropertiesTable();

  // Add properties to the table ...

  // 1. Const properties
  GeoMaterialPropertiesTable::GeoMatPMap_ConstIt it1_first = thePropTable->beginPMap();
  GeoMaterialPropertiesTable::GeoMatPMap_ConstIt it1_last  = thePropTable->endPMap();

  for(;it1_first!=it1_last;it1_first++)
    newTable->AddConstProperty((it1_first->first).c_str(),it1_first->second);

  // 2. Vector properties
  GeoMaterialPropertiesTable::GeoMatPVMap_ConstIt it2_first = thePropTable->beginPVMap();
  GeoMaterialPropertiesTable::GeoMatPVMap_ConstIt it2_last  = thePropTable->endPVMap();

  for(;it2_first!=it2_last;it2_first++)
    {
      GeoMaterialPropertyVector* geoMPV = it2_first->second;
      //from G4 9.6 G4MaterialPropertyVector is now a typedef of G4PhysicsOrderedFreeVector
      G4MaterialPropertyVector* g4MPV = new G4MaterialPropertyVector();

      geoMPV->ResetIterator();

      while((*geoMPV).operator++())
        {
          //g4MPV->AddElement(geoMPV->GetPhotonMomentum(),geoMPV->GetProperty()); // G4 9.4 syntax
          //assume G4PhysicsOrderedFreeVector::InsertValues is equivalent to G4MaterialPropertyVector::AddElement
          g4MPV->InsertValues(geoMPV->GetPhotonMomentum(),geoMPV->GetProperty()); // G4 9.6 syntax
        }

      newTable->AddProperty((it2_first->first).c_str(),g4MPV);
    }

  // Save new table to the map
  definedTables[thePropTable]=newTable;

  return newTable;
}
