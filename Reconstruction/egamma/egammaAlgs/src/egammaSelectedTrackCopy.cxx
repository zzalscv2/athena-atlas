/*
   Copyright (C) 2002-2021  CERN for the benefit of the ATLAS collaboration
 */

/*
NAME:     egammaSelectedTrackCopy
PACKAGE:  offline/Reconstruction/egamma/egammaAlgs/egammaSelectedTrackCopy
AUTHORS:  Anastopoulos
CREATED:  25/06/2018

PURPOSE: Select track to be refitted later on with GSF
UPDATE : 25/06/2018
*/

#include "egammaSelectedTrackCopy.h"
//
#include "AthenaKernel/errorcheck.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "egammaUtils/CandidateMatchHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
// std includes
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

egammaSelectedTrackCopy::egammaSelectedTrackCopy(const std::string& name,
                                                 ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode
egammaSelectedTrackCopy::initialize()
{
  ATH_CHECK(m_clusterContainerKey.initialize());
  ATH_CHECK(m_fwdClusterContainerKey.initialize());
  ATH_CHECK(m_trackParticleContainerKey.initialize());
  ATH_CHECK(m_OutputTrkPartContainerKey.initialize());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_egammaCaloClusterSelector.retrieve());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());

   m_doForwardTracks =!m_fwdClusterContainerKey.empty() &&
                      !m_OutputFwdTrkPartContainerKey.empty();

    ATH_CHECK(m_fwdClusterContainerKey.initialize(m_doForwardTracks));
    ATH_CHECK(m_OutputFwdTrkPartContainerKey.initialize(m_doForwardTracks));

  return StatusCode::SUCCESS;
}

StatusCode
egammaSelectedTrackCopy::egammaSelectedTrackCopy::finalize()
{
  ATH_MSG_INFO("--- egamma Selected Track Copy Statistics ---");
  ATH_MSG_INFO("--- All Clusters: " << m_AllClusters);
  ATH_MSG_INFO("--- Selected Clusters: " << m_SelectedClusters);
  ATH_MSG_INFO("--- All Tracks: " << m_AllTracks);
  ATH_MSG_INFO("--- Selected Tracks: " << m_SelectedTracks);
  ATH_MSG_INFO("--- All Si Tracks: " << m_AllSiTracks);
  ATH_MSG_INFO("--- Selected Si Tracks: " << m_SelectedSiTracks);
  ATH_MSG_INFO("--- All TRT Tracks: " << m_AllTRTTracks);
  ATH_MSG_INFO("--- Selected TRT Tracks: " << m_SelectedTRTTracks);
  ATH_MSG_INFO("---------------------------------------------");

  return StatusCode::SUCCESS;
}

StatusCode
egammaSelectedTrackCopy::execute(const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::CaloClusterContainer> clusterTES(m_clusterContainerKey, ctx);
  if (!clusterTES.isValid()) {
    ATH_MSG_FATAL(
      "Failed to retrieve cluster container: " << 
      m_clusterContainerKey.key()
    );

    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::TrackParticleContainer> trackTES(m_trackParticleContainerKey, ctx);
  if (!trackTES.isValid()) {
    ATH_MSG_FATAL(
      "Failed to retrieve TrackParticle container: " << 
      m_trackParticleContainerKey.key()
    );

    return StatusCode::FAILURE;
  }

  SG::WriteHandle<ConstDataVector<xAOD::TrackParticleContainer>> outputTrkPartContainer(
    m_OutputTrkPartContainerKey, 
    ctx
  );

  SG::ReadHandle<xAOD::CaloClusterContainer> fwdClusterTES;
  SG::WriteHandle<ConstDataVector<xAOD::TrackParticleContainer>> outputFwdTrkPartContainer;
  if (m_doForwardTracks) {
    fwdClusterTES = SG::ReadHandle<xAOD::CaloClusterContainer>(
      m_fwdClusterContainerKey, 
      ctx
    );

    if (!fwdClusterTES.isValid()) {
      ATH_MSG_FATAL(
        "Failed to retrieve forward cluster container: " << 
        m_fwdClusterContainerKey.key()
      );

      return StatusCode::FAILURE;
    }

    outputFwdTrkPartContainer = SG::WriteHandle<ConstDataVector<xAOD::TrackParticleContainer>> (
      m_OutputFwdTrkPartContainerKey, 
      ctx
    );
  }

  // Here it just needs to be a view copy , i.e the collection of selected 
  // trackParticles we create does not really own its elements.
  auto viewCopy = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(SG::VIEW_ELEMENTS);
  auto fwdViewCopy = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(SG::VIEW_ELEMENTS);

  // Local counters.
  auto allClusters = m_AllClusters.buffer();
  auto selectedClusters = m_SelectedClusters.buffer();
  auto allTracks = m_AllTracks.buffer();
  auto selectedTracks = m_SelectedTracks.buffer();
  auto allSiTracks = m_AllSiTracks.buffer();
  auto selectedSiTracks = m_SelectedSiTracks.buffer();
  auto allTRTTracks = m_AllTRTTracks.buffer();
  auto selectedTRTTracks = m_SelectedTRTTracks.buffer();

  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle{m_caloDetDescrMgrKey, ctx};
  ATH_CHECK(caloDetDescrMgrHandle.isValid());

  const CaloDetDescrManager* calodetdescrmgr = *caloDetDescrMgrHandle;

  // Check which clusters to seed on.
  std::vector<const xAOD::CaloCluster*> passingClusters;
  for (const xAOD::CaloCluster* cluster : *clusterTES) {
    if (m_egammaCaloClusterSelector->passSelection(cluster, *calodetdescrmgr)) {
      passingClusters.push_back(cluster);
    }
  }
      
  allClusters += clusterTES->size();
  selectedClusters += passingClusters.size();

  // Extrapolation cache.
  for (const xAOD::TrackParticle* track : *trackTES) {
    ++allTracks;
    bool isTRT = false;
    int nhits(0);
    uint8_t dummy(0);
    if (track->summaryValue(dummy, xAOD::numberOfPixelHits)) {
      nhits += dummy;
    }
    if (track->summaryValue(dummy, xAOD::numberOfSCTHits)) {
      nhits += dummy;
    }
    if (nhits < 4) {
      isTRT = true;
      ++allTRTTracks;
    } else {
      isTRT = false;
      ++allSiTracks;
    }

    for (const xAOD::CaloCluster* cluster : passingClusters) {
      // Check if the track is selected due to this cluster and if not 
      // continue to next cluster.
      if (!selectTrack(ctx, cluster, track, isTRT, *calodetdescrmgr)) {
        continue;
      }
      viewCopy->push_back(track);
      ++selectedTracks;
      if (isTRT) {
        ++selectedTRTTracks;
      } else {
        ++selectedSiTracks;
      }
      
      // The particular track got selected due to a cluster (any one of them will 
      // do) break here and move to the next track.
      break;
    } // Loop on clusters.

    if (m_doForwardTracks) { fwdViewCopy->push_back(track); }
  }   // Loop on tracks.

  ATH_CHECK(outputTrkPartContainer.record(std::move(viewCopy)));
  if (m_doForwardTracks) { ATH_CHECK(outputFwdTrkPartContainer.record(std::move(fwdViewCopy))); }

  return StatusCode::SUCCESS;
}

bool
egammaSelectedTrackCopy::selectTrack(const EventContext& ctx,
                                     const xAOD::CaloCluster* cluster,
                                     const xAOD::TrackParticle* track,
                                     bool trkTRT,
                                     const CaloDetDescrManager& caloDD) const
{

  if (cluster == nullptr || track == nullptr) {
    return false;
  }

  const Trk::Perigee& candidatePerigee = track->perigeeParameters();

  // Get Perigee Parameters.
  const double trkPhi = candidatePerigee.parameters()[Trk::phi];
  const double trkEta = candidatePerigee.eta();
  const double z_perigee = candidatePerigee.position().z();
  const double r_perigee = candidatePerigee.position().perp();
  const Amg::Vector3D PerigeeXYZPosition(candidatePerigee.position().x(),
                                         candidatePerigee.position().y(),
                                         z_perigee);

  // Get Cluster parameters.
  const double clusterEta = xAOD::EgammaHelpers::isFCAL(cluster) ? cluster->eta() : cluster->etaBE(2);
  const bool isEndCap = !xAOD::EgammaHelpers::isBarrel(cluster);
  
  // Use trkEta only if sufficient hits in the Si.
  const double Et = trkTRT ? cluster->et() : cluster->e() / cosh(trkEta);
  
  // A few sanity checks.
  if (std::abs(clusterEta) > 10.0 || std::abs(trkEta) > 10.0 || Et < 10.0) {
    ATH_MSG_DEBUG(
      "FAILS sanity checks :  Track Eta : " << trkEta << 
      ", Cluster Eta " << clusterEta
    );

    return false;
  }

  // Calculate the eta/phi of the cluster as would be seen from the perigee
  // position of the Track.
  const Amg::Vector3D XYZClusterWrtTrackPerigee = CandidateMatchHelpers::approxXYZwrtPoint(
    *cluster, 
    PerigeeXYZPosition, 
    isEndCap
  );
  
  // Calculate the possible rotation of the track.
  // Once assuming the cluster Et being the better estimate (e.g big brem).
  const double phiRotRescaled = CandidateMatchHelpers::PhiROT(
    Et,
    trkEta, 
    track->charge(),
    r_perigee,
    isEndCap
  );

  // And also assuming the track Pt being correct.
  const double phiRotTrack = CandidateMatchHelpers::PhiROT(
    track->pt(),
    trkEta,
    track->charge(),
    r_perigee, 
    isEndCap
  );

  const double clusterPhiCorrected = XYZClusterWrtTrackPerigee.phi();

  // DeltaPhi between the track and the cluster.
  const double deltaPhiStd = P4Helpers::deltaPhi(clusterPhiCorrected, trkPhi);
  
  // DeltaPhi between the track and the cluster accounting for rotation assuming
  // cluster Et is a better estimator.
  const double trkPhiRescaled = P4Helpers::deltaPhi(trkPhi, phiRotRescaled);
  const double deltaPhiRescaled = P4Helpers::deltaPhi(clusterPhiCorrected, trkPhiRescaled);

  // DeltaPhi between the track and the cluster accounting for rotation.
  const double trkPhiCorrTrack = P4Helpers::deltaPhi(trkPhi, phiRotTrack);
  const double deltaPhiTrack = P4Helpers::deltaPhi(clusterPhiCorrected, trkPhiCorrTrack);
  
  // First we will see if it fails the quick match.
  // Then if it passed it will get 2 chances to be selected.
  // One if it matches from last measurement.
  // The second if it matched from Perigee rescales.
  // Broad phi check.
  if (
    (std::abs(deltaPhiRescaled) > m_broadDeltaPhi) &&
    (std::abs(deltaPhiTrack) > m_broadDeltaPhi) &&
    (std::abs(deltaPhiStd) > m_broadDeltaPhi)
  ) {
    ATH_MSG_DEBUG(
      "FAILS broad window phi match (track phi, phirotCluster , phiRotTrack , " <<
      "cluster phi corrected, cluster phi): ( " << 
      trkPhi << ", " << 
      phiRotRescaled << ", " << 
      phiRotTrack << ", " << 
      clusterPhiCorrected << ", " << 
      cluster->phi() << ")"
    );

    return false;
  }
  
  // if TRT we can stop here , we can not check much in eta really.
  if (trkTRT) { return true; }

  const double clusterEtaCorrected = XYZClusterWrtTrackPerigee.eta();

  // Broad eta check.
  if (
    std::abs(cluster->eta() - trkEta) > m_broadDeltaEta &&
    std::abs(clusterEtaCorrected - trkEta) > m_broadDeltaEta
  ) {
    ATH_MSG_DEBUG(
      "FAILS broad window eta match (track eta, cluster eta, cluster eta corrected): ( " << 
      trkEta << ", " << 
      cluster->etaBE(2) << ", " <<
      clusterEtaCorrected << " )"
    );

    return false;
  }

  std::pair<
    std::vector<CaloSampling::CaloSample>,
    std::vector<std::unique_ptr<Trk::Surface>>
  > layersAndSurfaces = m_extrapolationTool->getClusterLayerSurfaces(*cluster, caloDD);

  // Extrapolate from last measurement to the four EM layers.
  // Since this is before brem fit last measurement is OK.
  std::array<double, 4> eta = { -999.0, -999.0, -999.0, -999.0 };
  std::array<double, 4> phi = { -999.0, -999.0, -999.0, -999.0 };
  std::array<double, 4> deltaEta = { -999.0, -999.0, -999.0, -999.0 };
  std::array<double, 4> deltaPhi = { -999.0, -999.0, -999.0, -999.0 };
  if (m_extrapolationTool ->getMatchAtCalo(
    ctx,
    *cluster,
    *track,
    layersAndSurfaces.first,
    layersAndSurfaces.second,
    eta,
    phi,
    deltaEta,
    deltaPhi,
    IEMExtrapolationTools::fromLastMeasurement
  ).isFailure()) {
    return false;
  }

  // Selection in narrow eta/phi window from last measurement.
  if (
    std::abs(deltaEta[2]) < m_narrowDeltaEta &&
    deltaPhi[2] < m_narrowDeltaPhi && 
    deltaPhi[2] > -m_narrowDeltaPhiBrem
  ) {
    ATH_MSG_DEBUG("Match from Last measurement is successful :  " << deltaPhi[2]); 
    return true;
  }

  // Cases where:
  // - It passes deltaEta[2] from last measurement (rescaling should not affect the eta side).
  // - and we have a cluster with higher Et.
  // Rescale up the track to account for radiative loses and retry.
  if (
    std::abs(deltaEta[2]) < m_narrowDeltaEta && 
    cluster->et() > track->pt()
  ) {
    // Extrapolate from Perigee Rescaled.
    std::array<double, 4> etaRes = { -999.0, -999.0, -999.0, -999.0 };
    std::array<double, 4> phiRes = { -999.0, -999.0, -999.0, -999.0 };
    std::array<double, 4> deltaEtaRes = { -999.0, -999.0, -999.0, -999.0 };
    std::array<double, 4> deltaPhiRes = { -999.0, -999.0, -999.0, -999.0 };

    if (m_extrapolationTool->getMatchAtCalo(
      ctx,
      *cluster,
      *track,
      layersAndSurfaces.first,
      layersAndSurfaces.second,
      etaRes,
      phiRes,
      deltaEtaRes,
      deltaPhiRes,
      IEMExtrapolationTools::fromPerigeeRescaled
    ).isFailure()) {
      return false;
    }

    // Redo the check with rescale.
    if (
      std::abs(deltaEtaRes[2]) < m_narrowDeltaEta &&
      deltaPhiRes[2] < m_narrowRescale &&
      deltaPhiRes[2] > -m_narrowRescaleBrem
    ) {
      ATH_MSG_DEBUG("Rescale Match success " << deltaPhiRes[2]);
      return true;
    }
  }
  
  // Default is fail.
  return false;
}
