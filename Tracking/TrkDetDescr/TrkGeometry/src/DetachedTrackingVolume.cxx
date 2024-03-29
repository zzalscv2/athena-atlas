/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DetachedTrackingVolume.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/DetachedTrackingVolume.h"

#include <utility>

#include "TrkGeometry/Layer.h"
#include "TrkGeometry/TrackingVolume.h"

Trk::DetachedTrackingVolume::DetachedTrackingVolume()
  : m_trkVolume()
  , m_name("undefined")
  , m_layerRepresentation(nullptr)
  , m_multilayerRepresentation(nullptr)
  , m_baseTransform(nullptr)
  , m_constituents(nullptr)
{}

Trk::DetachedTrackingVolume::DetachedTrackingVolume(std::string name,
                                                    Trk::TrackingVolume* volume)
  : m_trkVolume(volume)
  , m_name(std::move(name))
  , m_layerRepresentation(nullptr)
  , m_multilayerRepresentation(nullptr)
  , m_baseTransform(nullptr)
  , m_constituents(nullptr)
{}

Trk::DetachedTrackingVolume::DetachedTrackingVolume(
  std::string name,
  Trk::TrackingVolume* volume,
  Trk::Layer* lay,
  const std::vector<Trk::Layer*>* multilay)
  : m_trkVolume(volume)
  , m_name(std::move(name))
  , m_layerRepresentation(lay)
  , m_multilayerRepresentation(multilay)
  , m_baseTransform(nullptr)
  , m_constituents(nullptr)
{}

Trk::DetachedTrackingVolume::~DetachedTrackingVolume() {
  delete m_trkVolume;
  if (m_layerRepresentation) delete m_layerRepresentation;
  if (m_multilayerRepresentation) {
    for (auto *layer : *m_multilayerRepresentation)
      delete layer;
    delete m_multilayerRepresentation;
  }
  delete m_baseTransform;
}

void
Trk::DetachedTrackingVolume::move(Amg::Transform3D& shift)
{
  m_trkVolume->moveTV(shift);
  if (m_layerRepresentation) {
    m_layerRepresentation->moveLayer(shift);
  }
  if (m_multilayerRepresentation) {
    for (auto *layer : *m_multilayerRepresentation) {
      layer->moveLayer(shift);
    }
  }
}

Trk::DetachedTrackingVolume*
Trk::DetachedTrackingVolume::clone(const std::string& name,
                                   Amg::Transform3D& shift) const
{
  Trk::TrackingVolume* newTV =
    new TrackingVolume(*(this->trackingVolume()), shift);
  Trk::DetachedTrackingVolume* newStat = nullptr;
  // layer representation ?
  Trk::PlaneLayer* newLay = nullptr;
  if (this->layerRepresentation()) {
    std::vector<Trk::Layer*>* newMulti = nullptr;
    const Trk::PlaneLayer* pl =
        dynamic_cast<const Trk::PlaneLayer*>(this->layerRepresentation());
    if (pl) {
      newLay = new Trk::PlaneLayer(*pl);
      newLay->moveLayer(shift);
      if (!this->multilayerRepresentation().empty()) {
        newMulti = new std::vector<Trk::Layer*>;
        for (unsigned int i = 0; i < this->multilayerRepresentation().size();
             i++) {
          const Trk::PlaneLayer* mpl = dynamic_cast<const Trk::PlaneLayer*>(
            (this->multilayerRepresentation())[i]);
          if (mpl) {
            Trk::PlaneLayer* newPl = new Trk::PlaneLayer(*mpl);
            newPl->moveLayer(shift);
            newMulti->push_back(newPl);
          } else
            std::cout << "WARNING   Trk::DetachedTrackingVolume::clone()   "
                         "dynamic cast to 'const Trk::PlaneLayer* mpl' failed!"
                      << std::endl;
        }
      }
      newStat = new Trk::DetachedTrackingVolume(name, newTV, newLay, newMulti);
    } else {
      std::cout << "WARNING   Trk::DetachedTrackingVolume::clone()   dynamic "
                   "cast to 'const Trk::PlaneLayer* pl' failed!"
                << std::endl;
      newStat = new Trk::DetachedTrackingVolume(name, newTV);
    }
  } else {
    newStat = new Trk::DetachedTrackingVolume(name, newTV);
  }
  //
  // enclose layers
  if (newTV->confinedVolumes()) {
    BinnedArraySpan<Trk::TrackingVolume * const> vols =
        newTV->confinedVolumes()->arrayObjects();
    for (auto *vol : vols) {
      Trk::LayerArray* layAr = vol->confinedLayers();
      Trk::ArraySpan<Trk::Layer* const> alays =
          vol->confinedArbitraryLayers();
      if (layAr) {
        Trk::BinnedArraySpan<Trk::Layer* const> lays = layAr->arrayObjects();
        for (auto *lay : lays) {
          lay->encloseDetachedTrackingVolume(*newStat);
        }
      }
      if (!alays.empty()) {
        for (auto *alay : alays) {
          alay->encloseDetachedTrackingVolume(*newStat);
        }
      }
    }
  }
  if (newTV->confinedLayers()) {
    BinnedArraySpan<Trk::Layer* const> lays =
        newTV->confinedLayers()->arrayObjects();
    for (auto *lay : lays){
      lay->encloseDetachedTrackingVolume(*newStat);
    }
  }
  if (!newTV->confinedArbitraryLayers().empty()) {
    Trk::ArraySpan<Trk::Layer* const> alays =
        newTV->confinedArbitraryLayers();
    for (auto *alay : alays) {
      alay->encloseDetachedTrackingVolume(*newStat);
    }
  }
  //
  newStat->saveConstituents(this->constituents());
  return newStat;
}

void
Trk::DetachedTrackingVolume::compactify(size_t& cSurfaces, size_t& tSurfaces)
{
  // deal with the Tracking Volume representation
  if (m_trkVolume)
    m_trkVolume->compactify(cSurfaces, tSurfaces);

  // deal with the layer representation
  if (layerRepresentation()) {
    ++tSurfaces;
    if (layerRepresentation()->surfaceRepresentation().owner() == Trk::noOwn) {
      layerRepresentation()->surfaceRepresentation().setOwner(Trk::TGOwn);
      ++cSurfaces;
    }
  }
  // deal with the multi-layer representation
  if (!multilayerRepresentation().empty()) {
    tSurfaces += m_multilayerRepresentation->size();
    for (const auto& mLayerIter : (*m_multilayerRepresentation)) {
      if ((*mLayerIter).surfaceRepresentation().owner() == Trk::noOwn) {
        (*mLayerIter).surfaceRepresentation().setOwner(Trk::TGOwn);
        ++cSurfaces;
      }
    }
  }
  //<< !@ TODO include volumes
}

void
Trk::DetachedTrackingVolume::sign(GeometrySignature signat,
                                  GeometryType geotype)
{
  m_trkVolume->sign(signat, geotype);
}

Trk::GeometrySignature Trk::DetachedTrackingVolume::geometrySignature() const {
  return m_trkVolume->geometrySignature();
}

Trk::GeometryType Trk::DetachedTrackingVolume::geometryType() const {
  return m_trkVolume->geometryType();
}

void Trk::DetachedTrackingVolume::setBaseTransform(Amg::Transform3D* transf) {
  if (transf)
    m_baseTransform = transf;
  else {
    delete m_baseTransform;
    m_baseTransform = new Amg::Transform3D(this->trackingVolume()->transform());
  }
}

