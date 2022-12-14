/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/BeamPipeBuilderCond.h"

// constructor
InDet::BeamPipeBuilderCond::BeamPipeBuilderCond(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

/** LayerBuilder interface method - returning Barrel-like layers */
//TODO: context is not used, beamPipe retrieved from manager. range is infinite
std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
InDet::BeamPipeBuilderCond::cylindricalLayers(const EventContext&,
                                              SG::WriteCondHandle<Trk::TrackingGeometry>& /*whandle*/) const
{
  return InDet::BeamPipeBuilderImpl::cylindricalLayersImpl();
}
