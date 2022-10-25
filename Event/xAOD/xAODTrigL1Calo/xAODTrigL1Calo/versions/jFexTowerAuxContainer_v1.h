/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALO_VERSIONS_JFEXTOWERAUXCONTAINER_V1_H
#define XAODTRIGL1CALO_VERSIONS_JFEXTOWERAUXCONTAINER_V1_H

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"

// System include(s):
#include <stdint.h>

// STL include(s):
#include <vector>

namespace xAOD{

  /// AuxContainer for jFexTower_v1

class jFexTowerAuxContainer_v1 : public AuxContainerBase {
    public:
        // Default constructor
        jFexTowerAuxContainer_v1();

    private:

        std::vector<float>    eta;
        std::vector<float>    phi;
        std::vector<int>      globalEta;
        std::vector<unsigned int>      globalPhi;
        std::vector<uint8_t>  module;
        std::vector<uint8_t>  fpga;
        std::vector<uint8_t>  channel;
        std::vector<uint8_t>  jFEXdataID;
        std::vector<uint32_t> jFEXtowerID;
        std::vector<uint8_t>  Calosource;
        
        std::vector<std::vector<uint16_t> > et_count;
        std::vector<std::vector<char>>      isSaturated;
        
}; // class jFexTowerAuxContainer_v1
} // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::jFexTowerAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL1CALO_VERSIONS_JFEXTOWERAUXCONTAINER_V1_H
