/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_IACTSDETECTORELEMENT_H
#define ACTSGEOMETRYINTERFACES_IACTSDETECTORELEMENT_H

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// In AthSimulation, the Acts core library is not available yet
#ifndef SIMULATIONBASE 
#   include "Acts/Geometry/DetectorElementBase.hpp"
#endif
#include "ActsGeometryInterfaces/RawGeomAlignStore.h"
#include "Identifier/Identifier.h"

namespace ActsTrk {
    class IDetectorElement
#ifndef SIMULATIONBASE    
     : public Acts::DetectorElementBase
#endif 
                                      {
    public:
        virtual ~IDetectorElement() = default;

        /// Returns the ATLAS identifier
        virtual Identifier identify() const = 0;
        /// Returns the detector element type
        virtual DetectorType detectorType() const = 0;
        /// Cache the detector element transformations in the Geometry Context
        /// Returns false if the detectorType() of the DetectorElement does not
        /// match the one in the alignment store
        virtual bool storeAlignment(RawGeomAlignStore& store) const = 0;
    };
}  // namespace ActsTrk

#endif