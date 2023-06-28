/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/SCT_ID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/SCT_RDORawData.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetReadoutGeometry/SiDetectorElementStatus.h>
#include <xAODInDetMeasurement/StripClusterContainer.h>


namespace ActsTrk {

using StripRDORawData = SCT_RDORawData;
using StripID = SCT_ID;

class IStripClusteringTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IStripClusteringTool, 1, 0);

    virtual StatusCode
    clusterize(const InDetRawDataCollection<StripRDORawData>& RDOs,
	       const StripID& stripID,
	       const InDetDD::SiDetectorElement* element,
	       const InDet::SiDetectorElementStatus *stripDetElStatus,
	       xAOD::StripClusterContainer& container) const = 0;
};

}

#endif
