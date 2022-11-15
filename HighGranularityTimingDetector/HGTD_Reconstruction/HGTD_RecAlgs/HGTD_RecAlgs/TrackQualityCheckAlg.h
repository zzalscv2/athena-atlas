/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/TrackQualityCheckAlg.h
 * @author Valentina Raskina <valentina.raskina@cern.ch>
 * @author
 * @date July, 2022
 *
 * @brief Gets the track time extension and checks the time compatibility 
 *
 * TODO:
 * 
 */

#ifndef HGTD_RECALGS_TRACKQUALITYCHECKALG_H
#define HGTD_RECALGS_TRACKQUALITYCHECKALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "GeneratorObjects/McEventCollection.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include <string>

namespace HGTD {

class TrackQualityCheckAlg : public AthReentrantAlgorithm {

public:
  TrackQualityCheckAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrackQualityCheckAlg() {}
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{this, "TrackParticleContainerName", "InDetTrackParticles", "Name of the TrackParticle container"};

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerHasExtensionKey{this, "HGTD_has_extension", "InDetTrackParticles.HGTD_has_extension", "deco with a handle for an extension"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerExtensionChi2Key{this, "HGTD_extension_chi2", "InDetTrackParticles.HGTD_extension_chi2", "deco with a handle for a ch2 of extension"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterRawTimeKey{this, "HGTD_cluster_raw_time", "InDetTrackParticles.HGTD_cluster_raw_time", "deco with a handle for layer cluster raw time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTimeKey{this, "HGTD_cluster_time", "InDetTrackParticles.HGTD_cluster_time", "deco with a handle for cluster time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTruthClassKey{this, "HGTD_cluster_truth_class", "InDetTrackParticles.HGTD_cluster_truth_class", "deco with a handle for a truth time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterShadowedKey{this, "HGTD_cluster_shadowed", "InDetTrackParticles.HGTD_cluster_shadowed", "deco with a handle for a shadowed cluster"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterMergedKey{this, "HGTD_cluster_merged", "InDetTrackParticles.HGTD_cluster_merged", "deco with a handle for a merged cluster"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerPrimaryExpectedKey{this, "HGTD_primary_expected", "InDetTrackParticles.HGTD_primary_expected", "deco with a handle for an expected primary"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extrapXKey{this, "HGTD_extrap_x", "InDetTrackParticles.HGTD_extrap_x", "deco with a handle for an x of extrap"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extrapYKey{this, "HGTD_extrap_y", "InDetTrackParticles.HGTD_extrap_y", "deco with a handle for an y of extrap"};

  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_compatibleHitsTimesKey{this, "HGTD_times_of_compatible_hits", "InDetTrackParticles.HGTD_times_of_compatible_hits", "deco with a handle for the time compatible hits' times"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_compatibleHitsPrimeInfoKey{this, "HGTD_compatible_hits_isprime", "InDetTrackParticles.HGTD_hits_isprime", "deco with a handle for the primary info for time compatible hits"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_lastHitInITkCutKey{this, "HGTD_last_hit_in_ITk_cut", "InDetTrackParticles.HGTD_last_hit_in_ITk_cut", "deco with a handle for the last hit to be close to HGTDrequirement"};

  //TODO: the resolution is fixed to 0.035 ps, should add the resolution calculation after the irradiation
  struct Hit {
    float m_time = 0.;
    bool m_isprime = false;
    float m_resolution = 0.035;
  };

  FloatProperty m_chi2_threshold{this, "Chi2Threshold", 1.5,
                           "Quality cut for decision to keep hits compatible in time"};
  FloatProperty m_deltat_cut{this, "DeltaTCut", 2.0,
                           "Upper limit for a cluster delta t cut"};
  // using HitVec_t = std::vector<Hit>;
  // using FloatVec_t = std::vector<float>;
  float calculateChi2(const std::vector<Hit>& hits) const;
  std::vector<TrackQualityCheckAlg::Hit> 
      getValidHits(const xAOD::TrackParticle* track_particle) const;
  bool passesDeltaT(const std::vector<Hit>& hits) const;
  std::vector<TrackQualityCheckAlg::Hit> 
      getTimeCompatibleHits(const xAOD::TrackParticle* track_particle) const;
  bool lastHitIsOnLastSurface(const xAOD::TrackParticle& track_particle) const;
  const Trk::TrackParameters* getLastHitOnTrack(const Trk::Track& track) const;

};

} // namespace HGTD

#endif // HGTD_RECALGS_TRACKQUALITYCHECKALG_H
