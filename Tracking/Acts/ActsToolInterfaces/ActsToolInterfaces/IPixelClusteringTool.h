/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/PixelID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/PixelRDO_Container.h>
#include <InDetRawData/PixelRDORawData.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"


namespace ActsTrk {

class IPixelClusteringTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IPixelClusteringTool, 1, 0);

    using RDOContainer = PixelRDO_Container;
    using RawDataCollection = RDOContainer::base_value_type;
    using IDHelper = PixelID;
    using ClusterContainer = xAOD::PixelClusterContainer;
    using ClusterAuxContainer = xAOD::PixelClusterAuxContainer;

    virtual StatusCode
    clusterize(const RawDataCollection& RDOs,
	       const IDHelper& pixelID,
	       const EventContext& ctx,
	       ClusterContainer& container) const = 0;
};

}

#endif
