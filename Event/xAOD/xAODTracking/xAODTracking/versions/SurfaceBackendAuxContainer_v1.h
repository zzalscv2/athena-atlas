/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_SURFACEBACKENDAUXCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_SURFACEBACKENDAUXCONTAINER_V1_H

#include "xAODCore/AuxContainerBase.h"
#include "xAODTracking/TrackingPrimitives.h"
#include <vector>

namespace xAOD {
 class SurfaceBackendAuxContainer_v1 : public AuxContainerBase {
    public:
        SurfaceBackendAuxContainer_v1();
        // we use vector instead of array even though the size is fixed
        // this saves on generating ROOT dictionaries for all array dimensions
        
        typedef std::vector<float> Storage;


        std::vector<Storage> translation;
        std::vector<Storage> rotation;
        std::vector<Storage> boundValues;
        std::vector<xAOD::SurfaceType> SurfaceType;

        
    };
}

#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::SurfaceBackendAuxContainer_v1, xAOD::AuxContainerBase);


#endif
