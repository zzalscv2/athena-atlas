/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/TRT_LayerBuilder.h"
//Trk
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
// InDetDD
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

// constructor
InDet::TRT_LayerBuilder::TRT_LayerBuilder(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}


// Athena standard methods
// initialize
StatusCode InDet::TRT_LayerBuilder::initialize()
{
  ATH_MSG_DEBUG( "initialize()" );
  // get TRT Detector Description Manager
  ATH_CHECK(detStore()->retrieve(m_trtMgr, m_trtMgrLocation));
  return StatusCode::SUCCESS;
}


/** LayerBuilder interface method - returning Barrel-like layers */
std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
InDet::TRT_LayerBuilder::cylindricalLayers() const
{
  if (!m_trtMgr) return nullptr;
  const InDetDD::TRT_DetElementContainer* trtContainer = m_trtMgr->getDetectorElementContainer();
  return cylindricalLayersImpl(trtContainer);
}


/** LayerBuilder interface method - returning Endcap-like layers */
std::unique_ptr<const std::vector<Trk::DiscLayer*> > InDet::TRT_LayerBuilder::discLayers() const
{
  if (!m_trtMgr) return nullptr;
  const InDetDD::TRT_DetElementContainer* trtContainer = m_trtMgr->getDetectorElementContainer();
  return discLayersImpl(trtContainer);
}
