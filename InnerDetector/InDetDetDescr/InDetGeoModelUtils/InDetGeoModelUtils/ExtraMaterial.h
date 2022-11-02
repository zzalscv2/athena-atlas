/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETGEOMODELUTILS_EXTRAMATERIAL
#define INDETGEOMODELUTILS_EXTRAMATERIAL

#include "AthenaBaseComps/AthMessaging.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include <cmath>
#include <string>
#include <sstream>

class GeoPhysVol;
class GeoFullPhysVol;
class StoredMaterialManager;

namespace InDetDD {

class DistortedMaterialManager;

class ExtraMaterial : public AthMessaging
{
public:
  ExtraMaterial(IRDBRecordset_ptr xMatTable, StoredMaterialManager * matManager);
  ExtraMaterial(InDetDD::DistortedMaterialManager * manager);
  void add(GeoPhysVol * parent, const std::string & parentName, double zPos = 0);
  void add(GeoFullPhysVol * parent, const std::string & parentName, double zPos = 0);

private:
  void add(GeoPhysVol * parent, GeoFullPhysVol * fullparent, const std::string & parentName, double zPos);
  IRDBRecordset_ptr  m_xMatTable;
  StoredMaterialManager * m_matManager;
};

} // end namespace

#endif // InDetGeoModelUtils_ExtraMaterial
