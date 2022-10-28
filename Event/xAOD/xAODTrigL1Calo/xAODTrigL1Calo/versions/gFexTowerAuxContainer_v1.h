/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALO_VERSIONS_GFEXTOWERAUXCONTAINER_V1_H
#define XAODTRIGL1CALO_VERSIONS_GFEXTOWERAUXCONTAINER_V1_H

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"

// System include(s):
#include <stdint.h>

// STL include(s):
#include <vector>

namespace xAOD{

  /// AuxContainer for gFexTower_v1

  class gFexTowerAuxContainer_v1 : public AuxContainerBase {
  public:
    // Default constructor
    gFexTowerAuxContainer_v1();
    
  private:   
    
    std::vector<float>    eta;
    std::vector<float>    phi;
    std::vector<uint8_t> iEta;           
    std::vector<uint8_t> iPhi; 
    std::vector<uint8_t> fpga;
    std::vector<int> towerEt;
    std::vector<char> isSaturated;
    
  }; // class gFexTowerAuxContainer_v1 
} // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::gFexTowerAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL1CALO_VERSIONS_GFEXTOWERAUXCONTAINER_V1_H
