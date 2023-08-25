/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_SPACEPOINT_V1_H
#define XAODINDETMEASUREMENT_VERSION_SPACEPOINT_V1_H

#include "AthContainers/AuxElement.h"
#include "Identifier/Identifier.h"
#include "EventPrimitives/EventPrimitives.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "AthLinks/ElementLink.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

#include <array>

namespace xAOD {
  /// @brief A struct mimicking std::array<float ,3>
  /// this structure is a temporary solution for our dynamic variables.
  /// There is an issue with ROOT's handling of std::vector<std::array<T, N> >,
  /// followed in https://github.com/root-project/root/issues/12007, that prevents 
  /// us from using std::vector< std::array<float, 3> > for dynamic variables.
  /// This structure bypass the issue.
  struct ArrayFloat3 {
    float* data() { return &values[0]; }
    const float* data() const { return &values[0]; }
    float values[3]; 
  };

  class SpacePoint_v1 : public SG::AuxElement {
  public:
    using ConstVectorMap = Eigen::Map<const Eigen::Matrix<float, 3, 1>>;
    using VectorMap = Eigen::Map<Eigen::Matrix<float, 3, 1>>;
    
    /// @name Functions to get space point properties
    /// @{

    /// Returns the IdentifierHash of the spacepoint (corresponds to the detector element IdentifierHash)
    const std::vector<DetectorIDHashType>& elementIdList() const;

    /// Returns the global position of the pixel cluster
    ConstVectorMap globalPosition() const;
    VectorMap globalPosition();

    // radius
    float radius() const;

    /// Returns the variances
    float varianceR() const;
    float varianceZ() const;

    /// Returns the index of the measurements
    const std::vector< ElementLink< xAOD::UncalibratedMeasurementContainer > >& measurements() const;

    /// Return details
    float topHalfStripLength() const;
    float bottomHalfStripLength() const;

    ConstVectorMap topStripDirection() const;
    ConstVectorMap bottomStripDirection() const;
    ConstVectorMap stripCenterDistance() const;
    ConstVectorMap topStripCenter() const;

    VectorMap topStripDirection();
    VectorMap bottomStripDirection();
    VectorMap stripCenterDistance();
    VectorMap topStripCenter();

    float x() const;
    float y() const;
    float z() const;

    /// @}

    /// @name Functions to set space point properties
    /// @{

    /// Sets the IdentifierHash of the measurement (corresponds to the detector element IdentifierHash)
    void setElementIdList(const std::vector<DetectorIDHashType>& idHash);

    /// Set the radius
    void setRadius(float);

    /// Sets the variances
    void setVarianceR(float);
    void setVarianceZ(float);

    /// Sets the index of the measurements
    void setMeasurements(const std::vector< ElementLink< xAOD::UncalibratedMeasurementContainer > >&);

    void setTopHalfStripLength(float);
    void setBottomHalfStripLength(float);

    /// @}

    void setSpacePoint(DetectorIDHashType idHash,
		       const Eigen::Matrix<float,3,1>& globPos,
		       float cov_r, float cov_z,
		       const std::vector< ElementLink< xAOD::UncalibratedMeasurementContainer > >& measurementIndexes);

    void setSpacePoint(const std::vector<DetectorIDHashType>& idHashes,
		       const Eigen::Matrix<float,3,1>& globPos,
		       float cov_r, float cov_z,
		       const std::vector< ElementLink< xAOD::UncalibratedMeasurementContainer > >& measurementIndexes,
		       float topHalfStripLength,
		       float bottomHalfStripLength,
		       const Eigen::Matrix<float,3,1>& topStripDirection,
		       const Eigen::Matrix<float,3,1>& bottomStripDirection,
		       const Eigen::Matrix<float,3,1>& stripCenterDistance,
		       const Eigen::Matrix<float,3,1>& topStripCenter);
  };

}

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE( xAOD::SpacePoint_v1, SG::AuxElement);

#endif
