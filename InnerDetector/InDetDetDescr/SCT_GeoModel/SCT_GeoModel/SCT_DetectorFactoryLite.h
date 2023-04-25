/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_GEOMODEL_SCT_DETECTORFACTORYLITE_H
#define SCT_GEOMODEL_SCT_DETECTORFACTORYLITE_H
 
#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h" 
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "CxxUtils/checker_macros.h"
#include "GeoModelKernel/GeoVDetectorFactory.h"

#include <memory>
#include <map>
#include <string>

class GeoPhysVol;
class GeoFullPhysVol;
class GeoAlignableTransform;

class SCT_DataBase;
class SCT_GeometryManager;
class SCT_GeoModelAthenaComps;
class SCT_MaterialManager;
class SCT_Options;

namespace GeoModelIO {
  class ReadGeoModel;
}

class SCT_DetectorFactoryLite : public InDetDD::DetectorFactoryBase
{ 
  
 public: 
  // Constructor
  SCT_DetectorFactoryLite(GeoModelIO::ReadGeoModel *sqliteReader,SCT_GeoModelAthenaComps * athenaComps,
		      const SCT_Options & options);

  // Destructor
  virtual ~SCT_DetectorFactoryLite();

  // Creation of geometry:
  virtual void create(GeoPhysVol *world);   

  // Access to the results: 
  virtual const InDetDD::SCT_DetectorManager * getDetectorManager() const; 

 private: 
  // Copy and assignments operations illegal and so are made private
  SCT_DetectorFactoryLite(const SCT_DetectorFactoryLite &right);
  const SCT_DetectorFactoryLite & operator=(const SCT_DetectorFactoryLite &right);

  // private member data:
  GeoModelIO::ReadGeoModel    *m_sqliteReader;
  InDetDD::SCT_DetectorManager *m_detectorManager;
  std::unique_ptr<SCT_GeometryManager> m_geometryManager;
  std::unique_ptr<SCT_DataBase> m_db;
  std::unique_ptr<SCT_MaterialManager> m_materials;
  bool m_useDynamicAlignFolders;

  std::shared_ptr<std::map<std::string, GeoFullPhysVol*>>        m_mapFPV;
  std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> m_mapAX;


}; 
 
#endif 
 
