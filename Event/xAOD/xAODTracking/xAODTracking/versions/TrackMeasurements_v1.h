/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKMEASUREMENTS_V1_H
#define XAODTRACKING_VERSIONS_TRACKMEASUREMENTS_V1_H
#include <cstdint>
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "EventPrimitives/EventPrimitives.h"


namespace xAOD {
    /**
     * @brief Track Measurements for Acts MultiTrajectory
     **/

    class TrackMeasurements_v1 : public SG::AuxElement {
    public:
        using ConstVectorMap = Eigen::Map<const Eigen::Matrix<double, 6, 1>>;
        using VectorMap = Eigen::Map<Eigen::Matrix<double, 6, 1>>;
        using ConstMatrixMap = Eigen::Map<const Eigen::Matrix<double, 6, 6>>;
        using MatrixMap = Eigen::Map<Eigen::Matrix<double, 6, 6>>;

        TrackMeasurements_v1() = default;
        /**
         * access track Measurements vector of const element
         **/
        ConstVectorMap measEigen() const;
        /**
         * access Measurements of non const element
         **/
        VectorMap measEigen();

        /**
         * access track Measurements as plain vector
         **/
        const std::vector<double>& meas() const;
        /**
         * access set Measurements from plain vector
         **/
        void setMeas( const std::vector<double>& m);


        /**
         * access track covariance matrix (flattened, rows layout) of const element
         **/
        ConstMatrixMap covMatrixEigen() const;

        /**
         * access track covariance matrix (flattened, rows layout)
         **/
        MatrixMap covMatrixEigen();

        /**
         * access track covariance as plain vector
         **/
        const std::vector<double>& covMatrix() const;
        /**
         * access set covariance from plain vector
         **/
        void setCovMatrix( const std::vector<double>& m);


        /**
         * @brief access the uncalibrated measurement
         * TODO consider bare pointer access
         */
        const ElementLink<UncalibratedMeasurementContainer>& uncalibratedMeasurementLink() const;

        /**
         * @brief return pointer to uncalibrated measurement if the underlying link is valid
         * 
         * @return const UncalibratedMeasurement* or nullptr
         */
        const UncalibratedMeasurement* uncalibratedMeasurement() const;

        /**
         * @brief set uncalibrated measurement
         */
        void setUncalibratedMeasurementLink( const ElementLink<UncalibratedMeasurementContainer>& link);


        // TODO projectors

        /**
         * @brief expands sizes of internal vectors for the data storage
         * ( by default this is 6 for Measurements and 6x6 for Covariance)
         * typically only 5 are used
         */
        void resize(size_t sz = 6);
    };
}
#endif