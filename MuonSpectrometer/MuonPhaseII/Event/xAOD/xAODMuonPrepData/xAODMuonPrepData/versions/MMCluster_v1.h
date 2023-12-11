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
    unsigned int numDimensions() const override final { return 2; }

    /** @brief Returns the hash of the measurement channel*/
    IdentifierHash measurementHash() const;

    /** @brief Returns the time  (ns). 
    The time is calibrated, i.e. it is in units of ns, after t0 subtraction.*/
    uint16_t time() const;
    /** @brief Sets the TDC counts */
    void setTime(uint16_t value);

    /** @brief Returns the charge
     * The charge is calibrated, i.e. it is in units of electrons, after pedestal subtraction.
    */
    uint32_t charge() const;
    /** @brief Sets the calibrated charge */
    void setCharge(uint32_t value);

    /** @brief Returns the Drift Distance*/
    float driftDist() const;
    /** @brief Sets the drift distance */
    void setDriftDist(float value);

    /** @brief Returns the microTPC angle */
    float angle() const;
    /** @brief Sets the microTPC angle*/
    void setAngle(float value);

    /** @brief Returns the microTPC chisq Prob. */
    float chiSqProb() const;
    /** @brief Sets the microTPC chisq probability*/
    void setChiSqProb(float value);

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

    uint16_t quality() const;
    void setQuality(uint16_t quality);

    /** @brief returns the list of strip numbers */
    const std::vector<uint16_t>& stripNumbers() const;
    void setStripNumbers(const std::vector<uint16_t>& stripNumbers);

    /** @brief returns the list of times */
    const std::vector<int16_t>& stripTimes() const;
    void setStripTimes(const std::vector<int16_t>& stripTimes);

    /** @brief returns the list of charges */
    const std::vector<int>& stripCharges() const;
    void setStripCharges(const std::vector<int>& stripCharges);

    /** @brief returns the list of drift distances */
    const std::vector<float>& stripDriftDist() const;
    void setStripDriftDist(const std::vector<float>& stripDriftDist);

    /** @brief returns the list of drift distances */
    const std::vector<Amg::MatrixX>& stripDriftErrors() const;
    void setStripDriftErrors(const std::vector<Amg::MatrixX>& stripDriftErrors);
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::MMCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif