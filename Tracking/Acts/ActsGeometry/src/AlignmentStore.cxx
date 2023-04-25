/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsGeometryInterfaces/AlignmentStore.h"

#include "Acts/Definitions/Algebra.hpp"
#include "ActsGeometry/ActsDetectorElement.h"

namespace ActsTrk {
    void AlignmentStore::setTransform(const void *ade, const Acts::Transform3 &xf) {
        if (!m_transforms.setTransform(ade, xf)) { throw ExcAlignmentStore("Attempted to overwrite Delta in the Alignment Store"); }
    }

    const Acts::Transform3 *AlignmentStore::getTransform(const void *ade) const { return m_transforms.getTransform(ade); }
}  // namespace ActsTrk
