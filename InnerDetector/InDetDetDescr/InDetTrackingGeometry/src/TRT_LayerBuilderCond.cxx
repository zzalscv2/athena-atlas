/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/TRT_LayerBuilderCond.h"
//Trk
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
// InDetDD
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
// Athena
#include "AthenaKernel/IOVInfiniteRange.h"

// constructor
InDet::TRT_LayerBuilderCond::TRT_LayerBuilderCond(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}


// Athena standard methods
// initialize
StatusCode InDet::TRT_LayerBuilderCond::initialize()
{
  ATH_MSG_DEBUG( "initialize()" );
  // get TRT Detector Description
  ATH_CHECK(m_readKeyTRTContainer.initialize());
  return StatusCode::SUCCESS;
}


/** LayerBuilderCond interface method - returning Barrel-like layers */
std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
InDet::TRT_LayerBuilderCond::cylindricalLayers(const EventContext& ctx,
                                               SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const
{
  SG::ReadCondHandle<InDetDD::TRT_DetElementContainer> readHandleTRTContainer{m_readKeyTRTContainer,ctx};
  const InDetDD::TRT_DetElementContainer* trtContainer{*readHandleTRTContainer};
  if(trtContainer == nullptr){
    ATH_MSG_ERROR("Aligned TRT could not be retrieved from CondStore: " << m_readKeyTRTContainer);
    return nullptr;
  }
  whandle.addDependency (readHandleTRTContainer);
  return cylindricalLayersImpl(trtContainer);
}


/** LayerBuilderCond interface method - returning Endcap-like layers */
std::unique_ptr<const std::vector<Trk::DiscLayer*> >
InDet::TRT_LayerBuilderCond::discLayers(const EventContext& ctx,
                                        SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const
{
  SG::ReadCondHandle<InDetDD::TRT_DetElementContainer> readHandleTRTContainer{m_readKeyTRTContainer, ctx};
  const InDetDD::TRT_DetElementContainer* trtContainer{*readHandleTRTContainer};
  if(trtContainer == nullptr){
    ATH_MSG_ERROR("Aligned TRT could not be retrieved from CondStore: " << m_readKeyTRTContainer);
    return nullptr;
  }
  whandle.addDependency (readHandleTRTContainer);
  return discLayersImpl(trtContainer);
}
