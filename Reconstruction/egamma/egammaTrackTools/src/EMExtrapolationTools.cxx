/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "EMExtrapolationTools.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParameters/TrackParameters.h"

// extrapolation
#include "InDetIdentifier/TRT_ID.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkNeutralParameters/NeutralParameters.h"
//
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
//
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkTrack/Track.h"
//
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/EventContext.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
//
#include <tuple>

namespace {
/*
 * Create Rescaled Perigee Parametrs
 */
Trk::Perigee
getRescaledPerigee(const xAOD::TrackParticle& trkPB,
                   const xAOD::CaloCluster& cluster)
{
  // copy over the one of the input track particle
  Trk::Perigee perigee = trkPB.perigeeParameters();
  // we rescale q/p
  AmgVector(5) rescaled;
  rescaled << trkPB.d0(), trkPB.z0(), trkPB.phi0(), trkPB.theta(),
    trkPB.charge() / cluster.e();
  perigee.setParameters(rescaled);

  return perigee;
}

template <std::size_t N>
void
getClusterLayers(const std::array<CaloSampling::CaloSample, N>& srcLayers,
                 std::vector<CaloSampling::CaloSample>& destLayers,
                 const xAOD::CaloCluster& cluster) {
  for (const CaloSampling::CaloSample lay : srcLayers) {
    if (cluster.hasSampling(lay)) {
      destLayers.emplace_back(lay);
    }
  }
}

} // end of anonymous namespace

EMExtrapolationTools::EMExtrapolationTools(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_trtId(nullptr)
{
  declareInterface<IEMExtrapolationTools>(this);
}

EMExtrapolationTools::~EMExtrapolationTools() = default;

StatusCode
EMExtrapolationTools::initialize()
{

  ATH_MSG_DEBUG("Initializing " << name() << "...");
  // Retrieve tools
  ATH_CHECK(m_ParticleCaloExtensionTool.retrieve());
  ATH_CHECK(m_extrapolator.retrieve());

  // retrieve TRT-ID helper
  if (m_enableTRT && detStore()->contains<TRT_ID>("TRT_ID")) {
    StatusCode sc = detStore()->retrieve(m_trtId, "TRT_ID");
    if (sc.isFailure() || !m_trtId->is_valid()) {
      // TRT is not present for sLHC
      ATH_MSG_DEBUG("Could not get TRT_ID helper !");
      m_trtId = nullptr;
    }
    ATH_MSG_DEBUG("m_trtId initialization successful");
  } else {
    ATH_MSG_DEBUG("Could not get TRT_ID helper !");
  }
  return StatusCode::SUCCESS;
}

std::pair<std::vector<CaloSampling::CaloSample>,
          std::vector<std::unique_ptr<Trk::Surface>>>
EMExtrapolationTools::getClusterLayerSurfaces(
  const xAOD::CaloCluster& cluster,
  const CaloDetDescrManager& caloDD) const
{
  // figure which layer we need
  // based on the where most of the energy of the cluster
  // is we might want to do EM barrel, EM endCap
  // or forward calo layers/samplings
  constexpr std::array<CaloSampling::CaloSample, 4> barrelLayers = {
    CaloSampling::PreSamplerB,
    CaloSampling::EMB1,
    CaloSampling::EMB2,
    CaloSampling::EMB3
  };
  constexpr std::array<CaloSampling::CaloSample, 4> endcapLayers = {
    CaloSampling::PreSamplerE,
    CaloSampling::EME1,
    CaloSampling::EME2,
    CaloSampling::EME3
  };
  constexpr std::array<CaloSampling::CaloSample, 4> endcapLayersAboveEta2p5 = {
    CaloSampling::EME2,
    CaloSampling::EME3
  };
  constexpr std::array<CaloSampling::CaloSample, 1> forwardLayers = {
    CaloSampling::FCAL0,
  };

  std::vector<CaloSampling::CaloSample> clusterLayers;
  clusterLayers.reserve(4);

  if (cluster.inBarrel() && (!cluster.inEndcap() || 
                             cluster.eSample(CaloSampling::EMB2) >=
                             cluster.eSample(CaloSampling::EME2))) {
      getClusterLayers(barrelLayers, clusterLayers, cluster);
  }
  else if (cluster.eSample(CaloSampling::EME2) >
           cluster.eSample(CaloSampling::FCAL0)) {
    if(std::abs(cluster.eta()) < 2.5){
      getClusterLayers(endcapLayers, clusterLayers, cluster);
    }
    else {
      getClusterLayers(endcapLayersAboveEta2p5, clusterLayers, cluster);
    }
  }
  else {
    getClusterLayers(forwardLayers, clusterLayers, cluster);
  }

  std::vector<std::unique_ptr<Trk::Surface>> caloSurfaces =
    m_ParticleCaloExtensionTool->caloSurfacesFromLayers(
      clusterLayers, cluster.eta(), caloDD);

  return { std::move(clusterLayers), std::move(caloSurfaces) };
}

/*
 * This is the method that does the heavy lifting for the
 * electrons extrapolations. Handles multipe extrapolation modes.
 */
StatusCode
EMExtrapolationTools::getMatchAtCalo(
  const EventContext& ctx,
  const xAOD::CaloCluster& cluster,
  const xAOD::TrackParticle& trkPB,
  const std::vector<CaloSampling::CaloSample>& samples,
  const std::vector<std::unique_ptr<Trk::Surface>>& surfaces,
  std::array<double, 4>& eta,
  std::array<double, 4>& phi,
  std::array<double, 4>& deltaEta,
  std::array<double, 4>& deltaPhi,
  unsigned int extrapFrom) const
{
  /* Extrapolate track to calo and return
   * the extrapolated eta/phi and
   * the deta/dphi between cluster and track
   * We allow different ways to extrapolate:
   * 1) from the last measurement  track parameters (this is always the case for
   * TRT standalone)
   * 2) from the perigee track parameters
   * 3) from the perigee
   * with the track momentum rescaled by the cluster energy
   */
  if (cluster.e() < 10 && trkPB.pt() < 10) { // This is 10 MeV
    ATH_MSG_WARNING("Too small cluster E :"
                    << cluster.e() << " , or too small track pt" << trkPB.pt());
    return StatusCode::FAILURE;
  }

  bool didExtension = false;
  CaloExtensionHelpers::EtaPhiPerLayerVector intersections;
  intersections.reserve(samples.size());

  switch (extrapFrom) {
    case fromPerigeeRescaled: {
      Trk::Perigee trkPar = getRescaledPerigee(trkPB, cluster);
      const auto extension = m_ParticleCaloExtensionTool->surfaceCaloExtension(
        ctx, trkPar, samples, surfaces, Trk::nonInteracting);
      didExtension = !extension.empty();
      for (const auto& i : extension) {
        intersections.emplace_back(
          i.first, i.second->position().eta(), i.second->position().phi());
      }
    } break;

    case fromPerigee: {
      const auto extension = m_ParticleCaloExtensionTool->surfaceCaloExtension(
        ctx, trkPB.perigeeParameters(), samples, surfaces, Trk::nonInteracting);
      didExtension = !extension.empty();
      for (const auto& i : extension) {
        intersections.emplace_back(
          i.first, i.second->position().eta(), i.second->position().phi());
      }
    } break;

    case fromLastMeasurement: {
      unsigned int index(0);
      if (trkPB.indexOfParameterAtPosition(index, xAOD::LastMeasurement)) {
        const Trk::CurvilinearParameters& lastParams =
          trkPB.curvilinearParameters(index);
        const Amg::Vector3D& position = lastParams.position();
        // Calo entry around z EME1 3750  and r  EMB1 1550
        if (position.perp() > 1550. || std::abs(position.z()) > 3750.) {
          ATH_MSG_WARNING("Probematic last parameters : " << lastParams);
          didExtension = false;
        } else {
          const auto extension =
            m_ParticleCaloExtensionTool->surfaceCaloExtension(
              ctx, lastParams, samples, surfaces, Trk::nonInteracting);
          didExtension = !extension.empty();
          for (const auto& i : extension) {
            intersections.emplace_back(
              i.first, i.second->position().eta(), i.second->position().phi());
          }
        }
      }
    } break;

    default: {
      ATH_MSG_ERROR("Invalid ExtrapolateFrom " << extrapFrom);
    }
  }
  /*
   * Given the extension calculate the deta/dphi for the layers
   */
  if (!didExtension) {
    ATH_MSG_DEBUG("Could not create an extension from "
                  << extrapFrom << " for a track with : "
                  << " Track Pt " << trkPB.pt() << " Track Eta " << trkPB.eta()
                  << " Track Phi " << trkPB.phi() << " Track Fitter "
                  << trkPB.trackFitter());
    return StatusCode::FAILURE;
  }
  // Negative tracks bend to the positive direction.
  // flip sign for positive ones
  const bool flipSign = trkPB.charge() > 0;

  for (const auto& p : intersections) {
    int i(0);
    CaloSampling::CaloSample sample = std::get<0>(p);
    if (sample == CaloSampling::PreSamplerE ||
        sample == CaloSampling::PreSamplerB) {
      i = 0;
    } else if (sample == CaloSampling::EME1 || sample == CaloSampling::EMB1) {
      i = 1;
    } else if (sample == CaloSampling::EME2 || sample == CaloSampling::EMB2 || sample == CaloSampling::FCAL0) {
      i = 2;
    } else if (sample == CaloSampling::EME3 || sample == CaloSampling::EMB3) {
      i = 3;
    } else {
      continue;
    }
    eta[i] = std::get<1>(p);
    phi[i] = std::get<2>(p);
    deltaEta[i] = cluster.etaSample(sample) - std::get<1>(p);
    deltaPhi[i] =
      P4Helpers::deltaPhi(cluster.phiSample(sample), std::get<2>(p));
    // Should we flip the sign for deltaPhi?
    if (flipSign) {
      deltaPhi[i] = -deltaPhi[i];
    }
    ATH_MSG_DEBUG("getMatchAtCalo: i, eta, phi, deta, dphi: "
                  << i << " " << eta[i] << " " << phi[i] << " " << deltaEta[i]
                  << " " << deltaPhi[i]);
  }
  return StatusCode::SUCCESS;
}

/*
 * Photon/vertex/Neutral track parameters
 * Related methods
 * needed for  Extrapolating/matching conversions
 */
bool
EMExtrapolationTools::matchesAtCalo(const xAOD::CaloCluster* cluster,
                                    const xAOD::Vertex* vertex,
                                    float etaAtCalo,
                                    float phiAtCalo) const
{
  if (!cluster || !vertex) {
    return false;
  }
  float deltaEta = fabs(etaAtCalo - cluster->etaBE(2));
  float deltaPhi = fabs(P4Helpers::deltaPhi(cluster->phi(), phiAtCalo));

  int TRTsection = 0;
  if (xAOD::EgammaHelpers::numberOfSiTracks(vertex) == 0)
    TRTsection = getTRTsection(vertex->trackParticle(0));

  // First pass on TRT tracks, skip barrel tracks matching endcap clusters and
  // vice-versa
  if ((TRTsection == 2 && (cluster->eta() <= 0.6 || cluster->eta() >= 2.4)) ||
      (TRTsection == -2 &&
       (cluster->eta() >= -0.6 || cluster->eta() <= -2.4)) ||
      (TRTsection == 1 && (cluster->eta() <= -0.1 || cluster->eta() >= 1.3)) ||
      (TRTsection == -1 && (cluster->eta() >= 0.1 || cluster->eta() <= -1.3))) {
    return false;
  }

  // The maximum deltaEta/deltaPhi for Si, TRT barrel, TRT endcap
  static const std::vector<double> dEtaV{ m_narrowDeltaEta,
                                          m_TRTbarrelDeltaEta,
                                          m_TRTendcapDeltaEta };
  static const std::vector<double> dPhiV{ m_narrowDeltaPhi,
                                          m_narrowDeltaPhiTRTbarrel,
                                          m_narrowDeltaPhiTRTendcap };

  return (deltaEta < dEtaV[abs(TRTsection)] &&
          deltaPhi < dPhiV[abs(TRTsection)]);
}
/*
 * The following two are the heavy lifting methods.
 * Start from vertex/Track Parameters
 * and then calculate the eta/phi at calo
 */
bool
EMExtrapolationTools::getEtaPhiAtCalo(const EventContext& ctx,
                                      const xAOD::Vertex* vertex,
                                      float* etaAtCalo,
                                      float* phiAtCalo) const
{
  if (!vertex) {
    return false;
  }
  Amg::Vector3D momentum = getMomentumAtVertex(ctx, *vertex);
  if (momentum.mag() < 1e-5) {
    ATH_MSG_DEBUG("Intersection failed");
    return false;
  }
  /*
   * Create high pt track parameters to mimic a neutral particle.
   * This in principle is an approximation
   */
  Trk::PerigeeSurface surface(vertex->position());
  Trk::Perigee trkPar(
    vertex->position(), momentum.unit() * 1.e10, +1, surface, std::nullopt);
  bool success = getEtaPhiAtCalo(ctx, &trkPar, etaAtCalo, phiAtCalo);
  return success;
}
/*  The actual calculation happens here*/
bool
EMExtrapolationTools::getEtaPhiAtCalo(const EventContext& ctx,
                                      const Trk::TrackParameters* trkPar,
                                      float* etaAtCalo,
                                      float* phiAtCalo) const
{
  if (!trkPar)
    return false;
  CaloExtensionHelpers::LayersToSelect layersToSelect;
  if (fabs(trkPar->eta()) < 1.425) {
    // Barrel
    layersToSelect.insert(CaloSampling::EMB2);
  } else {
    // Endcap
    layersToSelect.insert(CaloSampling::EME2);
  }

  std::unique_ptr<Trk::CaloExtension> extension = nullptr;
  extension = m_ParticleCaloExtensionTool->caloExtension(
    ctx, *trkPar, Trk::alongMomentum, Trk::muon);
  if (!extension) {
    ATH_MSG_WARNING("Could not create an extension from getEtaPhiAtCalo ");
    return false;
  }
  CaloExtensionHelpers::EtaPhiPerLayerVector intersections;
  CaloExtensionHelpers::midPointEtaPhiPerLayerVector(
    *extension, intersections, &layersToSelect);
  bool hitEM2(false);
  for (const auto& p : intersections) {
    int i(0);
    auto sample = std::get<0>(p);
    if ((sample == CaloSampling::EME2 || sample == CaloSampling::EMB2)) {
      *etaAtCalo = std::get<1>(p);
      *phiAtCalo = std::get<2>(p);
      hitEM2 = true;
      ++i;
      ATH_MSG_DEBUG("getMatchAtCalo: i, eta, phi : "
                    << i << " " << std::get<1>(p) << " " << std::get<2>(p));
    }
  }
  return hitEM2;
}

/* Methods to get the momemtum at the conversion vertex*/
Amg::Vector3D
EMExtrapolationTools::getMomentumAtVertex(const EventContext& ctx,
                                          const xAOD::Vertex& vertex,
                                          unsigned int index) const
{
  Amg::Vector3D momentum(0., 0., 0.);
  if (vertex.nTrackParticles() <= index) {
    ATH_MSG_WARNING("Invalid track index");
  } else if (vertex.vxTrackAtVertexAvailable() &&
             !vertex.vxTrackAtVertex().empty()) {
    // Use the parameters at the vertex
    // (the tracks should be parallel but we will do the sum anyway)
    ATH_MSG_DEBUG("getMomentumAtVertex : getting from vxTrackAtVertex");
    const auto& trkAtVertex = vertex.vxTrackAtVertex()[index];
    const Trk::TrackParameters* paramAtVertex = trkAtVertex.perigeeAtVertex();
    if (!paramAtVertex) {
      ATH_MSG_WARNING("VxTrackAtVertex does not have perigee at vertex");
    } else {
      return paramAtVertex->momentum();
    }
  } else if (vertex.nTrackParticles() == 1) {
    // Use the first measurement
    ATH_MSG_DEBUG(
      "getMomentumAtVertex : 1 track only, getting from first measurement");
    const xAOD::TrackParticle* tp = vertex.trackParticle(0);
    unsigned int paramindex(0);
    if (!tp ||
        !tp->indexOfParameterAtPosition(paramindex, xAOD::FirstMeasurement)) {
      ATH_MSG_WARNING("No TrackParticle or no have first measurement");
    } else {
      momentum += tp->curvilinearParameters(paramindex).momentum();
    }
  } else {
    // Extrapolate track particle to vertex
    ATH_MSG_DEBUG("getMomentumAtVertex : extrapolating to perigee surface");
    const xAOD::TrackParticle* tp = vertex.trackParticle(index);
    if (!tp) {
      ATH_MSG_WARNING("NULL pointer to TrackParticle in vertex");
    } else {
      Trk::PerigeeSurface surface(vertex.position());
      std::unique_ptr<const Trk::TrackParameters> params =
          m_extrapolator->extrapolate(ctx, tp->perigeeParameters(), surface,
                                      Trk::alongMomentum);
      if (!params) {
        ATH_MSG_DEBUG("Extrapolation to vertex (perigee) failed");
      } else {
        momentum += params->momentum();
      }
    }
  }
  return momentum;
}

Amg::Vector3D
EMExtrapolationTools::getMomentumAtVertex(const EventContext& ctx,
                                          const xAOD::Vertex& vertex,
                                          bool reuse /* = true */) const
{
  Amg::Vector3D momentum(0., 0., 0.);
  const static SG::AuxElement::Accessor<float> accPx("px");
  const static SG::AuxElement::Accessor<float> accPy("py");
  const static SG::AuxElement::Accessor<float> accPz("pz");
  if (vertex.nTrackParticles() == 0) {
    ATH_MSG_WARNING("getMomentumAtVertex : vertex has no track particles!");
    return momentum;
  }
  if (reuse && accPx.isAvailable(vertex) && accPy.isAvailable(vertex) &&
      accPz.isAvailable(vertex)) {
    // Already decorated with parameters at vertex
    ATH_MSG_DEBUG("getMomentumAtVertex : getting from auxdata");
    return { accPx(vertex), accPy(vertex), accPz(vertex) };
  }
  for (unsigned int i = 0; i < vertex.nTrackParticles(); ++i) {
    momentum += getMomentumAtVertex(ctx, vertex, i);
  }

  return momentum;
}
/*
 * Helper to identify the TRT section
 */
int
EMExtrapolationTools::getTRTsection(const xAOD::TrackParticle* trkPB) const
{
  if (!trkPB) {
    ATH_MSG_DEBUG("Null pointer to TrackParticle");
    return 0;
  }
  if (!m_trtId) {
    ATH_MSG_DEBUG(
      "No trt ID guessing TRT section based on eta: " << trkPB->eta());
    return (trkPB->eta() > 0 ? 1 : -1) * (fabs(trkPB->eta()) < 0.6 ? 1 : 2);
  }
  const Trk::MeasurementBase* trkPar = nullptr;
  if (trkPB->trackLink().isValid() && trkPB->track() != nullptr) {
    ATH_MSG_DEBUG("Will get TrackParameters from Trk::Track");
    const Trk::TrackStates* trackStates =
      trkPB->track()->trackStateOnSurfaces();
    if (!trackStates) {
      ATH_MSG_WARNING("NULL pointer to trackStateOnSurfaces");
      return 0;
    }
    // Loop over the TrkStateOnSurfaces search last valid TSOS first
    for (Trk::TrackStates::const_reverse_iterator
           rItTSoS = trackStates->rbegin();
         rItTSoS != trackStates->rend();
         ++rItTSoS) {
      if ((*rItTSoS)->type(Trk::TrackStateOnSurface::Measurement) &&
          !((*rItTSoS)->type(Trk::TrackStateOnSurface::Outlier)) &&
          (*rItTSoS)->measurementOnTrack() != nullptr &&
          !((*rItTSoS)->measurementOnTrack()->type(
            Trk::MeasurementBaseType::PseudoMeasurementOnTrack))) {
        trkPar = (*rItTSoS)->measurementOnTrack();
        break;
      }
    }
  } else {
    ATH_MSG_WARNING("Track particle without Trk::Track");
  }
  if (!trkPar) {
    return 0;
  }
  const Trk::Surface& sf = trkPar->associatedSurface();
  const Identifier tid = sf.associatedDetectorElementIdentifier();
  return m_trtId->barrel_ec(tid);
}
