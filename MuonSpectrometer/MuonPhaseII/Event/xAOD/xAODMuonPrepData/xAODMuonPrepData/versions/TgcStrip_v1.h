/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {
/// https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecEvent/MuonPrepRawData/MuonPrepRawData/MdtPrepData.h

class TgcStrip_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    TgcStrip_v1() = default;
    /// Virtual destructor
    virtual ~TgcStrip_v1() = default;

    /// Returns the type of the Tgc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::TgcStripType;
    }
    unsigned int numDimensions() const override final { return 1; }

    /** @brief Returns the bcBitMap of this PRD
      bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
    uint16_t bcBitMap() const;

    void setBcBitMap(uint16_t);


    /** @brief Returns the hash of the measurement channel (tube (x) layer) */
    IdentifierHash measurementHash() const;
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::TgcStrip_v1, xAOD::UncalibratedMeasurement_v1);
#endif