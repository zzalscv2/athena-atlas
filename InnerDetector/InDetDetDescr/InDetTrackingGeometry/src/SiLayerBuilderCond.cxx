/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/SiLayerBuilderCond.h"
// InDet include
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
// Trk inlcude
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkSurfaces/Surface.h"
// Athena
#include "AthenaKernel/IOVInfiniteRange.h"
// STL
#include <map>

// constructor
InDet::SiLayerBuilderCond::SiLayerBuilderCond(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

// Athena standard methods
// initialize
StatusCode InDet::SiLayerBuilderCond::initialize()
{
    ATH_CHECK(InDet::SiLayerBuilderImpl::initialize());
    ATH_CHECK(m_SCT_ReadKey.initialize(!m_pixelCase));
    ATH_CHECK(m_PixelReadKey.initialize());
    return StatusCode::SUCCESS;
}


SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> InDet::SiLayerBuilderCond::retrieveSiDetElements(const EventContext& ctx) const
{
  if(m_pixelCase){
    auto readHandle = SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> (m_PixelReadKey, ctx);
    if (*readHandle==nullptr) {
      ATH_MSG_ERROR("Null pointer to the read conditions object of " << m_PixelReadKey.key());
    }
    return readHandle;
  }else{
    auto readHandle = SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> (m_SCT_ReadKey, ctx);
    if (*readHandle==nullptr) {
      ATH_MSG_ERROR("Null pointer to the read conditions object of " << m_SCT_ReadKey.key());
    }
    return readHandle;
  }
}

/** LayerBuilder interface method - returning Barrel-like layers */
std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
InDet::SiLayerBuilderCond::cylindricalLayers(const EventContext& ctx,
                                             SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const
{
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> readHandle = retrieveSiDetElements(ctx);
  if(*readHandle == nullptr){
    return nullptr;
  }
  whandle.addDependency (readHandle);

  const InDetDD::SiDetectorElementCollection* readCdo{*readHandle};
  return cylindricalLayersImpl(*readCdo);
}


/** LayerBuilder interface method - returning Endcap-like layers */
std::unique_ptr<const std::vector<Trk::DiscLayer*> >
InDet::SiLayerBuilderCond::discLayers(const EventContext& ctx,
                                      SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const
{
  // sanity check for ID Helper
  if (!m_pixIdHelper && !m_sctIdHelper){
       ATH_MSG_ERROR("Neither Pixel nor SCT Detector Manager or ID Helper could be retrieved - giving up.");
       return nullptr;
  }

  // check for DBMS
  int nDBMLayers = m_siMgr->numerology().numEndcapsDBM();
  if (!nDBMLayers) return ((m_pixelCase and m_useRingLayout) ? createRingLayers(ctx, whandle) : createDiscLayers(ctx, whandle));

  ATH_MSG_DEBUG( "Found " << m_siMgr->numerology().numEndcapsDBM() << " DBM layers active, building first ECs, then DBMS");
  std::unique_ptr<std::vector<Trk::DiscLayer*> >  ecLayers = createDiscLayers(ctx, whandle);
  if (ecLayers) {
      ATH_MSG_VERBOSE( "Created " << ecLayers->size() << " endcap layers w/o  DBM.");
      ecLayers = createDiscLayers(ctx, whandle, std::move(ecLayers));
      ATH_MSG_VERBOSE( "Created " << ecLayers->size() << " endcap layers with DBM.");
  }
  return ecLayers;
}

/** LayerBuilder interface method - returning Endcap-like layers */
std::unique_ptr<std::vector<Trk::DiscLayer*> >
InDet::SiLayerBuilderCond::createDiscLayers(
  const EventContext& ctx,
  SG::WriteCondHandle<Trk::TrackingGeometry>& whandle,
  std::unique_ptr<std::vector<Trk::DiscLayer*> > discLayers) const
{

  // get general layout
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> readHandle = retrieveSiDetElements(ctx);
  if(*readHandle == nullptr){
    return nullptr;
  }
  whandle.addDependency (readHandle);

  const InDetDD::SiDetectorElementCollection* readCdo{*readHandle};
  return createDiscLayersImpl(*readCdo, std::move(discLayers));
}


/** LayerBuilder interface method - returning ring-like layers */
/** this is ITk pixel specific and doesn't include DBM modules */
std::unique_ptr<std::vector< Trk::DiscLayer*> >
InDet::SiLayerBuilderCond::createRingLayers(const EventContext& ctx,
                                            SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const {

  // get general layout
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> readHandle = retrieveSiDetElements(ctx);
  if(*readHandle == nullptr){
    return nullptr;
  }
  whandle.addDependency (readHandle);

  const InDetDD::SiDetectorElementCollection* readCdo{*readHandle};
  return createRingLayersImpl(*readCdo);
}



