/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonRDO/versions/NRPCRDO_v1.h"

namespace xAOD{



AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, uint32_t, bcid, setBcid)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, float, time, setTime)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, uint16_t, subdetector, setSubdetector)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, uint16_t, tdcsector, setTdcsector)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, uint16_t, tdc, setTdc)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, uint16_t, channel, setChannel)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::NRPCRDO_v1, float, timeoverthr, setTimeoverthr)

}