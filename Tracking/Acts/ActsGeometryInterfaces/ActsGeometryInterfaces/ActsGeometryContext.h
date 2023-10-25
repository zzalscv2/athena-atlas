/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ACTSGEOMETRYCONTEXT_H
#define ACTSGEOMETRYINTERFACES_ACTSGEOMETRYCONTEXT_H

#include <map>
#include <memory>
/// Include the GeoPrimitives which need to be put first
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// If the package is loaded in AthSimulation, the Acts library is not avaialble
#ifndef SIMULATIONBASE 
#   include "Acts/Geometry/GeometryContext.hpp"
#endif

#include "ActsGeometryInterfaces/AlignmentStore.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

class ActsGeometryContext {
public:
    using DetectorType = ActsTrk::DetectorType;
    using AlignmentStore = ActsTrk::AlignmentStore;
    /// Populate a map containing the alignment stores of each
    /// sub detector --> Avoid duplication of copying the transforms if
    /// only one subsystem updates alignment
    using SubDetAlignments = std::map<DetectorType, GeoModel::TransientConstSharedPtr<AlignmentStore>>;

    SubDetAlignments alignmentStores{};
#ifndef SIMULATIONBASE 
    Acts::GeometryContext context() const { return Acts::GeometryContext(this); }
#endif
};

CLASS_DEF(ActsGeometryContext, 51464195, 1)
CONDCONT_DEF(ActsGeometryContext, 11228079);

#endif
