/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_MMCluster_V1_H
#define XAODMUONPREPDATA_VERSION_MMCluster_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {
/// https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecEvent/MuonPrepRawData/MuonPrepRawData/MdtPrepData.h

class MMCluster_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    MMCluster_v1() = default;
    /// Virtual destructor
    virtual ~MMCluster_v1() = default;

    /// Returns the type of the MM strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::MMClusterType;
    }
    unsigned int numDimensions() const override final { return 1; }

    /** @brief Returns the hash of the measurement channel*/
    IdentifierHash measurementHash() const;

    /** @brief Returns the time  (ns). 
    The time is calibrated, i.e. it is in units of ns, after t0 subtraction.*/
    uint16_t time() const;

    /** @brief Returns the charge
     * The charge is calibrated, i.e. it is in units of electrons, after pedestal subtraction.
    */
    uint32_t charge() const;

    /** @brief Returns the Drift Distance*/
    float driftDist() const;

    /** @brief Returns the microTPC angle */
    float angle() const;

    /** @brief Returns the microTPC chisq Prob. */
    float chiSqProb() const;

    enum class Author : unsigned int {
      RDOTOPRDConverter,
      SimpleClusterBuilder,
      ProjectionClusterBuilder,
      ClusterTimeProjectionClusterBuilder,
      ConstraintuTPCClusterBuilder,
      uTPCClusterBuilder,
    };

    Author author() const;
    void setAuthor(Author author);

    /** @brief Sets the TDC counts */
    void setTime(uint16_t value);
    /** @brief Sets the calibrated charge */
    void setCharge(uint32_t value);
    /** @brief Sets the drift distance */
    void setDriftDist(float value);
    /** @brief Sets the microTPC angle*/
    void setAngle(float value);
    /** @brief Sets the microTPC chisq probability*/
    void setChiSqProb(float value);
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::MMCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif