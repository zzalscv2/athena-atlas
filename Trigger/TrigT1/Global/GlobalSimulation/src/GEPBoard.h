//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#ifndef GLOBALSIM_GEPBOARD_H
#define GLOBALSIM_GEPBOARD_H

#include "IGlobalAlg.h"

#include <vector>
#include <memory>

/* In GlobalSim, GEPBoard represents the hardware GEPBoard in tht
 *  it constains Algorithms which are be run.
 *  Algorithms read from and write to a DataRepository which
 * plays the role of hardware IO FIFOs
 */
   

namespace TCS {
  class InputEvent;
}

namespace GlobalSim {

  class DataRepository;
  
  class GEPBoard {
  public:
    GEPBoard(const std::vector<std::shared_ptr<IGlobalAlg>>&);
    bool run(DataRepository&,
	     const TCS::TopoInputEvent& );
	     
  private:
    std::vector<std::shared_ptr<IGlobalAlg>> m_gsAlgs;
  };
}
#endif
