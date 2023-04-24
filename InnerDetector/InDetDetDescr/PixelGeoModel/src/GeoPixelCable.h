/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELCABLE_H
#define GEOPIXELCABLE_H

#include "GeoVPixelFactory.h"
#include <vector>

class GeoPixelCable : public GeoVPixelFactory {
 public:
  GeoPixelCable(InDetDD::PixelDetectorManager* ddmgr,
                PixelGeometryManager* mgr,
		GeoModelIO::ReadGeoModel* sqliteReader,
                std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;
  int numElements() const {return m_elements.size();}
  void setElement(int i) {m_currentElement = i;}
  double getStackOffset();
  double zpos();

 private:
  int m_currentElement;
  std::vector<int> m_elements;
  void fillElements(int layer, int phiSector);

};



#endif
