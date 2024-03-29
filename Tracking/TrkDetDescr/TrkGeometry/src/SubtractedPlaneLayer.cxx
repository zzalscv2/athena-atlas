/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SubtractedPlaneLayer.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/SubtractedPlaneLayer.h"

#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkParameters/TrackParameters.h"
// CLHEP
#include "GeoPrimitives/GeoPrimitives.h"

Trk::SubtractedPlaneLayer::SubtractedPlaneLayer(
    const SubtractedPlaneSurface* subtrPlaneSurf,
    const Trk::LayerMaterialProperties& laymatprop, double thickness,
    std::unique_ptr<Trk::OverlapDescriptor> olap, int laytyp)
    : SubtractedPlaneSurface(*subtrPlaneSurf),
      Layer(laymatprop, thickness, std::move(olap), laytyp) {}

Trk::SubtractedPlaneLayer::SubtractedPlaneLayer(
    const Trk::SubtractedPlaneLayer& play) = default;

Trk::SubtractedPlaneLayer::SubtractedPlaneLayer(
    const Trk::SubtractedPlaneLayer& play, const Amg::Transform3D& transf)
    : SubtractedPlaneSurface(play, transf), Layer(play) {}

Trk::SubtractedPlaneLayer& Trk::SubtractedPlaneLayer::operator=(
    const SubtractedPlaneLayer& play) {
  if (this != &play) {
    // call the assignments of the base classes
    Trk::SubtractedPlaneSurface::operator=(play);
    Trk::Layer::operator=(play);
  }
  return (*this);
}

const Trk::SubtractedPlaneSurface&
Trk::SubtractedPlaneLayer::surfaceRepresentation() const
{
  return (*this);
}

Trk::SubtractedPlaneSurface&
Trk::SubtractedPlaneLayer::surfaceRepresentation()
{
  return (*this);
}

double Trk::SubtractedPlaneLayer::preUpdateMaterialFactor(
    const Trk::TrackParameters& parm, Trk::PropDirection dir) const {
  if (!Trk::Layer::m_layerMaterialProperties) return 0.;
  if (Trk::SubtractedPlaneSurface::normal().dot(
          dir * parm.momentum().normalized()) > 0.)
    return Trk::Layer::m_layerMaterialProperties->alongPreFactor();
  return Trk::Layer::m_layerMaterialProperties->oppositePreFactor();
}

double Trk::SubtractedPlaneLayer::postUpdateMaterialFactor(
    const Trk::TrackParameters& parm, Trk::PropDirection dir) const {
  if (!Trk::Layer::m_layerMaterialProperties) return 0.;
  if (Trk::SubtractedPlaneSurface::normal().dot(
          dir * parm.momentum().normalized()) > 0.)
    return Trk::Layer::m_layerMaterialProperties->alongPostFactor();
  return Trk::Layer::m_layerMaterialProperties->oppositePostFactor();
}

void Trk::SubtractedPlaneLayer::moveLayer(Amg::Transform3D& shift) {
  m_transforms =
      std::make_unique<Transforms>(shift * (m_transforms->transform));
}
