/*
Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "Utils.h"

namespace Utils {
double z0wrtPV(const xAOD::TrackParticle* trk, const xAOD::Vertex* vtx) {
  return (trk->z0() + trk->vz() - vtx->z()) * std::sin(trk->theta());
}

const xAOD::Vertex* selectPV(SG::ReadHandle<xAOD::VertexContainer>& container) {
  if (not container.isValid()) {
    return nullptr;
  }

  for (const xAOD::Vertex* vtx : *container) {
    if (vtx->vertexType() == xAOD::VxType::PriVtx) {
      return vtx;
    }
  }

  return nullptr;
}

const xAOD::Jet* findLeadingJet(SG::ReadHandle<xAOD::JetContainer>& container) {
  auto compareJetPt = [](const auto& jet1, const auto& jet2) { return jet1->pt() < jet2->pt(); };

  const auto leadingJet = std::max_element(container->begin(), container->end(), compareJetPt);
  if (leadingJet == container->end()) {
    return nullptr;
  }

  return *leadingJet;
}

}  // namespace Utils
