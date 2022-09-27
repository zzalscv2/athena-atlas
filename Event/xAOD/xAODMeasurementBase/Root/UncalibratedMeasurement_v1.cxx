/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

static const SG::AuxElement::Accessor< xAOD::DetectorIDHashType > identifierHashAcc( "identifierHash" );

void xAOD::UncalibratedMeasurement_v1::setIdentifierHash(const xAOD::DetectorIDHashType id) {
    identifierHashAcc(*this) = id;
}

xAOD::DetectorIDHashType xAOD::UncalibratedMeasurement_v1::identifierHash() const {
    return identifierHashAcc(*this);
}

