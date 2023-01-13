/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Trk include
#include "TrkDetDescrTools/LayerProvider.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"

// constructor
Trk::LayerProvider::LayerProvider(const std::string& t, const std::string& n, const IInterface* p)
: base_class(t,n,p)
{
}

// initialize
StatusCode Trk::LayerProvider::initialize()
{
    if (m_layerBuilder.retrieve().isFailure()){
        ATH_MSG_WARNING("Could not retrieve layer builder");
    }
    return StatusCode::SUCCESS;
}

/** LayerBuilder interface method - returning the central layers */
const std::vector<Trk::Layer*> Trk::LayerProvider::centralLayers() const
{
    // retrieving the cylinder layers from the layer builder
    std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
      cylinderLayers = m_layerBuilder->cylindricalLayers();
  return cylindricalLayersToCentralLayers(std::move(cylinderLayers));
}

/** LayerBuilder interface method - returning the endcap layer */
std::pair<const std::vector<Trk::Layer*>, const std::vector<Trk::Layer*> >
Trk::LayerProvider::endcapLayer() const
{
  // retrieving the cylinder layers from the layer builder
  std::unique_ptr<const std::vector<Trk::DiscLayer*> > discLayers =
    m_layerBuilder->discLayers();
  return discLayersToEndcapLayers(std::move(discLayers));
}

const std::string& Trk::LayerProvider::identification() const
{
    return m_layerBuilder->identification();
}
