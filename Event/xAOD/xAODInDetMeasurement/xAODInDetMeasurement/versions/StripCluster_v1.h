/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_STRIPCLUSTER_V1_H
#define XAODINDETMEASUREMENT_VERSION_STRIPCLUSTER_V1_H

#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "Identifier/Identifier.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace xAOD {

    /// @class StripCluster_v1
    /// Class describing strip clusters

    class StripCluster_v1 : public UncalibratedMeasurement_v1 {

    public:
        using ConstVectorMap = Eigen::Map<const Eigen::Matrix<float, 3, 1>>;
        using VectorMap = Eigen::Map<Eigen::Matrix<float, 3, 1>>;

        /// Default constructor
        StripCluster_v1() = default;
        /// Virtual destructor
        virtual ~StripCluster_v1() = default;

        /// @name Functions to get strip cluster properties
        /// @{

        /// Returns the type of the strip cluster as a simple enumeration
        xAOD::UncalibMeasType type() const override final {
            return xAOD::UncalibMeasType::StripClusterType;
        }

        /// Returns the global position of the strip cluster
        ConstVectorMap globalPosition() const;
        VectorMap globalPosition();

        /// Returns the list of identifiers of the channels building the cluster
        const std::vector< Identifier > rdoList() const;

        /// Returns the dimensions of the cluster in numbers of channels in phi (x), respectively
        int channelsInPhi() const;

        /// @}

        /// @name Functions to set pixel cluster properties
        /// @{

        /// Sets the list of identifiers of the channels building the cluster
        void setRDOlist(const std::vector< Identifier >& rdolist);

        /// Sets the dimensions of the cluster in numbers of channels in phi (x)
        void setChannelsInPhi(int channelsInPhi);

        /// @}

    };

}
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE( xAOD::StripCluster_v1, xAOD::UncalibratedMeasurement_v1);

#endif
