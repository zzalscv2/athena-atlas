/*
Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/ReadHandle.h"
#include "xAODJet/Jet.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"

namespace Utils {
/// Provide the trk DCA w.r.t. the PV
double z0wrtPV(const xAOD::TrackParticle* trk, const xAOD::Vertex* vtx);

/// Finds the Primary Vertex
const xAOD::Vertex* selectPV(SG::ReadHandle<xAOD::VertexContainer>& container);

/// Find leading jet
const xAOD::Jet* findLeadingJet(SG::ReadHandle<xAOD::JetContainer>& container);
}  // namespace Utils
