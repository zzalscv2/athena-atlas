/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/SiLayerBuilder.h"
// InDet include
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
// Trk inlcude
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkSurfaces/Surface.h"
// STL
#include <map>

// constructor
InDet::SiLayerBuilder::SiLayerBuilder(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

// Athena standard methods
// initialize
StatusCode InDet::SiLayerBuilder::initialize()
{
  ATH_CHECK(InDet::SiLayerBuilderImpl::initialize());
  return StatusCode::SUCCESS;
}


/** LayerBuilder interface method - returning Barrel-like layers */
std::unique_ptr<const std::vector<Trk::CylinderLayer*> > InDet::SiLayerBuilder::cylindricalLayers() const
{
  const InDetDD::SiDetectorElementCollection* siDetElementCollectionPtr = m_siMgr->getDetectorElementCollection();
  return cylindricalLayersImpl(*siDetElementCollectionPtr);
}


/** LayerBuilder interface method - returning Endcap-like layers */
std::unique_ptr<const std::vector<Trk::DiscLayer* > > InDet::SiLayerBuilder::discLayers() const
{
  // sanity check for ID Helper
  if (!m_pixIdHelper && !m_sctIdHelper){
    ATH_MSG_ERROR("Neither Pixel nor SCT Detector Manager or ID Helper could be retrieved - giving up.");
    return nullptr;
  }

  // check for DBMS
  int nDBMLayers = m_siMgr->numerology().numEndcapsDBM();
  if (!nDBMLayers) return ((m_pixelCase and m_useRingLayout) ? createRingLayers() : createDiscLayers());

  ATH_MSG_DEBUG( "Found " << m_siMgr->numerology().numEndcapsDBM() << " DBM layers active, building first ECs, then DBMS");
  std::unique_ptr<std::vector<Trk::DiscLayer*> > ecLayers = createDiscLayers();
  if (ecLayers) {
    ATH_MSG_VERBOSE( "Created " << ecLayers->size() << " endcap layers w/o  DBM.");
    ecLayers = createDiscLayers(std::move(ecLayers));
    ATH_MSG_VERBOSE( "Created " << ecLayers->size() << " endcap layers with DBM.");
  }
  return ecLayers;

}


std::unique_ptr<std::vector< Trk::DiscLayer*> >
InDet::SiLayerBuilder::createDiscLayers(std::unique_ptr<std::vector< Trk::DiscLayer*> > discLayers) const {

  // get general layout
  const InDetDD::SiDetectorElementCollection* siDetElementCollectionPtr = m_siMgr->getDetectorElementCollection();
  return createDiscLayersImpl(*siDetElementCollectionPtr, std::move(discLayers));
}


/** LayerBuilder interface method - returning ring-like layers */
/** this is ITk pixel specific and doesn't include DBM modules */
std::unique_ptr<std::vector< Trk::DiscLayer*> >
InDet::SiLayerBuilder::createRingLayers() const {

  // get general layout
  const InDetDD::SiDetectorElementCollection* siDetElementCollectionPtr = m_siMgr->getDetectorElementCollection();
  return createRingLayersImpl(*siDetElementCollectionPtr);
}


void InDet::SiLayerBuilder::registerSurfacesToLayer(Trk::BinnedArraySpan<Trk::Surface * const >& layerSurfaces, Trk::Layer& lay) const
{
  if (!m_setLayerAssociation) return;

  Trk::BinnedArraySpan<Trk::Surface * const >::const_iterator laySurfIter    = layerSurfaces.begin();
  Trk::BinnedArraySpan<Trk::Surface * const >::const_iterator laySurfIterEnd = layerSurfaces.end();
  // register the surfaces to the layer
  for (; laySurfIter != laySurfIterEnd; ++laySurfIter){
    if (*laySurfIter) {
      // register the current surface --------------------------------------------------------
      // Needs care in Athena MT
      Trk::ILayerBuilder::associateLayer(lay, (**laySurfIter));
      const InDetDD::SiDetectorElement* detElement
        = dynamic_cast<const InDetDD::SiDetectorElement*>((*laySurfIter)->associatedDetectorElement());
      // register the backise if necessary ---------------------------------------------------
      const InDetDD::SiDetectorElement* otherSideElement = detElement ?  detElement->otherSide() : nullptr;
      const Trk::Surface* otherSideSurface = otherSideElement ? &(otherSideElement->surface()) : nullptr;
      if (otherSideSurface) {
        //Needs care in Athena MT
        //Note that we again couple directly to the det element surface not a
        Trk::ILayerBuilder::associateLayer(lay, const_cast<Trk::Surface&>(*otherSideSurface));
      }
    }
  }
}


