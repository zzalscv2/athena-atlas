/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_PIXELDETECTORFACTORYLITE_H
#define PIXELGEOMODEL_PIXELDETECTORFACTORYLITE_H 

#include <map>
#include <string>

#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h" 

#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "CxxUtils/checker_macros.h"

class GeoFullPhysVol;
class GeoAlignableTransform;

class PixelSwitches;
class PixelGeometryManager;
class PixelGeoModelAthenaComps;

namespace GeoModelIO {
  class ReadGeoModel;
}

class PixelDetectorFactoryLite : public InDetDD::DetectorFactoryBase {

 public:
  
  PixelDetectorFactoryLite(GeoModelIO::ReadGeoModel* sqliteReader
			   , PixelGeoModelAthenaComps* athenaComps
			   , const PixelSwitches& switches);
  PixelDetectorFactoryLite() = delete;
  PixelDetectorFactoryLite(const PixelDetectorFactoryLite &right) = delete;
  const PixelDetectorFactoryLite & operator=(const PixelDetectorFactoryLite &right) = delete;

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;
  
  // Access to the results:
  virtual const InDetDD::PixelDetectorManager* getDetectorManager() const override{ return m_detectorManager; }

 private:  
  GeoModelIO::ReadGeoModel*             m_sqliteReader{nullptr};
  InDetDD::PixelDetectorManager*        m_detectorManager{nullptr}; //ownership handed to caller
  std::unique_ptr<PixelGeometryManager> m_geometryManager;

  bool m_useDynamicAlignFolders{false};
  std::shared_ptr<std::map<std::string, GeoFullPhysVol*>>        m_mapFPV;
  std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> m_mapAX;

  void doChecks();
};

#endif
