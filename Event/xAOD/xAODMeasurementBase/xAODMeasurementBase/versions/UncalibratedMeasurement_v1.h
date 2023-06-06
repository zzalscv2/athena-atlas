/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMEASUREMENTBASE_VERSION_UNCALIBRATEDMEASUREMENT_V1_H
#define XAODMEASUREMENTBASE_VERSION_UNCALIBRATEDMEASUREMENT_V1_H

// EDM include(s):
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {

class UncalibratedMeasurement_v1 : public SG::AuxElement {

   public:
    /// Default constructor
    UncalibratedMeasurement_v1() = default;
    /// Default copy constructors
    UncalibratedMeasurement_v1(const UncalibratedMeasurement_v1&) = default;
    UncalibratedMeasurement_v1& operator=(const UncalibratedMeasurement_v1&) =
        default;

    /// Virtual destructor
    virtual ~UncalibratedMeasurement_v1() = default;

    UncalibratedMeasurement_v1(UncalibratedMeasurement_v1&&) = delete;
    UncalibratedMeasurement_v1& operator=(UncalibratedMeasurement_v1&&) =
        delete;

    /// @name Functions to get measurement properties
    /// @{

    /// Returns the IdentifierHash of the measurement (corresponds to the
    /// detector element IdentifierHash)
    DetectorIDHashType identifierHash() const;
    /// Returns the full Identifier of the measurement
    DetectorIdentType identifier() const;

    /// Returns the local position of the measurement
    template <int N>
    ConstVectorMap<N> localPosition() const;
    /// Returns the local position as mutable eigen map which can be assigned as
    /// well
    template <int N>
    VectorMap<N> localPosition();
    /// Returns the local covariance of the measurement
    template <int N>
    ConstMatrixMap<N> localCovariance() const;

    /// Returns the local covariance as mutable eigen map which can be assigned
    /// as well
    template <int N>
    MatrixMap<N> localCovariance();
    /// Returns the type of the measurement type as a simple enumeration
    virtual xAOD::UncalibMeasType type() const = 0;
    /// Returns the number of dimensions of the measurement
    virtual unsigned int numDimensions() const = 0;

    /// @}

    /// @name Functions to set measurement properties
    /// @{

    /// Sets the IdentifierHash of the measurement (corresponds to the detector
    /// element IdentifierHash)
    void setIdentifierHash(const DetectorIDHashType idHash);
    /// Sets the full Identifier of the measurement
    void setIdentifier(const DetectorIdentType measId);

    /// @}

    /// @name Direct method to set measurement properties
    /// @{

    /// Sets IdentifierHash, local position and local covariance of the
    /// measurement
    template <int N>
    void setMeasurement(const DetectorIDHashType idHash, MeasVector<N> locPos,
                        MeasMatrix<N> locCov);
    /// @}
};

}  // namespace xAOD

#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.icc"
#endif  // XAODMEASUREMENTBASE_VERSION_UNCALIBRATEDMEASUREMENT_V1_H
