/**
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecTools/src/HGTD_IterativeExtensionTool.cxx
 * @author Noemi Calace <noemi.calace@cern.ch>
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date August, 2021
 * @brief
 */

#include "HGTD_IterativeExtensionTool.h"

#include "GeneratorObjects/McEventCollection.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_RIO_OnTrack/HGTD_ClusterOnTrack.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include <iostream>
#include <vector>

HGTD_IterativeExtensionTool::HGTD_IterativeExtensionTool(const std::string& t,
                                                         const std::string& n,
                                                         const IInterface* p)
    : base_class(t, n, p) {}

StatusCode HGTD_IterativeExtensionTool::initialize() {
  StatusCode sc = AthAlgTool::initialize();

  ATH_CHECK(m_extrapolator.retrieve());

  ATH_CHECK(m_updator.retrieve());

  ATH_CHECK(m_tof_corr_tool.retrieve());

  ATH_CHECK(m_truth_tool.retrieve());

  // get HGTD Detector Description Manager and HGTD Helper
  ATH_CHECK(detStore()->retrieve(m_hgtd_det_mgr, "HGTD"));
  ATH_CHECK(detStore()->retrieve(m_hgtd_id_helper, "HGTD_ID"));

  return sc;
}

HGTD::ExtensionObject HGTD_IterativeExtensionTool::extendTrackToHGTD(
    const EventContext& ctx, const xAOD::TrackParticle& track_ptkl,
    const HGTD_ClusterContainer* container, const HepMC::GenEvent* hs_event,
    const InDetSimDataCollection* sim_data) {

  ATH_MSG_DEBUG("Start extending");

  const Trk::Track* track = track_ptkl.track();

  HGTD::ExtensionObject result;

  // get last hit on track in ITk as a starting point for the extrapolation
  const Trk::TrackStateOnSurface* last_hit = getLastHitOnTrack(*track);
  const Trk::TrackParameters* last_param = last_hit->trackParameters();

  ATH_MSG_DEBUG("last param  x: " << last_param->position().x()
                                  << ", y: " << last_param->position().y()
                                  << ", z: " << last_param->position().z());
  ATH_MSG_DEBUG("last param  px: " << last_param->momentum().x()
                                   << ", py: " << last_param->momentum().y()
                                   << ", pz: " << last_param->momentum().z());

  const xAOD::TruthParticle* truth_ptkl =
      xAOD::TruthHelpers::getTruthParticle(track_ptkl);

  // get the tracking geometry
  const Trk::TrackingGeometry* trk_geom = m_extrapolator->trackingGeometry();
  if (not trk_geom) {
    ATH_MSG_DEBUG("trackingGeometry returns null");
    return result;
  }

  bool is_pos_endcap = last_param->eta() > 0;

  // get the target volume
  const Trk::TrackingVolume* hgtd_trk_volume = trk_geom->trackingVolume(
      is_pos_endcap ? "HGTD::PositiveEndcap" : "HGTD::NegativeEndcap");

  if (not hgtd_trk_volume) {
    ATH_MSG_DEBUG("trackingVolume returns null");
    return result;
  }

  const Trk::BinnedArray<Trk::Layer>* confined_layers =
      hgtd_trk_volume->confinedLayers();
  // careful, this array is not ordered from inside out (only in pos endcap)
  if (not confined_layers) {
    ATH_MSG_DEBUG("confinedLayers returns null");
    return result;
  }

  // get the layers, traverse depending on endcap used
  // since they are not in ascending z order!!
  Trk::BinnedArraySpan<const Trk::Layer* const> layers =
      confined_layers
          ->arrayObjects(); // is CxxUtils::span<const Trk::Layer* const>
  size_t layer_size = layers.size();

  short position = is_pos_endcap ? 0 : layer_size - 1;
  short step = is_pos_endcap ? 1 : -1;
  short hgtd_layer_i = -1; // start at -1 since the ++ happens at the beginning
  for (size_t i = 0; i < layer_size - 1; i++, position = position + step) {

    const Trk::Layer* layer = layers[position];
    // only use it, if it is an active one
    if (layer->layerType() != 1) {
      continue;
    }
    hgtd_layer_i++;

    const Trk::Surface& surf_obj = layer->surfaceRepresentation();
    ATH_MSG_DEBUG("extrapolation surface z: " << surf_obj.center().z());

    // if it is a good surface, extrapolate to this surface
    // uses same particle hypothesis used for the track itself
    // TODO: BoundaryCheck set to false as in 20.20 -> what does this do?
    std::unique_ptr<const Trk::TrackParameters> extrap_result = nullptr;

    extrap_result = m_extrapolator->extrapolate(
        ctx, *last_param, surf_obj, Trk::PropDirection::alongMomentum, false,
        track->info().particleHypothesis());

    //
    if (not extrap_result) {
      ATH_MSG_WARNING("Extrapolator returned null");
      result.m_hits.at(hgtd_layer_i) = nullptr;
      result.m_truth_primary_hits.at(hgtd_layer_i) = nullptr;
      result.m_truth_primary_info.at(hgtd_layer_i) = HGTD::ClusterTruthInfo();
      continue;
    }

    // get the extrapolation position info only on the first layer
    if (hgtd_layer_i == 0) {
      result.m_extrap_x = extrap_result->position().x();
      result.m_extrap_y = extrap_result->position().y();
    }

    ATH_MSG_DEBUG("extrap. params  x: "
                  << extrap_result->position().x()
                  << ", y: " << extrap_result->position().y()
                  << ", z: " << extrap_result->position().z());
    ATH_MSG_DEBUG("extrap. params  px: "
                  << extrap_result->momentum().x()
                  << ", py: " << extrap_result->momentum().y()
                  << ", pz: " << extrap_result->momentum().z());

    // find active surfaces in the vincinity
    auto compatible_surfaces = getCompatibleSurfaces(*extrap_result, layer);

    if (compatible_surfaces.empty()) {
      ATH_MSG_WARNING("No compatible surfaces for position");
      result.m_hits.at(hgtd_layer_i) = nullptr;
      result.m_truth_primary_hits.at(hgtd_layer_i) = nullptr;
      result.m_truth_primary_info.at(hgtd_layer_i) = HGTD::ClusterTruthInfo();
      continue;
    }

    auto extrapolated_params =
        extrapolateToSurfaces(ctx, *last_param, compatible_surfaces);

    std::unique_ptr<const Trk::TrackStateOnSurface> updated_state =
        updateStateWithBestFittingCluster(track, extrapolated_params,
                                          container);

    if (not updated_state) {
      result.m_hits.at(hgtd_layer_i) = nullptr;
      result.m_truth_primary_hits.at(hgtd_layer_i) = nullptr;
      result.m_truth_primary_info.at(hgtd_layer_i) = HGTD::ClusterTruthInfo();
      continue;
    }
    // if the state was updated with a measurement, the it becomes the new last
    // parameter
    last_param = updated_state->trackParameters();
    // store the last track state to be returned

    std::pair<const HGTD_Cluster*, HGTD::ClusterTruthInfo> truth_info =
        getTruthMatchedCluster(compatible_surfaces, container, truth_ptkl,
                               hs_event, sim_data);

    result.m_hits.at(hgtd_layer_i) = std::move(updated_state);
    result.m_truth_primary_hits.at(hgtd_layer_i) = truth_info.first;
    result.m_truth_primary_info.at(hgtd_layer_i) = truth_info.second;
  }

  return result;
}

const Trk::TrackStateOnSurface*
HGTD_IterativeExtensionTool::getLastHitOnTrack(const Trk::Track& track) const {

  const Trk::TrackStates* tsos =
      track.trackStateOnSurfaces();
  if (not tsos) {
    ATH_MSG_ERROR("Failed to retrieve track state on surfaces");
    return nullptr;
  }
  // loop over the associated hits in ITk in reverse order, since we want to
  // select the one closest to HGTD to start the extrapolation
  for (auto i = tsos->rbegin(); i != tsos->rend(); ++i) {
    const auto* curr_last_tsos = *i;
    if (not curr_last_tsos) {
      continue;
    }
    if (curr_last_tsos->type(Trk::TrackStateOnSurface::Measurement) and
        curr_last_tsos->trackParameters() and
        curr_last_tsos->measurementOnTrack()) {
      return curr_last_tsos;
    }
  }
  return nullptr;
}

std::vector<const Trk::Surface*>
HGTD_IterativeExtensionTool::getCompatibleSurfaces(
    const Trk::TrackParameters& param, const Trk::Layer* layer) {

  std::vector<const Trk::Surface*> surfaces;

  // point of extrapolation as global position
  const Amg::Vector3D& position = param.position();
  // get the surface at the point of extrapolation
  const auto* surface_arr = layer->surfaceArray(); // these are the modules
  if (!surface_arr) {
    return surfaces;
  }
  const Trk::Surface* module_surface = surface_arr->object(
      position); // from this binned object, get the module closeby
  if (!module_surface) {
    return surfaces;
  }
  surfaces.push_back(module_surface);

  // pick up additional surfaces in a 4cm radius
  // TODO REPLACE THIS BY NEIGHBOUR FUNCTIONALITY IN FUTURE!!
  short steps = 16;
  float delta_angle = 6.2831853 / (float)steps;
  float angle = 0.;
  float radius = 20.0;
  for (short i = 0; i < steps; i++, angle = angle + delta_angle) {
    Amg::Vector3D delta(radius * std::cos(angle), radius * std::sin(angle), 0);
    Amg::Vector3D result = position + delta;
    const Trk::Surface* additional_surface = surface_arr->object(result);
    if (!additional_surface) {
      continue;
    }
    // check if surface was added to avoid duplicates
    if (std::find(surfaces.begin(), surfaces.end(), additional_surface) ==
        surfaces.end()) {
      surfaces.push_back(additional_surface);
    }
  }

  return surfaces;
}

std::vector<std::unique_ptr<const Trk::TrackParameters>>
HGTD_IterativeExtensionTool::extrapolateToSurfaces(
    const EventContext& ctx, const Trk::TrackParameters& param,
    const std::vector<const Trk::Surface*>& surfaces) const {

  std::vector<std::unique_ptr<const Trk::TrackParameters>> params;
  params.reserve(surfaces.size());

  for (const auto* surface : surfaces) {
    std::unique_ptr<const Trk::TrackParameters> extrapolated_params = nullptr;

    extrapolated_params = m_extrapolator->extrapolate(
        ctx, param, *surface, Trk::alongMomentum, false,
        static_cast<Trk::ParticleHypothesis>(m_particle_hypot.value()));
    if (not extrapolated_params) {
      continue;
    }
    params.push_back(std::move(extrapolated_params));
  }

  return params;
}

std::unique_ptr<const Trk::TrackStateOnSurface>
HGTD_IterativeExtensionTool::updateStateWithBestFittingCluster(
    const Trk::Track* track,
    const std::vector<std::unique_ptr<const Trk::TrackParameters>>& params,
    const HGTD_ClusterContainer* container) const {
  ATH_MSG_DEBUG("[updateStateWithBestFittingCluster] start updating");

  std::unique_ptr<const Trk::TrackStateOnSurface> updated_state = nullptr;

  double lowest_chi2 = -1.;
  // all compatible surfaces are tested for the best fitting cluster
  for (const auto& param : params) {
    std::unique_ptr<const Trk::TrackStateOnSurface> best_tsos =
        findBestCompatibleCluster(track, param.get(), container);
    if (not best_tsos) {
      ATH_MSG_DEBUG("[updateStateWithBestFittingCluster] tsos is null");
      continue;
    }
    ATH_MSG_DEBUG("[updateStateWithBestFittingCluster] tsos found");

    double chi2 = best_tsos->fitQualityOnSurface().chiSquared() /
                  best_tsos->fitQualityOnSurface().doubleNumberDoF();
    ATH_MSG_DEBUG(
        "[updateStateWithBestFittingCluster] found state with chi2 of "
        << chi2);
    // make sure only one TSOS is kept
    if (!updated_state or chi2 < lowest_chi2) {
      updated_state.swap(best_tsos);
      lowest_chi2 = chi2;
    }
  }

  return updated_state; // no need to std::move thanks to Return Value
                        // Optimization (RVO)
}

std::unique_ptr<const Trk::TrackStateOnSurface>
HGTD_IterativeExtensionTool::findBestCompatibleCluster(
    const Trk::Track* track, const Trk::TrackParameters* param,
    const HGTD_ClusterContainer* container) const {

  ATH_MSG_DEBUG("[findBestCompatibleCluster] start");

  std::unique_ptr<const Trk::TrackStateOnSurface> tsos = nullptr;

  double lowest_chi2 = -1;
  for (const auto* collection : *container) {
    // find the one collection that can be associated to the surface
    if (collection->identify() !=
        param->associatedSurface().associatedDetectorElementIdentifier()) {
      continue;
    }
    ATH_MSG_DEBUG(
        "[findBestCompatibleCluster] found collection of given surface");
    // test the clusters on this surface for compatibility, keep the best
    for (const auto* cluster : *collection) {
      // update the track parameters with the found cluster
      std::unique_ptr<const Trk::TrackStateOnSurface> candidate =
          updateState(track, param, cluster);

      if (not candidate) {
        continue;
      }
      if (not candidate->measurementOnTrack() and
          not candidate->fitQualityOnSurface()) {
        continue;
      }
      double chi2 = candidate->fitQualityOnSurface().chiSquared() /
                    candidate->fitQualityOnSurface().doubleNumberDoF();
      ATH_MSG_DEBUG("found cluster with chi2 of " << chi2);
      // apply cut on the chi2
      if (chi2 > m_chi2_cut) {
        continue;
      }
      // make sure only one TSOS is kept
      if (not tsos or chi2 < lowest_chi2) {
        tsos.swap(candidate);
        lowest_chi2 = chi2;
      }
    }
    // per trackparameter, there is only one fitting surface
    // break after having found it
    break;
  }

  return tsos; // no need to std::move thanks to Return Value Optimization (RVO)
}

std::unique_ptr<const Trk::TrackStateOnSurface>
HGTD_IterativeExtensionTool::updateState(const Trk::Track* track, const Trk::TrackParameters* param,
                                         const HGTD_Cluster* cluster) const {
  ATH_MSG_DEBUG("[updateState] calling the updator");

  // FIXME: the HGTD_Cluster should know its detector element here. needs
  // to be set up in the converter at some point!!
  const auto* det_el = m_hgtd_det_mgr->getDetectorElement(cluster->identify());

  // apply the time of flight correction before setting the time and resolution
  // in the HGTD_ClusterOnTrack
  std::pair<float, float> corr_time_and_res =
      m_tof_corr_tool->correctTimeAndResolution(
          *track, *cluster, cluster->time(), cluster->timeResolution());

  std::unique_ptr<HGTD_ClusterOnTrack> cot =
      std::make_unique<HGTD_ClusterOnTrack>(
          cluster, Trk::LocalParameters(cluster->localPosition()),
          Amg::MatrixX(cluster->localCovariance()), corr_time_and_res.first,
          corr_time_and_res.second, det_el->identifyHash());

  Trk::FitQualityOnSurface* quality = nullptr;

  std::unique_ptr<Trk::TrackParameters> pars = m_updator->addToState(
      *param, cot->localParameters(), cot->localCovariance(), quality);
  //Here one could  fix the addToState intefaces to
  //avoid them allocating memory for  Fit Quality On Surface
  auto uniqueQuality= std::unique_ptr<Trk::FitQualityOnSurface>(quality);
  auto QoS = uniqueQuality? *uniqueQuality : Trk::FitQualityOnSurface{};
  return std::make_unique<const Trk::TrackStateOnSurface>(QoS, std::move(cot),
                                                          std::move(pars));
}

std::pair<const HGTD_Cluster*, HGTD::ClusterTruthInfo>
HGTD_IterativeExtensionTool::getTruthMatchedCluster(
    const std::vector<const Trk::Surface*>& surfaces,
    const HGTD_ClusterContainer* container,
    const xAOD::TruthParticle* truth_ptkl, const HepMC::GenEvent* hs_event,
    const InDetSimDataCollection* sim_data) {

  if (not truth_ptkl or not sim_data) {
    if (not truth_ptkl)
      ATH_MSG_DEBUG("truth_ptkl is null");
    if (not sim_data)
      ATH_MSG_DEBUG("sim_data is null");

    return {nullptr, HGTD::ClusterTruthInfo()};
  }

  std::vector<Identifier> ids;
  std::for_each(surfaces.begin(), surfaces.end(),
                [&](const Trk::Surface* surf) {
                  ids.push_back(surf->associatedDetectorElementIdentifier());
                });

  for (const auto *const collection : *container) {
    // if the ID corresponding to this collection is not in the list, skip

    if (std::find(ids.begin(), ids.end(), collection->identify()) ==
        ids.end()) {
      continue;
    }
    for (const auto* cluster : *collection) {

      HGTD::ClusterTruthInfo truth_info = m_truth_tool->classifyCluster(
          cluster, truth_ptkl, sim_data, hs_event);
      // return cluster and truth info if the hit is matched to the truth
      // particle
      if (truth_info.origin == HGTD::ClusterTruthOrigin::TRUTH_PARTICLE) {
        return {cluster, truth_info};
      }
    }
  }
  // no matched cluster found
  return {nullptr, HGTD::ClusterTruthInfo()};
}
