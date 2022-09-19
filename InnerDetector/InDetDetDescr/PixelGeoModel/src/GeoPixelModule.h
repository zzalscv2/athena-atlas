/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELMODULE_H
#define PIXELGEOMODEL_GEOPIXELMODULE_H
#include "Identifier/Identifier.h"
#include "GeoVPixelFactory.h"
class GeoLogVol;
class GeoPixelSiCrystal;

class GeoPixelModule : public GeoVPixelFactory {

 public:
  GeoPixelModule(InDetDD::PixelDetectorManager* ddmgr,
                 PixelGeometryManager* mgr,
		 GeoModelIO::ReadGeoModel* sqliteReader,
                 GeoPixelSiCrystal &theSensor);
  GeoPixelModule (const GeoPixelModule&) = delete;
  GeoPixelModule& operator= (const GeoPixelModule&) = delete;
  virtual ~GeoPixelModule();
  virtual GeoVPhysVol* Build() override;
  double Thickness();
  double ThicknessN();
  double ThicknessN_noSvc();
  double ThicknessP();
  double Width();
  double Length();
  double ModuleServiceThickness() const { return m_moduleSvcThickness; }
  double ModuleServiceWidth() const { return m_moduleSvcWidth; }
  Identifier getID();

 private:

  const GeoShape*  addShape(const GeoShape * lastShape, const GeoShape * nextShape, const GeoTrf::Transform3D & trans);

  const GeoLogVol* m_theModule{nullptr};
  Identifier m_id;
  GeoPixelSiCrystal& m_theSensor;
  bool m_isModule3D{false};

  double m_moduleSvcThickness{0.};
  double m_moduleSvcWidth{0.};
  int m_nbModuleSvc{0};

};

#endif
