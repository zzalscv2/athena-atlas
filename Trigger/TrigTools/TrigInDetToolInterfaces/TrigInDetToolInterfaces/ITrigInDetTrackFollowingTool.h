/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __ITRIGINDETTRACKFOLLOWINGTOOL_H__ 
#define __ITRIGINDETTRACKFOLLOWINGTOOL_H__ 

#include <vector>
#include "GaudiKernel/IAlgTool.h"

namespace Trk {
  class Track;
  class SpacePoint;
}

namespace InDetDD {
  class SiDetectorElement;
}

static const InterfaceID IID_ITrigInDetTrackFollowingTool("ITrigInDetTrackFollowingTool", 1 , 0); 

class ITrigInDetTrackFollowingTool: virtual public IAlgTool 
{
 public:

  static const InterfaceID& interfaceID() {
    return IID_ITrigInDetTrackFollowingTool;
  }

  virtual Trk::Track* getTrack(const std::vector<const Trk::SpacePoint*>&, const std::vector<const InDetDD::SiDetectorElement*>&, const EventContext&) const = 0;

  
};

#endif


