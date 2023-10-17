/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/SCT_ID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/SCT_RDO_Container.h>
#include <InDetRawData/SCT_RDORawData.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetReadoutGeometry/SiDetectorElementStatus.h>
#include <xAODInDetMeasurement/StripClusterContainer.h>
#include <xAODInDetMeasurement/StripClusterAuxContainer.h>


namespace ActsTrk {

using StripRDORawData = SCT_RDORawData;

class IStripClusteringTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IStripClusteringTool, 1, 0);

    using RDOContainer = SCT_RDO_Container;
    using RawDataCollection = RDOContainer::base_value_type;
    using IDHelper = SCT_ID;
    using ClusterContainer = xAOD::StripClusterContainer;
    using ClusterAuxContainer = xAOD::StripClusterAuxContainer;

    virtual StatusCode
    clusterize(const RawDataCollection& RDOs,
	       const IDHelper& stripID,
	       const EventContext& ctx,
	       ClusterContainer& container) const = 0;
};

}

#endif
