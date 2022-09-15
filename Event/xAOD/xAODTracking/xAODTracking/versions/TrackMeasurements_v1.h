/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKMEASUREMENTS_V1_H
#define XAODTRACKING_VERSIONS_TRACKMEASUREMENTS_V1_H
#include <cstdint>
#include "AthContainers/AuxElement.h"
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
        ConstVectorMap measurements() const;
        /**
         * access Measurements of non const element
         **/
        VectorMap measurements();

        /**
         * access track covariance matrix (flattened, rows layout) of const element
         **/
        ConstMatrixMap covariance() const;

        /**
         * access track covariance matrix (flattened, rows layout)
         **/
        MatrixMap covariance();

        // TODO projectors
        // TODO SourceLinks

        /**
         * @brief expands sizes of internal vectors for the data storage
         * ( by default this is 6 for Measurements and 6x6 for Covariance)
         * typically only 5 are used
         */
        void resize(size_t sz = 6);
    };
}
#endif