/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/MuonClusterReadoutElement.h"
namespace MuonGM {

    MuonClusterReadoutElement::MuonClusterReadoutElement(GeoVFullPhysVol* pv, MuonDetectorManager* mgr, Trk::DetectorElemType detType) :
        MuonReadoutElement(pv, mgr, detType) {}

    MuonClusterReadoutElement::~MuonClusterReadoutElement() = default;

    void MuonClusterReadoutElement::shiftSurface(const Identifier&) { fillCache(); }

    void MuonClusterReadoutElement::restoreSurfaces() {
        m_surfaceData.reset();
        fillCache();
    }

}  // namespace MuonGM
