/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_VERSION_NRPCRDOAUXCONTAINER_V1_H
#define XAODMUONRDO_VERSION_NRPCRDOAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"

namespace xAOD {
    /// Auxiliary store for pixel clusters
    ///
    class NRPCRDOAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        NRPCRDOAuxContainer_v1();

    private:

        std::vector < uint32_t > bcid;
        std::vector < float > time;
        std::vector < uint16_t > subdetector;
        std::vector < uint16_t > tdcsector;
        std::vector < uint16_t > tdc;
        std::vector < uint16_t > channel;
        std::vector < float > timeoverthr;
	
    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::NRPCRDOAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODMUONRDO_VERSION_NRPCRDOAUXCONTAINER_V1_H
