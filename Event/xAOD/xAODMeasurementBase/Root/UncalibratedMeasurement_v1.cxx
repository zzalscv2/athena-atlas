/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(UncalibratedMeasurement_v1,
                                     DetectorIDHashType, identifierHash,
                                     setIdentifierHash)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(UncalibratedMeasurement_v1,
                                     DetectorIdentType, identifier,
                                     setIdentifier)
}  // namespace xAOD
