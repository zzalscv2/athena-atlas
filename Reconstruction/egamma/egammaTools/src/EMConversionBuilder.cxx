/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
NAME:     EMConversionBuilder.cxx
PACKAGE:  offline/Reconstruction/egamma/egammaRec

AUTHORS:  D. Zerwas, B. Lenzi , C. Anastopoulos
CREATED:  Jul, 2005
CHANGES:  Mar, 2014 (BL) xAOD migration
CHANGES:  2020 (CA) Athena MT migration

PURPOSE:  subAlgorithm which creates an EMConversion object.

********************************************************************/

// INCLUDE HEADER FILES:

#include "EMConversionBuilder.h"
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandle.h"
#include "egammaRecEvent/egammaRec.h"
#include "egammaRecEvent/egammaRecContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

//  END OF HEADER FILES INCLUDE

/////////////////////////////////////////////////////////////////

namespace {
/** Sort conversion vertices according to the following criteria:
 - Vertices with more Si tracks have priority
 - Vertices with more tracks have priority
 - Vertices with smaller radii have priority

 OLD SCHEME:
 - Vertices with 2 tracks have priority over the ones with 1 track
 - Vertices with Si + Si tracks have priority (if m_preferSi > 0)
 - Vertices with Si + TRT or TRT + TRT depending on m_preferSi
 - Vertices with smaller radii have priority
 **/
bool
ConvVxSorter(const xAOD::Vertex& vx1, const xAOD::Vertex& vx2)
{
  xAOD::EgammaParameters::ConversionType convType1;
  xAOD::EgammaParameters::ConversionType convType2;
  convType1 = xAOD::EgammaHelpers::conversionType(&vx1);
  convType2 = xAOD::EgammaHelpers::conversionType(&vx2);

  if (convType1 != convType2) {
    // Different conversion type, preference to vertices with Si tracks
    int nSi1 = xAOD::EgammaHelpers::numberOfSiTracks(convType1);
    int nSi2 = xAOD::EgammaHelpers::numberOfSiTracks(convType2);
    if (nSi1 != nSi2)
      return nSi1 > nSi2;

    // Same number of Si tracks: either 0 or 1 (Si+TRT vs. Si single)
    // For 1 Si track, preference to Si+TRT
    if (nSi1 != 0)
      return convType1 == xAOD::EgammaParameters::doubleSiTRT;

    // No Si track, preference to doubleTRT over single TRT
    return convType1 == xAOD::EgammaParameters::doubleTRT;
  }

  // Same conversion type, preference to lower radius
  return (vx1.position().perp() < vx2.position().perp());
}
} // end of namespace

using namespace xAOD::EgammaParameters;

EMConversionBuilder::EMConversionBuilder(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
  : AthAlgTool(type, name, parent)
{

  // declare interface
  declareInterface<IEMConversionBuilder>(this);
}

StatusCode
EMConversionBuilder::initialize()
{

  ATH_MSG_DEBUG("Initializing EMConversionBuilder");

  ATH_CHECK(m_conversionContainerKey.initialize());

  // the extrapolation tool
  if (m_extrapolationTool.retrieve().isFailure()) {
    ATH_MSG_ERROR("Cannot retrieve extrapolationTool " << m_extrapolationTool);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved extrapolationTool " << m_extrapolationTool);

  return StatusCode::SUCCESS;
}

StatusCode
EMConversionBuilder::executeRec(const EventContext& ctx, egammaRec* egRec) const
{
  // retrieve Conversion Container

  SG::ReadHandle<xAOD::VertexContainer> conversions(m_conversionContainerKey,
                                                    ctx);

  // only for serial running; remove for MT
  ATH_CHECK(conversions.isValid());
  // reset the vertices
  std::vector<ElementLink<xAOD::VertexContainer>> vertices;
  egRec->setVertices(vertices);
  ATH_CHECK(vertexExecute(ctx, egRec, conversions.cptr()));
  return StatusCode::SUCCESS;
}

StatusCode
EMConversionBuilder::vertexExecute(
  const EventContext& ctx,
  egammaRec* egRec,
  const xAOD::VertexContainer* conversions) const
{

  if (!egRec || !conversions) {
    ATH_MSG_WARNING(
      "trackExecute: NULL pointer to egammaRec or VertexContainer");
    return StatusCode::SUCCESS;
  }

  static const SG::AuxElement::Accessor<float> accetaAtCalo("etaAtCalo");
  static const SG::AuxElement::Accessor<float> accphiAtCalo("phiAtCalo");

  float etaAtCalo(0);
  float phiAtCalo(0);
  for (unsigned int iVtx = 0; iVtx < conversions->size(); ++iVtx) {

    const xAOD::Vertex* vertex = conversions->at(iVtx);
    // Check if vertex was already decorated with etaAtCalo, phiAtCalo
    if (accetaAtCalo.isAvailable(*vertex) &&
        accphiAtCalo.isAvailable(*vertex)) {
      etaAtCalo = accetaAtCalo(*vertex);
      phiAtCalo = accphiAtCalo(*vertex);
    }
    // check extrapolation, skip vertex in case of failure
    else if (!m_extrapolationTool->getEtaPhiAtCalo(
               ctx, vertex, &etaAtCalo, &phiAtCalo)) {
      continue;
    }
    const xAOD::CaloCluster* cluster = egRec->caloCluster();
    if (!passPtAndEoverP(ctx, *vertex, *cluster)) {
      continue;
    }
    if (!m_extrapolationTool->matchesAtCalo(
          cluster, vertex, etaAtCalo, phiAtCalo)) {
      continue;
    }
    const ElementLink<xAOD::VertexContainer> vertexLink(*conversions, iVtx, ctx);

    // If this is the best (or the first) vertex, push front and keep deltaEta,
    // deltaPhi
    if (!egRec->getNumberOfVertices() ||
        ConvVxSorter(*vertex, *egRec->vertex())) {
      egRec->pushFrontVertex(vertexLink);
      egRec->setDeltaEtaVtx(cluster->etaBE(2) - etaAtCalo);
      egRec->setDeltaPhiVtx(P4Helpers::deltaPhi(cluster->phiBE(2), phiAtCalo));
    } else { // Not the best vertex, push back
      egRec->pushBackVertex(vertexLink);
    }
  }
  return StatusCode::SUCCESS;
}

bool
EMConversionBuilder::passPtAndEoverP(const EventContext& ctx,
                                     const xAOD::Vertex& vertex,
                                     const xAOD::CaloCluster& cluster) const
{
  Amg::Vector3D momentum =
    m_extrapolationTool->getMomentumAtVertex(ctx, vertex);
  float pt = momentum.perp();
  float EoverP = cluster.e() / momentum.mag();

  auto convType = xAOD::EgammaHelpers::conversionType(&vertex);
  bool isSingle = (convType == singleTRT || convType == singleSi);
  bool isTRT =
    (convType == singleTRT || convType == xAOD::EgammaParameters::doubleTRT);
  float EoverPcut = m_maxEoverP_singleTrack *
                    (1 + m_maxEoverP_singleTrack_EtSf * cluster.et() * 1e-3);

  // Check TRT tube hit fraction
  float tubeHitFraction = getMaxTRTTubeHitFraction(vertex);
  if (isTRT && tubeHitFraction > m_maxTRTTubeHitFraction) {
    ATH_MSG_DEBUG("Conversion failed cut on TRT tube hit fraction: "
                  << tubeHitFraction << " vs. " << m_maxTRTTubeHitFraction);
    return false;
  }

  bool reject =
    ((isTRT && m_rejectAllTRT) || (isSingle && pt < m_minPt_singleTrack) ||
     (!isSingle && pt < m_minSumPt_double) ||
     (isSingle && EoverP > EoverPcut) ||
     (convType == singleTRT && pt < m_minPt_singleTRT) ||
     (convType == doubleTRT && pt < m_minSumPt_doubleTRT));

  if (reject) {
    ATH_MSG_DEBUG("Conversion failed pt or E/p cuts");
  }
  return !reject;
}

float
EMConversionBuilder::getMaxTRTTubeHitFraction(const xAOD::Vertex& vertex) const
{
  auto getTRTTubeHitFraction = [](const xAOD::TrackParticle* trk) {
    uint8_t nTRT;
    uint8_t nTRTTube;
    if (!trk || !trk->summaryValue(nTRT, xAOD::numberOfTRTHits) || !nTRT)
      return 0.;
    return trk->summaryValue(nTRTTube, xAOD::numberOfTRTTubeHits)
             ? 1. * nTRTTube / nTRT
             : 0.;
  };

  float maxTubeHitFraction = 0.;
  for (unsigned int i = 0; i < vertex.nTrackParticles(); ++i) {
    if (!vertex.trackParticle(i)) {
      ATH_MSG_WARNING("NULL pointer to track particle in conversion vertex");
    } else {
      float tubeHitFraction = getTRTTubeHitFraction(vertex.trackParticle(i));
      if (tubeHitFraction > maxTubeHitFraction) {
        maxTubeHitFraction = tubeHitFraction;
      }
    }
  }
  return maxTubeHitFraction;
}
