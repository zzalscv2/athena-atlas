/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKJACOBIANAUXCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKJACOBIANAUXCONTAINER_V1_H



#include "xAODCore/AuxContainerBase.h"
namespace xAOD {
 class TrackJacobianAuxContainer_v1 : public AuxContainerBase {
    public:
        TrackJacobianAuxContainer_v1();
        // see commend about storage in TrackParameters
        typedef std::vector<double> Storage;
        std::vector<Storage> values;
    };
}

#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TrackJacobianAuxContainer_v1, xAOD::AuxContainerBase);


#endif