/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELDISKSUPPORTS_H
#define GEOPIXELDISKSUPPORTS_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelDiskSupports : public GeoVPixelFactory {

 public:
  GeoPixelDiskSupports(InDetDD::PixelDetectorManager* ddmgr,
                       PixelGeometryManager* mgr,
		       GeoModelIO::ReadGeoModel* sqliteReader,
                       std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                       std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;

  int NCylinders(){return m_rmin.size();}
  void SetCylinder(int n) {m_nframe = n;}
  double ZPos() {return m_zpos[m_nframe];}

 private:
  std::vector<double> m_rmin,m_rmax,m_halflength,m_zpos;
  std::vector<int> m_typeNum;
  int m_nframe;

};

#endif
