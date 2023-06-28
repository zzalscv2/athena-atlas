/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/PixelID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/PixelRDORawData.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>


namespace ActsTrk {

class IPixelClusteringTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IPixelClusteringTool, 1, 0);

    virtual StatusCode
    clusterize(const InDetRawDataCollection<PixelRDORawData>& RDOs,
	       const PixelID& pixelID,
	       const EventContext& ctx,
	       xAOD::PixelClusterContainer& container) const = 0;
};

}

#endif
