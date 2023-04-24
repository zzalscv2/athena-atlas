/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELSTAVERINGSERVICES_H
#define PIXELGEOMODEL_GEOPIXELSTAVERINGSERVICES_H

#include "GeoVPixelFactory.h"

#include "GeoPixelStaveSupport.h"
#include "GeoPixelLadder.h"

#include "GeoModelKernel/GeoPhysVol.h"
#include "InDetGeoModelUtils/GeoNodePtr.h"

class GeoTransform;

class GeoPixelStaveRingServices :  public GeoVPixelFactory {

public:  

  GeoPixelStaveRingServices(InDetDD::PixelDetectorManager* ddmgr,
			     PixelGeometryManager* mgr,
			     GeoModelIO::ReadGeoModel* sqliteReader,
                             std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                             std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
			     GeoPixelLadder& ladder,
			     GeoPixelStaveSupport& staveSupport);
  virtual GeoVPhysVol* Build() override;

  GeoPhysVol* getSupportA(){ return m_supportPhysA; }
  GeoPhysVol* getSupportC(){ return m_supportPhysC; }
  GeoVPhysVol* getSupportMidRing(){ return m_supportMidRing; }

  GeoTransform* getSupportTrfA(){ return m_xformSupportA; }
  GeoTransform* getSupportTrfC(){ return m_xformSupportC; }
  GeoTransform* getSupportTrfMidRing(){ return m_xformSupportMidRing; }

 private:

  GeoPixelLadder& m_ladder;
  GeoPixelStaveSupport& m_staveSupport;  

  GeoNodePtr<GeoPhysVol>  m_supportPhysA;
  GeoNodePtr<GeoPhysVol>  m_supportPhysC;
  GeoNodePtr<GeoVPhysVol> m_supportMidRing;

  GeoNodePtr<GeoTransform> m_xformSupportA;
  GeoNodePtr<GeoTransform> m_xformSupportC;
  GeoNodePtr<GeoTransform> m_xformSupportMidRing;


};

#endif

