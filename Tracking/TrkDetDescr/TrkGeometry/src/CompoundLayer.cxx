/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CompoundLayer.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/CompoundLayer.h"

bool Trk::CompoundLayer::isOnCompoundLayer(const Amg::Vector3D &gp,
                                           double thickness) const {
  // try each surface in turn
  bool onSurface = false;
  for (unsigned int i = 0; !onSurface && i < m_surfaces.size(); ++i) {
    onSurface = m_surfaces[i]->isOnSurface(gp, true, thickness);
  }
  return onSurface;
}

Trk::CompoundLayer::~CompoundLayer() {
  for (auto & surface : m_surfaces) {
    delete surface;
  }
}
