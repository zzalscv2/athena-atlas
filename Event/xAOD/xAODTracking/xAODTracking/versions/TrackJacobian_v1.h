/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKJACOBIAN_V1_H
#define XAODTRACKING_VERSIONS_TRACKJACOBIAN_V1_H
#include <cstdint>
#include "AthContainers/AuxElement.h"
#include "EventPrimitives/EventPrimitives.h"

namespace xAOD {
    /**
     * @brief Storage for track Propagation Jacobian for Acts MultiTrajectory
     **/

    class TrackJacobian_v1 : public SG::AuxElement {
    public:
        using ConstMatrixMap = Eigen::Map<const Eigen::Matrix<double, 6, 6>>;
        using MatrixMap = Eigen::Map<Eigen::Matrix<double, 6, 6>>;

        TrackJacobian_v1() = default;

        /**
         * access track update Jacobian matrix
         **/
        ConstMatrixMap values() const;

        /**
         * access tack update Jacobian of non const element
         **/
        MatrixMap values();

        /**
         * @brief expands sizes of internal vectors for the data storage
         * ( by default this is 6x6 )
         */
        void resize(size_t sz = 6*6);
    };
}
#endif