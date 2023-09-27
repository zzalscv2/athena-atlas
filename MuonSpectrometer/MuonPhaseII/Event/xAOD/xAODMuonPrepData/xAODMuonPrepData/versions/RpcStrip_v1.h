/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_RPCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_RPCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {
/// https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecEvent/MuonPrepRawData/MuonPrepRawData/MdtPrepData.h

class RpcStrip_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    RpcStrip_v1() = default;
    /// Virtual destructor
    virtual ~RpcStrip_v1() = default;

    /// Returns the type of the Rpc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::RpcStripType;
    }
    unsigned int numDimensions() const override final { return 1; }
    /** @brief Returns the time. */
    float time() const;

    /** @brief Returns the trigger coincidence - usually false, unless ijk>5 or highpt&&ijk==0*/
    uint32_t triggerInfo() const;

    /** @brief Returns the number of ambiguities associated with this RpcPrepData.
        - 0 if the ambiguites have not been removed by choice;
        - 1 if the ambiguities are fully solved
        - i+1 if "i" other MuonPrepRawData are produced along with the current one from a single RDO hit*/
    uint8_t ambiguityFlag() const;

    /** @brief Returns the time over threshold */
    float timeOverThreshold() const;

    /** @brief Returns the hash of the measurement channel (tube (x) layer) */
    IdentifierHash measurementHash() const;

    /// Setter methods

    /** @brief Sets the TDC counts */
    void setTime(float time);
    /** @brief Sets the ADC counts */
    void setTriggerInfo(uint32_t triggerinfo);
    /** @brief Sets the ADC counts */
    void setAmbiguityFlag(uint8_t ambi);
    /** @brief Sets the TDC counts */
    void setTimeOverThreshold(float timeoverthreshold);
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::RpcStrip_v1, xAOD::UncalibratedMeasurement_v1);
#endif