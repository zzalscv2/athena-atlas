/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMEASUREMENTBASE_VERSION_UNCALIBRATEDMEASUREMENT_V1_H
#define XAODMEASUREMENTBASE_VERSION_UNCALIBRATEDMEASUREMENT_V1_H

// EDM include(s):
#include "AthContainers/AuxElement.h"

#include "EventPrimitives/EventPrimitives.h"

namespace xAOD {

    /// Define the type of the uncalibrated measurement
    enum class UncalibMeasType {
        Other = 0,
        // InDet
        PixelClusterType = 1,
        StripClusterType = 2
    };
    /// @ detector ID element hash
    using DetectorIDHashType = unsigned int;
    /// @class UncalibratedMeasurement_v1
    /// Class describing uncalibrated measurements
    class UncalibratedMeasurement_v1 : public SG::AuxElement {

    public:

        /// Default constructor
        UncalibratedMeasurement_v1() = default;
        /// Default copy constructors
        UncalibratedMeasurement_v1(const UncalibratedMeasurement_v1&) = default;
        UncalibratedMeasurement_v1& operator=(const UncalibratedMeasurement_v1&) = default;

        /// Virtual destructor
        virtual ~UncalibratedMeasurement_v1() = default;

        UncalibratedMeasurement_v1(UncalibratedMeasurement_v1&&) = delete;
        UncalibratedMeasurement_v1& operator=(UncalibratedMeasurement_v1&&) = delete;

        /// @name Functions to get measurement properties
        /// @{

        /// Returns the IdentifierHash of the measurement (corresponds to the detector element IdentifierHash)
        DetectorIDHashType identifierHash() const;

        using PosAccessor = const SG::AuxElement::Accessor< std::array< float, 3 > >;
        using CovAccessor = const SG::AuxElement::Accessor< std::array< float, 9 > >;

        /// Returns the local position of the measurement
        template < int N >
        Eigen::Map<const Eigen::Matrix<float,N,1>> localPosition() const {
            static PosAccessor locPosAcc( "localPosition" );
            const auto& values = locPosAcc(*this);
            if(values.size() < N) { 
                throw std::runtime_error{"Storage does not have sufficient size"};
            }
            assert(values.data() != nullptr);
            return Eigen::Map<const Eigen::Matrix<float,N,1>>{values.data()};
        }

        /// Returns the local position as mutable eigen map which can be assigned as well
        template < int N >
        Eigen::Map<Eigen::Matrix<float,N,1>> localPosition() {
            static PosAccessor locPosAcc( "localPosition" );
            auto& values = locPosAcc(*this);
            if(values.size() < N) {
                throw std::runtime_error{"Storage does not have sufficient size. Used size not allowed. Maximum size is 3."};
            }
            assert(values.data() != nullptr);
            return Eigen::Map<Eigen::Matrix<float,N,1>>{values.data()};
        }

        /// Returns the local covariance of the measurement
        template < int N >
        Eigen::Map<const Eigen::Matrix<float,N,N>> localCovariance() const {
            static CovAccessor locCovAcc( "localCovariance" );
            const auto& values = locCovAcc(*this);
            if(values.size() < N*N) { 
                throw std::runtime_error{"Storage does not have sufficient size"};
            }
            assert(values.data() != nullptr);
            return Eigen::Map<const Eigen::Matrix<float,N,N>>{values.data()};
        }

        /// Returns the local covariance as mutable eigen map which can be assigned as well
        template < int N >
        Eigen::Map<Eigen::Matrix<float,N,N>> localCovariance() {
            static CovAccessor locCovAcc( "localCovariance" );
            auto& values = locCovAcc(*this);
            if(values.size() < N*N) {
                throw std::runtime_error{"Storage does not have sufficient size. Used size not allowed. Maximum size is 9."};
            }
            assert(values.data() != nullptr);
            return Eigen::Map<Eigen::Matrix<float,N,N>>{values.data()};
        }


        /// Returns the type of the measurement type as a simple enumeration
        virtual xAOD::UncalibMeasType type() const =0 ;

        /// @}

        /// @name Functions to set measurement properties
        /// @{

        /// Sets the IdentifierHash of the measurement (corresponds to the detector element IdentifierHash)
        void setIdentifierHash(const DetectorIDHashType idHash);


        /// @}

        /// @name Direct method to set measurement properties
        /// @{

        /// Sets IdentifierHash, local position and local covariance of the measurement
        template < int N >
        void setMeasurement(DetectorIDHashType idHash,
                            Eigen::Matrix<float,N,1>& locPos,
                            Eigen::Matrix<float,N,N>& locCov) {
            setIdentifierHash(idHash);
            localPosition<N>() = locPos;
            localCovariance<N>() = locCov;
        }
        /// @}

    };

}


#endif // XAODMEASUREMENTBASE_VERSION_UNCALIBRATEDMEASUREMENT_V1_H
