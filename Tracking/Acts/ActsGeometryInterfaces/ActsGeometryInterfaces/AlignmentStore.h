/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ALIGNMENTSTORE_H
#define ACTSGEOMETRYINTERFACES_ALIGNMENTSTORE_H

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// To be put first

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "GeoModelUtilities/TransformMap.h"

namespace ActsTrk {
    using AlignmentStore = TransformMap<void, Acts::Transform3>;
}  // namespace ActsTrk

CLASS_DEF(ActsTrk::AlignmentStore, 134756361, 1);
CONDCONT_DEF(ActsTrk::AlignmentStore, 7514121);

#endif
