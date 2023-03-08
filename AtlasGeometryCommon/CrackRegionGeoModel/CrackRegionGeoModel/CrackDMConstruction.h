/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CRACKREGIONGEOMODEL_CRACKDMCONSTRUCTION_H
#define CRACKREGIONGEOMODEL_CRACKDMCONSTRUCTION_H

#include "AthenaBaseComps/AthMessaging.h"

class IRDBAccessSvc;
class IGeoModelSvc;
class StoredMaterialManager;
class GeoFullPhysVol;

class CrackDMConstruction : public AthMessaging
{
 public:
  CrackDMConstruction() = delete;
  CrackDMConstruction(IRDBAccessSvc* rdbAccess
		      , IGeoModelSvc* geoModel
		      , StoredMaterialManager* materialManager
		      , bool activateFT);
  
  void create(GeoFullPhysVol* envelope);

 private:
  IRDBAccessSvc*         m_rdbAccess{nullptr};
  IGeoModelSvc*          m_geoModel{nullptr};
  StoredMaterialManager* m_materialManager{nullptr};
  bool                   m_activateFT{false};

};

#endif
