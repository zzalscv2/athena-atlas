/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __ITRIGINDETROADPREDICTORTOOL_H__ 
#define __ITRIGINDETROADPREDICTORTOOL_H__ 

#include <vector>
#include "GaudiKernel/IAlgTool.h"

namespace Trk {
  class SpacePoint;
}

namespace InDetDD {
  class SiDetectorElement;
}

static const InterfaceID IID_ITrigInDetRoadPredictorTool("ITrigInDetRoadPredictorTool", 1 , 0); 

class ITrigInDetRoadPredictorTool: virtual public IAlgTool 
{
 public:

  static const InterfaceID& interfaceID() {
    return IID_ITrigInDetRoadPredictorTool;
  }

  virtual int getRoad(const std::vector<const Trk::SpacePoint *> &,
                      std::vector<const InDetDD::SiDetectorElement *> &,
                      const EventContext &) const = 0;

  
};

#endif


