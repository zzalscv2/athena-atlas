/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H
#define ACTSTRKTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H

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
