/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELIBLFWDSVC_CADMODEL_H
#define GEOPIXELIBLFWDSVC_CADMODEL_H

#include "GeoVPixelFactory.h"

#include "GeoPixelStaveSupport.h"
#include "GeoPixelLadder.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoPhysVol.h"

class GeoTransform;

class GeoPixelIBLFwdSvcCADModel :  public GeoVPixelFactory {

public:  

  GeoPixelIBLFwdSvcCADModel(InDetDD::PixelDetectorManager* ddmgr,
                            PixelGeometryManager* mgr, 
                            GeoModelIO::ReadGeoModel* sqliteReader,
                            std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                            std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
                            int);
  virtual GeoVPhysVol* Build() override;

  GeoPhysVol* getSupportA() { return m_supportPhysA; }
  GeoPhysVol* getSupportC() { return m_supportPhysC; }
  GeoTransform* getSupportTrfA() { return m_xformSupportA; }
  GeoTransform* getSupportTrfC() { return m_xformSupportC; }

 private:

  GeoPhysVol *m_supportPhysA;
  GeoPhysVol *m_supportPhysC;
  GeoTransform *m_xformSupportA;
  GeoTransform *m_xformSupportC;

  const GeoShape* addShape(const GeoShape * lastShape, const GeoShape * nextShape, const GeoTrf::Transform3D & trans);
};

#endif

