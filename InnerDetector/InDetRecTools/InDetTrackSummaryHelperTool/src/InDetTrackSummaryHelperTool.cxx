/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSummaryHelperTool/InDetTrackSummaryHelperTool.h"

// forward declares
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"
// normal includes
#include "Identifier/Identifier.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkParameters/TrackParameters.h"

#include <cassert>

//==========================================================================
InDet::InDetTrackSummaryHelperTool::InDetTrackSummaryHelperTool(
  const std::string& t,
  const std::string& n,
  const IInterface* p)
  : base_class(t, n, p)
{}

//==========================================================================

StatusCode
InDet::InDetTrackSummaryHelperTool::initialize()
{
  if (m_usePixel) {
    if (detStore()->retrieve(m_pixelId, "PixelID").isFailure()) {
      ATH_MSG_ERROR("Could not get PixelID helper !");
      return StatusCode::FAILURE;
    }
  }

  if (m_useSCT) {
    if (detStore()->retrieve(m_sctId, "SCT_ID").isFailure()) {
      ATH_MSG_ERROR("Could not get SCT_ID helper !");
      return StatusCode::FAILURE;
    }
  }

  if (m_useTRT) {
    if (detStore()->retrieve(m_trtId, "TRT_ID").isFailure()) {
      ATH_MSG_ERROR("Could not get TRT_ID helper !");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(m_holeSearchTool.retrieve(DisableTool{m_holeSearchTool.empty()}));
  ATH_CHECK(m_TRTStrawSummaryTool.retrieve(
      DisableTool{not m_useTRT or m_TRTStrawSummaryTool.empty()}));

  return StatusCode::SUCCESS;
}

//==========================================================================
void
InDet::InDetTrackSummaryHelperTool::analyse(
  const EventContext& ctx,
  const Trk::Track& track,
  const Trk::RIO_OnTrack* rot,
  const Trk::TrackStateOnSurface* tsos,
  std::vector<int>& information,
  std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const
{
  const Identifier& id = rot->identify();
  bool isOutlier = tsos->type(Trk::TrackStateOnSurface::Outlier);
  bool ispatterntrack = (track.info().trackFitter() == Trk::TrackInfo::Unknown);

  if (m_usePixel and m_pixelId->is_pixel(id)
      and (not isOutlier or ispatterntrack)) {
    // ME: outliers on pattern tracks may be
    // reintegrated by fitter, so count them as hits

    information[Trk::numberOfPixelHits]++;
    if (m_pixelId->layer_disk(id) == 0 and m_pixelId->is_barrel(id))
      information[Trk::numberOfInnermostPixelLayerHits]++;
    if (m_pixelId->layer_disk(id) == 1 and m_pixelId->is_barrel(id))
      information[Trk::numberOfNextToInnermostPixelLayerHits]++;

    // check to see if there's an ambiguity with the ganged cluster.
    const PixelClusterOnTrack* pix = nullptr;
    if (rot->rioType(Trk::RIO_OnTrackType::PixelCluster)) {
      pix = static_cast<const PixelClusterOnTrack*>(rot);
    }
    if (not pix) {
      ATH_MSG_ERROR("Could not cast pixel RoT to PixelClusterOnTrack!");
    } else {
      if (pix->isBroadCluster())
	information[Trk::numberOfPixelSpoiltHits]++;
      if (pix->hasClusterAmbiguity()) {
	information[Trk::numberOfGangedPixels]++;
	if (pix->isFake())
	  information[Trk::numberOfGangedFlaggedFakes]++;
      }

      if ((m_pixelId->is_barrel(id))) {
	int offset = m_pixelId->layer_disk(id);
	if (not hitPattern.test(offset))
	  information[Trk::numberOfContribPixelLayers]++;
	hitPattern.set(offset); // assumes numbered consecutively
      } else {
	int offset = static_cast<int>(Trk::pixelEndCap0); // get int value of first pixel endcap disc
	offset += m_pixelId->layer_disk(id);
	if (not hitPattern.test(offset))
	  information[Trk::numberOfContribPixelLayers]++;
	hitPattern.set(offset); // assumes numbered consecutively
      }
    }

  } else if (m_useSCT and m_sctId->is_sct(id)
	     and (not isOutlier or ispatterntrack)) {
    // ME: outliers on pattern tracks may be
    // reintegrated by fitter, so count them as hits

    information[Trk::numberOfSCTHits]++;

    const InDet::SCT_ClusterOnTrack* sctclus = nullptr;
    if (rot->rioType(Trk::RIO_OnTrackType::SCTCluster)) {
      sctclus = static_cast<const InDet::SCT_ClusterOnTrack*>(rot);
    }
    if (not sctclus) {
      ATH_MSG_ERROR("Could not cast SCT RoT to SCT_ClusterOnTrack!");
    } else {
      if (sctclus->isBroadCluster())
	information[Trk::numberOfSCTSpoiltHits]++;
    }

    if ((m_sctId->is_barrel(id))) {
      int offset = static_cast<int>(Trk::sctBarrel0);
      hitPattern.set(offset + m_sctId->layer_disk(id)); // assumes numbered consecutively
    } else {
      int offset = static_cast<int>(Trk::sctEndCap0); // get int value of first sct endcap disc
      hitPattern.set(offset + m_sctId->layer_disk(id)); // assumes numbered consecutively
    }

  } else if (m_useTRT and m_trtId->is_trt(id)) {
    bool isArgonStraw = false;
    bool isKryptonStraw = false;
    if (not m_TRTStrawSummaryTool.empty()) {
      int statusHT = m_TRTStrawSummaryTool->getStatusHT(id, ctx);
      if (statusHT == TRTCond::StrawStatus::Argon or
          statusHT == TRTCond::StrawStatus::Dead or
          statusHT == TRTCond::StrawStatus::EmulateArgon) {
        isArgonStraw = true;
      }
      if (statusHT == TRTCond::StrawStatus::Krypton or
          statusHT == TRTCond::StrawStatus::EmulateKrypton) {
        isKryptonStraw = true;
      }
    }
    if (not isArgonStraw and not isKryptonStraw) {
      information[Trk::numberOfTRTXenonHits]++;
    }

    if (isOutlier and not ispatterntrack) {
      // ME: outliers on pattern tracks may be
      // reintegrated by fitter, so count them as hits
      information[Trk::numberOfTRTOutliers]++;

      const InDet::TRT_DriftCircleOnTrack* trtDriftCircle = nullptr;
      if (rot->rioType(Trk::RIO_OnTrackType::TRT_DriftCircle)) {
        trtDriftCircle = static_cast<const InDet::TRT_DriftCircleOnTrack*>(rot);
      }
      if (not trtDriftCircle) {
        ATH_MSG_ERROR("Could not cast TRT RoT to TRT_DriftCircleOnTracknot ");
      } else {
        if (trtDriftCircle->highLevel() and not isArgonStraw and
            not isKryptonStraw)
          information[Trk::numberOfTRTHighThresholdOutliers]++;
      }
    } else {
      information[Trk::numberOfTRTHits]++;
      double error2 = rot->localCovariance()(0, 0);
      if (error2 > 1)
        information[Trk::numberOfTRTTubeHits]++;

      const InDet::TRT_DriftCircleOnTrack* trtDriftCircle = nullptr;
      if (rot->rioType(Trk::RIO_OnTrackType::TRT_DriftCircle)) {
        trtDriftCircle = static_cast<const InDet::TRT_DriftCircleOnTrack*>(rot);
      }
      if (not trtDriftCircle) {
        ATH_MSG_ERROR("Could not cast TRT RoT to TRT_DriftCircleOnTracknot ");
      } else {
        if (trtDriftCircle->highLevel()) {
          if (not isArgonStraw and not isKryptonStraw)
            information[Trk::numberOfTRTHighThresholdHits]++;
          assert(Trk::numberOfTRTHighThresholdHitsTotal < information.size());
          information[Trk::numberOfTRTHighThresholdHitsTotal]++;
        }
      }
    }

  }

}

void
InDet::InDetTrackSummaryHelperTool::analyse(
  const EventContext& ctx,
  const Trk::Track& track,
  const Trk::CompetingRIOsOnTrack* crot,
  const Trk::TrackStateOnSurface* tsos,
  std::vector<int>& information,
  std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const
{
  // re-produce prior behaviour (i.e. just take most probable ROT)
  analyse(ctx,
          track,
          &crot->rioOnTrack(crot->indexOfMaxAssignProb()),
          tsos,
          information,
          hitPattern);
}

void
InDet::InDetTrackSummaryHelperTool::searchForHoles(
  const Trk::Track& track,
  std::vector<int>& information,
  const Trk::ParticleHypothesis partHyp) const
{
  ATH_MSG_DEBUG("Do hole search within HELPER, PLEASE FIX THIS AFTER 16.0.X");
  m_holeSearchTool->countHoles(track, information, partHyp);
}



void
InDet::InDetTrackSummaryHelperTool::addDetailedTrackSummary(
  const EventContext&,
  const Trk::Track&,
  Trk::TrackSummary&) const
{
}

StatusCode
InDet::InDetTrackSummaryHelperTool::finalize()
{
  return StatusCode::SUCCESS;
}
