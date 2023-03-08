/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H
#define ACTSTRKTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H 1

// Athena
#include "GaudiKernel/IAlgTool.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

namespace ActsTrk {

    /// @class IPixelSpacePointFormationTool
    /// Base class for pixel space point formation tool

    class IPixelSpacePointFormationTool : virtual public IAlgTool {
    public:
        DeclareInterfaceID(IPixelSpacePointFormationTool, 1, 0);

        /// @name Production of space points
        //@{
	virtual StatusCode producePixelSpacePoint(const xAOD::PixelCluster& cluster,
						  xAOD::SpacePoint& sp,
						  const std::vector<std::size_t>& measIndexes,
						  const InDetDD::SiDetectorElement& element) const = 0;
        //@}

    };

} // ACTSTRKTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H

#endif


