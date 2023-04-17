/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKTOOLINTERFACES_ISTRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTRKTOOLINTERFACES_ISTRIPSPACEPOINTFORMATIONTOOL_H 1

// Athena
#include "GaudiKernel/IAlgTool.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

namespace ActsTrk {
  struct StripSP {
    StripSP() = default;

    std::vector<unsigned int> idHashes {};
    Eigen::Matrix<float,3,1> globPos {0, 0, 0};
    float cov_r {0};
    float cov_z {0};
    std::vector<std::size_t> measurementIndexes {};
    float topHalfStripLength {0};
    float bottomHalfStripLength {0};
    Eigen::Matrix<float,3,1> topStripDirection {0, 0, 0};
    Eigen::Matrix<float,3,1> bottomStripDirection {0, 0, 0};
    Eigen::Matrix<float,3,1> stripCenterDistance {0, 0, 0};
    Eigen::Matrix<float,3,1> topStripCenter {0, 0, 0};
  };

    /// @class IPixelSpacePointFormationTool
    /// Base class for strip space point formation tool

    class IStripSpacePointFormationTool : virtual public IAlgTool {
    public:
      DeclareInterfaceID(IStripSpacePointFormationTool, 1, 0);


      virtual StatusCode produceSpacePoints(const EventContext& ctx,
					    const xAOD::StripClusterContainer& clusterContainer,
					    const InDet::SiElementPropertiesTable& properties,
					    const InDetDD::SiDetectorElementCollection& elements,
					    const Amg::Vector3D& beamSpotVertex,
					    std::vector<StripSP>& spacePoints,
					    std::vector<StripSP>& overlapSpacePoints,
					    bool processOverlaps) const = 0;

    };

} // ACTSTRKTOOLINTERFACES_ISTRIPSPACEPOINTFORMATIONTOOL_H

#endif
