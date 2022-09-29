/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKING_VERSIONS_TRACKPARAMETERAUXCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKPARAMETERAUXCONTAINER_V1_H



#include "xAODCore/AuxContainerBase.h"
namespace xAOD {
 class TrackParameterAuxContainer_v1 : public AuxContainerBase {
    public:
        TrackParameterAuxContainer_v1();
        // we use vector instead of array even though the size is fixed
        // this saves on generating ROOT dictionaries for all array dimensions
        typedef std::vector<double> Storage;
        std::vector<Storage> params;
        std::vector<Storage> covMatrix;
    };
}

#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TrackParameterAuxContainer_v1, xAOD::AuxContainerBase);


#endif