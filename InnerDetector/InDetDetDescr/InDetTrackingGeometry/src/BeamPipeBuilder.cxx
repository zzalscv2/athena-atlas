/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/BeamPipeBuilder.h"

// constructor
InDet::BeamPipeBuilder::BeamPipeBuilder(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

/** LayerBuilder interface method - returning Barrel-like layers */
std::unique_ptr<const std::vector<Trk::CylinderLayer*> > InDet::BeamPipeBuilder::cylindricalLayers() const
{
  return InDet::BeamPipeBuilderImpl::cylindricalLayersImpl();
}
