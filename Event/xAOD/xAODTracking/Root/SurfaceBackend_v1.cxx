/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/SurfaceBackend_v1.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/AuxAccessorMacro.h"

namespace xAOD {

AUXSTORE_OBJECT_SETTER_AND_GETTER(SurfaceBackend_v1, std::vector<float>, 
                                  translation, setTranslation)

AUXSTORE_OBJECT_SETTER_AND_GETTER(SurfaceBackend_v1, std::vector<float>,
                                  rotation, setRotation)

AUXSTORE_OBJECT_SETTER_AND_GETTER(SurfaceBackend_v1, std::vector<float>,
                                  boundValues, setBoundValues)


DEFINE_API(SurfaceBackend_v1, xAOD::SurfaceType, SurfaceType, setSurfaceType)


size_t SurfaceBackend_v1::size() const {
  return translation().size();
}

}  // namespace xAOD
