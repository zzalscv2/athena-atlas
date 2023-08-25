/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 TrackParticleCreatorTool.cxx  -  Description
 -------------------
 begin   : Autumn 2003
 authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
 email   : andreas.wildauer@cern.ch, fredrik.akesson@cern.ch
 changes :

***************************************************************************/
#include "TrkParticleCreator/TrackParticleCreatorTool.h"
#include "TrkParticleCreator/DetailedHitInfo.h"

// forward declares
#include "Particle/TrackParticle.h"
#include "TrkTrack/Track.h"
#include "VxVertex/VxCandidate.h"

// normal includes
#include "AthContainers/DataVector.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "IdDictDetDescr/IdDictManager.h"

#include "InDetPrepRawData/PixelCluster.h"
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SiCluster.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"

#include "TrkEventPrimitives/CurvilinearUVT.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/JacobianLocalToCurvilinear.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrack/TrackStateOnSurface.h"

#include "xAODTracking/Vertex.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <vector>

// helper methods to print messages
template<class T>
inline MsgStream&
operator<<(MsgStream& msg_stream, const std::map<std::string, T>& elm_map)
{
  for (const std::pair<const std::string, T>& elm : elm_map) {
    msg_stream << " " << elm.first;
  }
  return msg_stream;
}

template<class T>
inline MsgStream&
operator<<(MsgStream& msg_stream, const std::vector<std::string>& elm_vector)
{
  for (const std::string& elm : elm_vector) {
    msg_stream << " " << elm;
  }
  return msg_stream;
}

namespace Trk {
const std::string TrackParticleCreatorTool::s_trtdEdxUsedHitsDecorationName{ "TRTdEdxUsedHits" };

namespace {
void
createEProbabilityMap(std::map<std::string, std::pair<Trk::eProbabilityType, bool>>& eprob_map)
{
  // key: name to be used to activate copying of the electron probability values to the xAOD
  // TrackParticle
  //      abd for those which are added as decoration the name to be used for the decoration
  // value.first: enum of the electron probability value
  // value.second: false is a non dynamic element of the xAOD TrackParticle and added via
  // setTrackSummary
  //               true  will be added as a decoration.
  eprob_map.insert(std::make_pair("eProbabilityComb", std::make_pair(Trk::eProbabilityComb, false)));
  eprob_map.insert(std::make_pair("eProbabilityHT", std::make_pair(Trk::eProbabilityHT, false)));

  // added as decorations
  eprob_map.insert(std::make_pair("eProbabilityToT", std::make_pair(Trk::eProbabilityToT, true)));
  eprob_map.insert(std::make_pair("eProbabilityBrem", std::make_pair(Trk::eProbabilityBrem, true)));
  eprob_map.insert(std::make_pair("eProbabilityNN", std::make_pair(Trk::eProbabilityNN, true)));
  eprob_map.insert(std::make_pair("TRTdEdx", std::make_pair(Trk::TRTdEdx, true)));
  eprob_map.insert(std::make_pair("TRTTrackOccupancy", std::make_pair(Trk::TRTTrackOccupancy, true)));
}

void
createExtraSummaryTypeMap(std::map<std::string, Trk::SummaryType>& extra_summary_type_map)
{
  extra_summary_type_map.insert(std::make_pair("TRTdEdxUsedHits", Trk::numberOfTRTHitsUsedFordEdx));
}
}

const SG::AuxElement::Accessor<uint8_t> TrackParticleCreatorTool::s_trtdEdxUsedHitsDecoration(
  TrackParticleCreatorTool::trtdEdxUsedHitsAuxName());

TrackParticleCreatorTool::TrackParticleCreatorTool(const std::string& t,
                                                   const std::string& n,
                                                   const IInterface* p)
  : base_class(t, n, p)
  , m_detID(nullptr)
  , m_pixelID(nullptr)
  , m_sctID(nullptr)
  , m_trtID(nullptr)
  , m_copyEProbabilities{}
  , m_decorateEProbabilities{}
  , m_decorateSummaryTypes{}
  , m_doIBL(false)
{
}

StatusCode
TrackParticleCreatorTool::initialize()
{

  ATH_MSG_DEBUG("initialize TrackParticleCreatorTool");
  ATH_CHECK(m_beamSpotKey.initialize());
  if (std::find(std::begin(m_perigeeOptions), std::end(m_perigeeOptions), m_perigeeExpression) ==
      std::end(m_perigeeOptions)) {
    ATH_MSG_ERROR("Unknown Configuration for Perigee Expression - please use one of "
                  << m_perigeeOptions);
    return StatusCode::FAILURE;
  }

  /* Retrieve track summary tool */
  if (!m_trackSummaryTool.empty()) {
    if (m_trackSummaryTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_trackSummaryTool);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_trackSummaryTool);
  } else {
    m_trackSummaryTool.disable();
  }

  if (detStore()->retrieve(m_detID, "AtlasID").isFailure()) {
    ATH_MSG_FATAL("Could not get AtlasDetectorID ");
    return StatusCode::FAILURE;
  }

  if (detStore()->retrieve(m_pixelID, "PixelID").isFailure()) {
    ATH_MSG_FATAL("Could not get PixelID ");
    return StatusCode::FAILURE;
  }

  if (detStore()->retrieve(m_sctID, "SCT_ID").isFailure()) {
    ATH_MSG_FATAL("Could not get SCT_ID ");
    return StatusCode::FAILURE;
  }

  if (detStore()->retrieve(m_trtID, "TRT_ID").isFailure()) {
    ATH_MSG_FATAL("Could not get TRT_ID ");
    return StatusCode::FAILURE;
  }

  if (!m_IBLParameterSvc.empty()) {
    if (m_IBLParameterSvc.retrieve().isFailure()) {
      ATH_MSG_FATAL("Could not retrieve IBLParameterSvc");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Retrieved tool " << m_IBLParameterSvc);
  }

  m_doIBL = !m_IBLParameterSvc.empty() && m_IBLParameterSvc->containsIBL();
  ATH_MSG_INFO("doIBL set to " << m_doIBL);

  if (m_doIBL && !m_IBLParameterSvc->contains3D()) {
    ATH_MSG_WARNING("Assuming hybrid 2D/3D IBL module composition, but geometry is all-planar");
  }

  /* Retrieve track to vertex from ToolService */
  if (m_trackToVertex.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool " << m_trackToVertex);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_trackToVertex);

  if (!m_hitSummaryTool.empty()) {
    /* Retrieve hit summary tool from ToolService */
    if (m_hitSummaryTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_hitSummaryTool);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_hitSummaryTool);

  } else {
    m_hitSummaryTool.disable();
  }
  ATH_CHECK(m_trackingVolumesSvc.retrieve());

  ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());

  StatusCode sc(StatusCode::SUCCESS);
  m_copyEProbabilities.clear();
  m_decorateEProbabilities.clear();
  m_decorateSummaryTypes.clear();

  if (!m_copyExtraSummaryName.empty()) {
    std::map<std::string, std::pair<Trk::eProbabilityType, bool>> eprob_map;
    std::map<std::string, Trk::SummaryType> extra_summary_type_map;
    createEProbabilityMap(eprob_map);
    createExtraSummaryTypeMap(extra_summary_type_map);

    std::vector<std::string> errors;
    for (const std::string& eprob_to_copy : m_copyExtraSummaryName) {
      std::map<std::string, std::pair<Trk::eProbabilityType, bool>>::const_iterator eprob_iter =
        eprob_map.find(eprob_to_copy);
      if (eprob_iter == eprob_map.end()) {
        std::map<std::string, Trk::SummaryType>::const_iterator extra_summary_type_iter =
          extra_summary_type_map.find(eprob_to_copy);
        if (extra_summary_type_iter == extra_summary_type_map.end()) {
          errors.push_back(eprob_to_copy);
        } else {
          m_decorateSummaryTypes.emplace_back(
            SG::AuxElement::Accessor<uint8_t>(extra_summary_type_iter->first),
            extra_summary_type_iter->second);
        }
      } else {
        if (!eprob_iter->second.second) {
          m_copyEProbabilities.push_back(eprob_iter->second.first);
        } else {
          m_decorateEProbabilities.emplace_back(SG::AuxElement::Accessor<float>(eprob_iter->first),
                                                eprob_iter->second.first);
        }
      }
    }

    if (!errors.empty()) {
      ATH_MSG_ERROR("Error in configuration. Unknown electron probability name: "
                    << errors << ". known are " << eprob_map << " " << extra_summary_type_map);
      sc = StatusCode::FAILURE;
    }
  }

  ATH_CHECK(  m_eProbabilityTool.retrieve( DisableTool{m_eProbabilityTool.empty()} ) );
  ATH_CHECK(  m_dedxtool.retrieve( DisableTool{m_dedxtool.empty()} ) );
  ATH_CHECK(  m_testPixelLayerTool.retrieve( DisableTool{m_testPixelLayerTool.empty()} ) );

  ATH_CHECK(m_assoMapContainer.initialize(!m_assoMapContainer.key().empty()));
  ATH_CHECK(m_clusterSplitProbContainer.initialize(m_runningTIDE_Ambi &&
						   !m_clusterSplitProbContainer.key().empty()));

  ATH_MSG_VERBOSE(" initialize successful.");
  return sc;
}

xAOD::TrackParticle*
TrackParticleCreatorTool::createParticle(const EventContext& ctx,
                                         const Trk::Track& track,
                                         xAOD::TrackParticleContainer* container,
                                         const xAOD::Vertex* vxCandidate,
                                         xAOD::ParticleHypothesis prtOrigin) const
{
  const Trk::Perigee* aPer = nullptr;
  const Trk::TrackParameters* parsToBeDeleted = nullptr;
  // Origin
  if (m_perigeeExpression == "Origin") {
    aPer = track.perigeeParameters();
    if (aPer) {
      // aMeasPer clone will be created later if all perigee option selected
      if (m_keepAllPerigee) {
        aPer = nullptr;
      }
    } else {
      const Amg::Vector3D persf(0, 0, 0);
      const Trk::Perigee* result = m_trackToVertex->perigeeAtVertex(ctx, track, persf).release();
      if (result != nullptr) {
        aPer = result;
        parsToBeDeleted = result;
      } else {
        ATH_MSG_WARNING("Could not extrapolate to 0,0,0. No TrackParticle created.");
        return nullptr;
      }
    }
    // Beamspot
  } else if (m_perigeeExpression == "BeamSpot") {
    const Trk::Perigee* result = m_trackToVertex->perigeeAtVertex(ctx, track, CacheBeamSpotData(ctx)->beamVtx().position()).release();
    if (!result) {
      ATH_MSG_WARNING("Failed to extrapolate to first Beamspot - No TrackParticle created.");
      return nullptr;
    }
    parsToBeDeleted = result;
    aPer = result;
  }
  // the non default way, express the perigee wrt. the vertex position
  else if (m_perigeeExpression == "Vertex") {
    if (vxCandidate != nullptr) {
      const Trk::Perigee* result = m_trackToVertex->perigeeAtVertex(ctx, track, vxCandidate->position()).release();
      if (result != nullptr) {
        parsToBeDeleted = result;
        aPer = result;
      } else {
        ATH_MSG_WARNING("Could not extrapolate track to vertex region! No TrackParticle created.");
        return nullptr;
      }
    } else {
      ATH_MSG_WARNING("Perigee expression at Vertex, but no vertex found! No TrackParticle created.");
    }
    //BeamLine
  } else if (m_perigeeExpression == "BeamLine") {
    const Trk::Perigee* result = m_trackToVertex->perigeeAtBeamline(ctx, track, CacheBeamSpotData(ctx)).release();
    if (!result) {
      ATH_MSG_WARNING("Failed to extrapolate to Beamline - No TrackParticle created.");
      return nullptr;
    }
    parsToBeDeleted = result;
    aPer = result;
  }
  /*
   * We start from the existing summary
   * and see what we want to add
   */
  std::unique_ptr<Trk::TrackSummary> updated_summary;
  const Trk::TrackSummary* summary = track.trackSummary();
  if (m_trackSummaryTool.get() != nullptr) {
    if (!track.trackSummary() || m_updateTrackSummary) {
      updated_summary = m_trackSummaryTool->summary(ctx, track);
      summary = updated_summary.get();
    }
  } else {
    ATH_MSG_VERBOSE(
      "No proper TrackSummaryTool found. Creating TrackParticle with a TrackSummary on track");
  }
  if (!summary) {
    ATH_MSG_WARNING("Track particle created for a track without a track summary");
  }

  // find the first and the last hit in track
  // we do that the same way as in the track slimming tool!
  // that way it is also ok on not slimmed tracks!
  std::vector<const Trk::TrackParameters*> parameters;
  std::vector<xAOD::ParameterPosition> parameterPositions;

  int nbc_meas_A1 = 0;
  int nbc_meas_B3 = 0;
  int nbc_meas_A1_or_B3 = 0;
  int nbc_meas_A1_or_B3_or_C = 0;

  int isBC_A1 = 0;
  int isBC_B3 = 0;
  int isBC_C = 0;

  const Trk::TrackStates* trackStates = track.trackStateOnSurfaces();
  const Trk::TrackParameters* first(nullptr);
  const Trk::TrackParameters* tp(nullptr);

  if (m_badclusterID != 0) {
    for (const TrackStateOnSurface* tsos : *trackStates) {
      if (tsos->type(TrackStateOnSurface::Measurement) && tsos->trackParameters() != nullptr &&
          tsos->measurementOnTrack() != nullptr &&
          !(tsos->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack))) {
        tp = tsos->trackParameters();

        const InDet::SiClusterOnTrack* clus =
          dynamic_cast<const InDet::SiClusterOnTrack*>(tsos->measurementOnTrack());
        if (!clus) {
          ATH_MSG_DEBUG("Failed dynamic_cast to InDet::SiClusterOnTrack ");
          continue;
        }
        const Trk::PrepRawData* prdc = nullptr;
        prdc = clus->prepRawData();
        if (!prdc) {
          ATH_MSG_DEBUG("No PRD for Si cluster");
        }
        const InDet::SiCluster* RawDataClus = dynamic_cast<const InDet::SiCluster*>(clus->prepRawData());
        if (!RawDataClus) {
          ATH_MSG_DEBUG("No RDC for Si cluster");
          continue;
        }
        const Trk::MeasurementBase* mesb = tsos->measurementOnTrack();

        if (RawDataClus->detectorElement()->isPixel()) {
          const InDetDD::SiDetectorElement* element = nullptr;
          const InDet::PixelCluster* pixelCluster =
            dynamic_cast<const InDet::PixelCluster*>(RawDataClus);
          if (!pixelCluster) {
            ATH_MSG_DEBUG("Pixel cluster null though detector element matches pixel");
          }

          else {
            float size = pixelCluster->rdoList().size();
            float tot = pixelCluster->totalToT();
            float charge = pixelCluster->totalCharge();
            float cotthetaz = -1;
            int zWidth = -1;

            element = pixelCluster->detectorElement();
            if (!element)
              ATH_MSG_DEBUG("No element for track incidence angles!");
            float PixTrkAngle = -1000;
            float PixTrkThetaI = -1000;
            float theta = -1000;
            if (element) {
              const Amg::Vector3D& my_track = tp->momentum();
              const Amg::Vector3D& my_normal = element->normal();
              const Amg::Vector3D& my_phiax = element->phiAxis();
              const Amg::Vector3D& my_etaax = element->etaAxis();
              // track component on etaAxis:
              float trketacomp = my_track.dot(my_etaax);
              // track component on phiAxis:
              float trkphicomp = my_track.dot(my_phiax);
              // track component on the normal to the module
              float trknormcomp = my_track.dot(my_normal);
              // Track angle
              PixTrkAngle = atan2(trkphicomp, trknormcomp);
              PixTrkThetaI = atan2(trketacomp, trknormcomp);
              float length =
                sqrt(trketacomp * trketacomp + trkphicomp * trkphicomp + trknormcomp * trknormcomp);
              theta = acos(trknormcomp / length);
              cotthetaz = 1. / tan(PixTrkThetaI);

              // reducing the angle in the right quadrant
              // M_PI (pi) and M_PI_2 (pi/2.) are defined in cmath.
              if (PixTrkThetaI > M_PI_2)
                PixTrkThetaI -= M_PI;
              else if (PixTrkThetaI < -M_PI_2)
                PixTrkThetaI += M_PI;
              PixTrkThetaI = M_PI_2 - PixTrkThetaI;
              if (PixTrkAngle > M_PI_2)
                PixTrkAngle -= M_PI;
              else if (PixTrkAngle < -M_PI_2)
                PixTrkAngle += M_PI;
              PixTrkAngle = M_PI_2 - PixTrkAngle;
              if (theta > M_PI_2)
                theta = M_PI - theta;
            }

            Identifier surfaceID;
            surfaceID = mesb->associatedSurface().associatedDetectorElement()->identify();
            if (m_detID->is_pixel(surfaceID)) {
              const InDet::SiWidth& width = pixelCluster->width();
              zWidth = static_cast<int>(width.colRow().y());
            }

            int isIBLclus = false;
            if (m_doIBL && m_pixelID->barrel_ec(surfaceID) == 0 &&
                m_pixelID->layer_disk(surfaceID) == 0) {
              isIBLclus = true;
            }

            // count bad clusters
            if (!isIBLclus) {
              if ((size == 1 && tot < 8) || (size == 2 && tot < 15)) {
                isBC_A1 = true;
                nbc_meas_A1++;
              }
              // Need to replace these magic numbers with constexpr with meaning full names
              if (charge < 13750. / cos(theta) - 22500.) {
                isBC_B3 = true;
                nbc_meas_B3++;
              }
              if (isBC_A1 || isBC_B3) {
                nbc_meas_A1_or_B3++;
              }
              if ((zWidth == 1 && cotthetaz > 5.8) || (zWidth == 2 && cotthetaz > 5.8) ||
                  (zWidth == 3 && cotthetaz > 6.2) || (zWidth > 3 && cotthetaz < 2.5)) {
                isBC_C = true;
              }
              if (isBC_A1 || isBC_B3 || isBC_C) {
                nbc_meas_A1_or_B3_or_C++;
              }
            }
          }
        }
      }
    }
  }
  if (m_keepParameters || m_keepFirstParameters) {
    // search first valid TSOS first
    for (const TrackStateOnSurface* tsos : *trackStates) {
      if (tsos->type(TrackStateOnSurface::Measurement) && tsos->trackParameters() != nullptr &&
          tsos->measurementOnTrack() != nullptr &&
          !(tsos->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack))) {
        first = tsos->trackParameters();
        parameters.push_back(tsos->trackParameters());
        parameterPositions.push_back(xAOD::FirstMeasurement);
        break;
      }
    }

    if (!m_keepFirstParameters) {
      // search last valid TSOS first
      for (Trk::TrackStates::const_reverse_iterator rItTSoS = trackStates->rbegin();
           rItTSoS != trackStates->rend();
           ++rItTSoS) {
        if ((*rItTSoS)->type(TrackStateOnSurface::Measurement) &&
            (*rItTSoS)->trackParameters() != nullptr && (*rItTSoS)->measurementOnTrack() != nullptr &&
            !((*rItTSoS)->measurementOnTrack()->type(
              Trk::MeasurementBaseType::PseudoMeasurementOnTrack))) {
          if (!(first == (*rItTSoS)->trackParameters())) {
            parameters.push_back((*rItTSoS)->trackParameters());
            parameterPositions.push_back(xAOD::LastMeasurement);
          }
          break;
        }
      }
    }

    // security check:
    if (parameters.size() > 2)
      ATH_MSG_WARNING("More than two additional track parameters to be stored in TrackParticle!");
  }

  // KeepAllPerigee will keep all perigee's on the track plus the parameters at the first
  // measurement, provided this measurement precedes any second perigee. The track (initial) perigee
  // is the 'defining parameter' for the TrackParticle, by convention this is pushed to the back of
  // the parameter vector by the TP constructor.
  else if (m_keepAllPerigee) {
    bool haveFirstMeasurementParameters = false;
    for (const TrackStateOnSurface* tsos : *(track.trackStateOnSurfaces())) {
      if (!tsos->trackParameters())
        continue;

      if (!haveFirstMeasurementParameters && tsos->type(TrackStateOnSurface::Measurement) &&
          !tsos->type(TrackStateOnSurface::Outlier) && tsos->measurementOnTrack() &&
          !(tsos->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack))) {
        haveFirstMeasurementParameters = true;
        parameters.push_back(tsos->trackParameters());
        ATH_MSG_VERBOSE(" including first measurement parameters at R "
                        << tsos->trackParameters()->position().perp() << ", Z "
                        << tsos->trackParameters()->position().z());
        parameterPositions.push_back(xAOD::FirstMeasurement);
        continue;
      }
      if (!tsos->type(TrackStateOnSurface::Perigee) ||
          !(tsos->trackParameters()->surfaceType() == Trk::SurfaceType::Perigee) ||
          !(tsos->trackParameters()->type() == Trk::AtaSurface)) {
        continue;
      }
      if (!aPer) {
        aPer = static_cast<const Perigee*>(tsos->trackParameters());
      } else {
        parameters.push_back(tsos->trackParameters());
      }

      ATH_MSG_VERBOSE(" including perigee at R " << tsos->trackParameters()->position().perp() << ", Z "
                                                 << tsos->trackParameters()->position().z());

      // we are not interested in keeping measurement parameters after any second perigee
      if (!parameters.empty())
        haveFirstMeasurementParameters = true;
    }
  }

  xAOD::TrackParticle* trackparticle = createParticle(ctx,
                                                      aPer,
                                                      track.fitQuality(),
                                                      &track.info(),
                                                      summary,
                                                      parameters,
                                                      parameterPositions,
                                                      prtOrigin,
                                                      container,
                                                      &track);

  static const SG::AuxElement::Accessor<int> nbCmeas("nBC_meas");
  switch (m_badclusterID) {
    case 1: {
      nbCmeas(*trackparticle) = nbc_meas_A1;
      break;
    }
    case 2: {
      nbCmeas(*trackparticle) = nbc_meas_B3;
      break;
    }
    case 3: {
      nbCmeas(*trackparticle) = nbc_meas_A1_or_B3;
      break;
    }
    case 4: {
      nbCmeas(*trackparticle) = nbc_meas_A1_or_B3_or_C;
      break;
    }
    default: {
    }
  }

  delete parsToBeDeleted;
  return trackparticle;
}

xAOD::TrackParticle*
TrackParticleCreatorTool::createParticle(const EventContext& ctx,
                                         const Rec::TrackParticle& trackParticle,
                                         xAOD::TrackParticleContainer* container) const
{

  // Attempt to fill the position enums - will necessarily be a bit of a hack, since we don't have
  // all the information.
  std::vector<xAOD::ParameterPosition> positions;
  bool firstMeasurement = false;
  for (const auto* parameter : trackParticle.trackParameters()) {
    if (!firstMeasurement && parameter && !parameter->associatedSurface().isFree()) {
      // if the surface isn't free, it must belong to a detector element => measurement
      firstMeasurement = true;
      positions.push_back(xAOD::FirstMeasurement);
    } else if (firstMeasurement && parameter && !parameter->associatedSurface().isFree()) {
      // Making the (possibly unfounded assumption that if we have the first measurement, the next
      // will be the last)
      positions.push_back(xAOD::LastMeasurement);
    } else {
      positions.push_back(xAOD::BeamLine); // Don't have a default yet!
    }
  }

  xAOD::TrackParticle* trackparticle =
    createParticle(ctx,
                   trackParticle.measuredPerigee(),
                   trackParticle.fitQuality(),
                   &trackParticle.info(),
                   trackParticle.trackSummary(),
                   trackParticle.trackParameters(),
                   positions,
                   static_cast<xAOD::ParticleHypothesis>(trackParticle.info().particleHypothesis()),
                   container,
                   nullptr);

  if (!trackparticle) {
    ATH_MSG_WARNING("WARNING: Problem creating TrackParticle - Returning 0");
    return nullptr;
  }

  trackparticle->setTrackLink(*(trackParticle.trackElementLink()));

  if (m_checkConversion)
    compare(trackParticle, *trackparticle);

  return trackparticle;
}

xAOD::TrackParticle*
TrackParticleCreatorTool::createParticle(const EventContext& ctx,
                                         const ElementLink<TrackCollection>& trackLink,
                                         xAOD::TrackParticleContainer* container,
                                         const xAOD::Vertex* vxCandidate,
                                         xAOD::ParticleHypothesis prtOrigin) const
{

  xAOD::TrackParticle* trackparticle =
    createParticle(ctx, **trackLink, container, vxCandidate, prtOrigin);

  if (!trackparticle) {
    ATH_MSG_WARNING("WARNING: Problem creating TrackParticle - Returning 0");
    return nullptr;
  }

  trackparticle->setTrackLink(trackLink);

  return trackparticle;
}

inline xAOD::TrackParticle* TrackParticleCreatorTool::createParticle(
                      const EventContext& ctx,
                      const Perigee* perigee,
                      const FitQuality* fq,
                      const TrackInfo* trackInfo,
                      const TrackSummary* summary,
                      const std::vector<const Trk::TrackParameters*>& parameters,
                      const std::vector<xAOD::ParameterPosition>& positions,
                      xAOD::ParticleHypothesis prtOrigin,
                      xAOD::TrackParticleContainer* container) const {
   return createParticle(ctx,perigee, fq, trackInfo, summary, parameters,  positions, prtOrigin, container, nullptr);
}

xAOD::TrackParticle*
TrackParticleCreatorTool::createParticle(const EventContext& ctx,
                                         const Perigee* perigee,
                                         const FitQuality* fq,
                                         const TrackInfo* trackInfo,
                                         const TrackSummary* summary,
                                         const std::vector<const Trk::TrackParameters*>& parameters,
                                         const std::vector<xAOD::ParameterPosition>& positions,
                                         xAOD::ParticleHypothesis prtOrigin,
                                         xAOD::TrackParticleContainer* container,
                                         const Trk::Track *track) const
{

  xAOD::TrackParticle* trackparticle = new xAOD::TrackParticle;
  if (!trackparticle) {
    ATH_MSG_WARNING("WARNING: Problem creating TrackParticle - Returning 0");
    return nullptr;
  }
  /*
   * The following needs care as in one case the ownership
   * can be passed to StoreGate i.e to the relevant container
   * DataVector.
   * In the other the caller has the ownership
   */

  if (container) {
    container->push_back(trackparticle);
  } else {
    trackparticle->makePrivateStore();
  }

  // Fit quality
  if (fq) {
    setFitQuality(*trackparticle, *fq);
  }
  // Track Info
  if (trackInfo) {
    setTrackInfo(*trackparticle, *trackInfo, prtOrigin);
  }
  // track summary
  if (summary) {
    setTrackSummary(*trackparticle, *summary);
    setHitPattern(*trackparticle, summary->getHitPattern());
    addPIDInformation(ctx, track, *trackparticle);
    if(m_doITk) addDetailedHitInformation(track->trackStateOnSurfaces(), *trackparticle);
  }

  if (m_computeAdditionalInfo && track!=nullptr) {
    addExpectedHitInformation(track->perigeeParameters(), *trackparticle);
    addOutlierHitInformation(track->trackStateOnSurfaces(), *trackparticle);
    if(m_doSharedSiHits || m_doSharedTRTHits) addSharedHitInformation(track, *trackparticle);
    else if(m_doITk) addDummyEndcapSharedHitInformation(*trackparticle);
  }

  const auto* beamspot = CacheBeamSpotData(ctx);
  if (beamspot) {
    setTilt(*trackparticle, beamspot->beamTilt(0), beamspot->beamTilt(1));
  }
  // Parameters
  if (perigee) {
    setDefiningParameters(*trackparticle, *perigee);
  } else {
    ATH_MSG_WARNING("Track without perigee parameters? Not setting any defining parameters!");
  }
  setParameters(ctx, *trackparticle, parameters, positions);

  return trackparticle;
}

void
TrackParticleCreatorTool::compare(const TrackParameters& tp1, const TrackParameters& tp2) const
{
  int index = Amg::compare(tp1.parameters(), tp2.parameters(), 1e-6, true);
  if (index != -1) {
    ATH_MSG_WARNING("Bad parameters conversion " << Amg::toString(tp1.parameters(), 7) << " --- "
                                                 << Amg::toString(tp2.parameters(), 7));
  }
  if ((tp1.covariance() && !tp2.covariance()) || (!tp1.covariance() && tp2.covariance())) {
    ATH_MSG_WARNING("Bad Covariance conversion " << tp1.covariance() << " --- " << tp2.covariance());
  } else if (tp1.covariance() && tp2.covariance()) {
    std::pair<int, int> indices = Amg::compare(*tp1.covariance(), *tp2.covariance(), 1e-6, true);
    if (indices.first != -1)
      ATH_MSG_WARNING("Bad Covariance conversion " << std::endl
                                                   << Amg::toString(*tp1.covariance(), 10) << std::endl
                                                   << Amg::toString(*tp2.covariance(), 10));
  }
}

void
TrackParticleCreatorTool::compare(const Rec::TrackParticle& tp, const xAOD::TrackParticle& tpx) const
{
  if (tp.measuredPerigee()) {
    compare(*tp.measuredPerigee(), tpx.perigeeParameters());
  }

  // trackParticle.info(),trackParticle.trackSummary(),
  if (tp.trackParameters().size() != tpx.numberOfParameters()) {
    ATH_MSG_WARNING("Number of parameters not the same " << tp.trackParameters().size() << " --- "
                                                         << tpx.numberOfParameters());
  }
}

void
TrackParticleCreatorTool::setParameters(const EventContext& ctx,
                                        xAOD::TrackParticle& tp,
                                        const std::vector<const Trk::TrackParameters*>& parameters,
                                        const std::vector<xAOD::ParameterPosition>& positions) const
{
  std::vector<std::vector<float>> parametersVec;
  parametersVec.resize(parameters.size());
  unsigned int numParam = 0;

  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{ m_fieldCacheCondObjInputKey, ctx };
  const AtlasFieldCacheCondObj* fieldCondObj{ *readHandle };
  MagField::AtlasFieldCache fieldCache;
  fieldCondObj->getInitializedCache(fieldCache);

  for (const auto* param : parameters) {
    std::vector<float>& values = parametersVec[numParam];
    values.resize(6);
    const Amg::Vector3D& pos = param->position();
    const Amg::Vector3D& mom = param->momentum();
    values[0] = pos[0];
    values[1] = pos[1];
    values[2] = pos[2];
    values[3] = mom[0];
    values[4] = mom[1];
    values[5] = mom[2];
    
    const bool straightPars = (!fieldCache.solenoidOn() && m_trackingVolumesSvc->volume(Trk::ITrackingVolumesSvc::CalorimeterEntryLayer).inside(pos)) ||
                              (!fieldCache.toroidOn() && !m_trackingVolumesSvc->volume(Trk::ITrackingVolumesSvc::MuonSpectrometerEntryLayer).inside(pos) &&
                                   m_trackingVolumesSvc->volume(Trk::ITrackingVolumesSvc::MuonSpectrometerExitLayer).inside(pos));
    AmgSymMatrix(5) covarianceMatrix;
    covarianceMatrix.setIdentity();

    if (param->covariance()) {
      // has covariance matrix
      // now convert from to Curvilinear -- to be double checked for correctness
        CurvilinearUVT curvilinearUVT(mom.unit());
        if (!straightPars){
            Amg::Vector3D magnFieldVect;
            magnFieldVect.setZero();
            fieldCache.getField(pos.data(), magnFieldVect.data());
            const Amg::Transform3D& localToGlobalTransform = param->associatedSurface().transform();

            JacobianLocalToCurvilinear jacobian(magnFieldVect,
                                                  param->parameters()[Trk::qOverP],
                                                  std::sin(param->parameters()[Trk::theta]),
                                                  curvilinearUVT,
                                                  localToGlobalTransform.rotation().col(0),
                                                  localToGlobalTransform.rotation().col(1));

            covarianceMatrix = param->covariance()->similarity(jacobian);
        } else {
           const Amg::Vector3D loc_x {param->parameters()[Trk::locX],0,0};
           const Amg::Vector3D loc_y {0,param->parameters()[Trk::locY],0};
           JacobianLocalToCurvilinear jacobian(curvilinearUVT, loc_x, loc_y); 
           covarianceMatrix = param->covariance()->similarity(jacobian);
        }
    }
    std::vector<float> covMatrixVec;
    Amg::compress(covarianceMatrix, covMatrixVec);
    tp.setTrackParameterCovarianceMatrix(numParam, covMatrixVec);

    ++numParam;
  }

  tp.setTrackParameters(parametersVec);
  unsigned int i = 0;
  for (; i < positions.size(); ++i) {
    tp.setParameterPosition(i, positions[i]);
    if (positions[i] == xAOD::FirstMeasurement) {
      float x_position = tp.parameterX(i);
      float y_position = tp.parameterY(i);
      tp.setRadiusOfFirstHit(std::sqrt(x_position * x_position + y_position * y_position));
      tp.setIdentifierOfFirstHit(
        parameters[i]->associatedSurface().associatedDetectorElementIdentifier().get_compact());
    }
  }
}

void
TrackParticleCreatorTool::setTilt(xAOD::TrackParticle& tp, float tiltx, float tilty)
{
  tp.setBeamlineTiltX(tiltx);
  tp.setBeamlineTiltY(tilty);
}

void
TrackParticleCreatorTool::setHitPattern(xAOD::TrackParticle& tp, unsigned long hitpattern)
{
  tp.setHitPattern(hitpattern);
}

void
TrackParticleCreatorTool::setNumberOfUsedHits(xAOD::TrackParticle& tp, int hits)
{
  tp.setNumberOfUsedHitsdEdx(hits);
}

void
TrackParticleCreatorTool::setNumberOfOverflowHits(xAOD::TrackParticle& tp, int overflows)
{
  tp.setNumberOfIBLOverflowsdEdx(overflows);
}

void
TrackParticleCreatorTool::setTrackSummary(xAOD::TrackParticle& tp, const TrackSummary& summary) const
{
  // ensure that xAOD TrackSummary and TrackSummary enums are in sync.
  constexpr unsigned int xAodReferenceEnum1 = static_cast<unsigned int>(xAOD::numberOfTRTXenonHits);
  constexpr unsigned int TrkReferenceEnum1 = static_cast<unsigned int>(Trk::numberOfTRTXenonHits);
  static_assert(xAodReferenceEnum1 == TrkReferenceEnum1, "Trk and xAOD enums differ in their indices");
  constexpr unsigned int xAodReferenceEnum2 = static_cast<unsigned int>(xAOD::numberOfTRTTubeHits);
  constexpr unsigned int TrkReferenceEnum2 = static_cast<unsigned int>(Trk::numberOfTRTTubeHits);
  static_assert(xAodReferenceEnum2 == TrkReferenceEnum2, "Trk and xAOD enums differ in their indices");

  for (unsigned int i = 0; i < Trk::numberOfTrackSummaryTypes; i++) {
    // Only add values which are +ve (i.e., which were created)
    if (i >= Trk::numberOfMdtHits && i <= Trk::numberOfRpcEtaHits) {
      continue;
    }
    if (i == Trk::numberOfCscUnspoiltEtaHits) {
      continue;
    }
    if (i >= Trk::numberOfCscEtaHoles && i <= Trk::numberOfTgcPhiHoles) {
      continue;
    }
    // skip values which are floats
    if (std::find(unusedSummaryTypes.begin(), unusedSummaryTypes.end(), i) != unusedSummaryTypes.end()) {
      continue;
    }
    if (i >= Trk::numberOfStgcEtaHits && i <= Trk::numberOfMmHoles) {
      continue;
    }
    if (m_doITk && i == Trk::numberOfContribPixelLayers){ // Filled in addDetailedHitInformation for ITk
      continue;
    }

    int value = summary.get(static_cast<Trk::SummaryType>(i));
    uint8_t uvalue = static_cast<uint8_t>(value);
    // coverity[first_enum_type]
    if (value > 0) {
      tp.setSummaryValue(uvalue, static_cast<xAOD::SummaryType>(i));
    }
  }

  // muon hit info
  if (!m_hitSummaryTool.empty()) {
    ATH_MSG_DEBUG("now do muon hit info");
    Muon::IMuonHitSummaryTool::CompactSummary msSummary = m_hitSummaryTool->summary(summary);
    uint8_t numberOfPrecisionLayers = msSummary.nprecisionLayers;
    ATH_MSG_DEBUG("# of prec layers: " << static_cast<unsigned int>(numberOfPrecisionLayers));
    uint8_t numberOfPrecisionHoleLayers = msSummary.nprecisionHoleLayers;
    uint8_t numberOfPhiLayers = msSummary.nphiLayers;
    uint8_t numberOfPhiHoleLayers = msSummary.nphiHoleLayers;
    uint8_t numberOfTriggerEtaLayers = msSummary.ntrigEtaLayers;
    uint8_t numberOfTriggerEtaHoleLayers = msSummary.ntrigEtaHoleLayers;
    tp.setSummaryValue(numberOfPrecisionLayers, xAOD::numberOfPrecisionLayers);
    tp.setSummaryValue(numberOfPrecisionHoleLayers, xAOD::numberOfPrecisionHoleLayers);
    tp.setSummaryValue(numberOfPhiLayers, xAOD::numberOfPhiLayers);
    tp.setSummaryValue(numberOfPhiHoleLayers, xAOD::numberOfPhiHoleLayers);
    tp.setSummaryValue(numberOfTriggerEtaLayers, xAOD::numberOfTriggerEtaLayers);
    tp.setSummaryValue(numberOfTriggerEtaHoleLayers, xAOD::numberOfTriggerEtaHoleLayers);
  }

}

void
TrackParticleCreatorTool::addPIDInformation(const EventContext& ctx, const Trk::Track *track, xAOD::TrackParticle& tp) const
{
  // TRT PID information
  {
     static const std::vector<float> eProbabilityDefault(numberOfeProbabilityTypes,0.5);
     constexpr int initialValue{-1};
     std::vector<float> eProbability_tmp;
     const std::vector<float> &eProbability ( track && !m_eProbabilityTool.empty() ? eProbability_tmp = m_eProbabilityTool->electronProbability(ctx,*track)  : eProbabilityDefault);
     int nHits = track && !m_eProbabilityTool.empty() ? eProbability[Trk::eProbabilityNumberOfTRTHitsUsedFordEdx] : initialValue;
     for (const Trk::eProbabilityType& copy : m_copyEProbabilities) {
        float eProbability_value = eProbability.at(copy);
        tp.setSummaryValue(eProbability_value, static_cast<xAOD::SummaryType>(copy + xAOD::eProbabilityComb));
     }
     for (const std::pair<SG::AuxElement::Accessor<float>, Trk::eProbabilityType>& decoration :
             m_decorateEProbabilities) {
        float fvalue = eProbability.at(decoration.second);
        decoration.first(tp) = fvalue;
     }
     // now the extra summary types
     for (const std::pair<SG::AuxElement::Accessor<uint8_t>, Trk::SummaryType>& decoration :
             m_decorateSummaryTypes) {
        uint8_t summary_value = nHits;
        decoration.first(tp) = summary_value;
     }

  }

  // PixelPID
  {
     const int initialValue{-1};
     float dedx = initialValue;
     int nHitsUsed_dEdx = initialValue;
     int nOverflowHits_dEdx = initialValue;
     if (track && !m_dedxtool.empty() && track->info().trackFitter() != TrackInfo::Unknown) {
        dedx = m_dedxtool->dEdx(ctx, *track, nHitsUsed_dEdx, nOverflowHits_dEdx);
     }
     tp.setNumberOfUsedHitsdEdx(nHitsUsed_dEdx);
     tp.setNumberOfIBLOverflowsdEdx(nOverflowHits_dEdx);
     tp.setSummaryValue(dedx, static_cast<xAOD::SummaryType>(51));
  }
}

void
TrackParticleCreatorTool::addDetailedHitInformation(const Trk::TrackStates* trackStates, xAOD::TrackParticle& tp) const
{

  Trk::DetailedHitInfo detailedInfo;

  for (const TrackStateOnSurface* tsos : *trackStates) {

    const Trk::MeasurementBase* mesb = tsos->measurementOnTrack();
    if(mesb==nullptr) continue;
    // Check if the measurement type is RIO on Track (ROT)
    const RIO_OnTrack* rot = nullptr;
    if (mesb->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
      rot = static_cast<const RIO_OnTrack*>(mesb);
    }
    if(rot==nullptr) continue;

    const Identifier& id = rot->identify();
    if(!m_pixelID->is_pixel(id)) continue;

    Trk::DetectorRegion region;
    const InDetDD::SiDetectorElement* detEl = dynamic_cast<const InDetDD::SiDetectorElement*>(rot->detectorElement());
    InDetDD::DetectorType type = detEl->design().type();
    if(type==InDetDD::PixelInclined)  region = Trk::pixelBarrelInclined;
    else if(type==InDetDD::PixelBarrel) region = Trk::pixelBarrelFlat;
    else region = Trk::pixelEndcap;

    detailedInfo.addHit(region, m_pixelID->layer_disk(id), m_pixelID->eta_module(id));

  }

  uint8_t nContribPixelLayers = static_cast<uint8_t>(detailedInfo.getPixelContributions());

  uint8_t nContribPixelBarrelFlatLayers     = static_cast<uint8_t>(detailedInfo.getContributionFromRegion(Trk::pixelBarrelFlat    ));
  uint8_t nContribPixelBarrelInclinedLayers = static_cast<uint8_t>(detailedInfo.getContributionFromRegion(Trk::pixelBarrelInclined));
  uint8_t nContribPixelEndcap               = static_cast<uint8_t>(detailedInfo.getContributionFromRegion(Trk::pixelEndcap        ));

  uint8_t nPixelBarrelFlatHits     = static_cast<uint8_t>(detailedInfo.getHitsFromRegion(Trk::pixelBarrelFlat    ));
  uint8_t nPixelBarrelInclinedHits = static_cast<uint8_t>(detailedInfo.getHitsFromRegion(Trk::pixelBarrelInclined));
  uint8_t nPixelEndcapHits         = static_cast<uint8_t>(detailedInfo.getHitsFromRegion(Trk::pixelEndcap        ));

  uint8_t nInnermostPixelLayerEndcapHits = static_cast<uint8_t>(detailedInfo.getHits(Trk::pixelEndcap, 0));
  uint8_t nNextToInnermostPixelLayerEndcapHits = static_cast<uint8_t>(detailedInfo.getHits(Trk::pixelEndcap, 1)
								      + detailedInfo.getHits(Trk::pixelEndcap, 2)); // L0.5 shorties + L1

  tp.setSummaryValue(nContribPixelLayers, xAOD::numberOfContribPixelLayers);
  tp.setSummaryValue(nContribPixelBarrelFlatLayers, xAOD::numberOfContribPixelBarrelFlatLayers);
  tp.setSummaryValue(nContribPixelBarrelInclinedLayers, xAOD::numberOfContribPixelBarrelInclinedLayers);
  tp.setSummaryValue(nContribPixelEndcap, xAOD::numberOfContribPixelEndcap);
  tp.setSummaryValue(nPixelBarrelFlatHits, xAOD::numberOfPixelBarrelFlatHits);
  tp.setSummaryValue(nPixelBarrelInclinedHits, xAOD::numberOfPixelBarrelInclinedHits);
  tp.setSummaryValue(nPixelEndcapHits, xAOD::numberOfPixelEndcapHits);
  tp.setSummaryValue(nInnermostPixelLayerEndcapHits, xAOD::numberOfInnermostPixelLayerEndcapHits);
  tp.setSummaryValue(nNextToInnermostPixelLayerEndcapHits, xAOD::numberOfNextToInnermostPixelLayerEndcapHits);

}

void
TrackParticleCreatorTool::addExpectedHitInformation(const Perigee *perigee, xAOD::TrackParticle& tp) const
{

  if(m_testPixelLayerTool.empty() || perigee==nullptr) return;

  uint8_t zero = static_cast<uint8_t>(0);
  uint8_t nContribPixLayers, nInPixHits, nNextInPixHits;
  if(!tp.summaryValue(nContribPixLayers, xAOD::numberOfContribPixelLayers)){
    nContribPixLayers = zero;
  }
  if(nContribPixLayers==0){
    ATH_MSG_DEBUG("No pixels on track, so wo do not expect hit in inner layers");
    tp.setSummaryValue(zero, xAOD::expectInnermostPixelLayerHit);
    tp.setSummaryValue(zero, xAOD::expectNextToInnermostPixelLayerHit);
  }

  else {

    // Innermost pixel layer
    if(!tp.summaryValue(nInPixHits, xAOD::numberOfInnermostPixelLayerHits)){
      nInPixHits = zero;
    }
    if(nInPixHits>0){
      ATH_MSG_DEBUG("Innermost pixel Layer hit on track, so we expect an innermost pixel layer hit");
      uint8_t one = static_cast<uint8_t>(1);
      tp.setSummaryValue(one, xAOD::expectInnermostPixelLayerHit);
    } else {
      uint8_t expectHit = static_cast<uint8_t>(m_testPixelLayerTool->expectHitInInnermostPixelLayer(perigee));
      tp.setSummaryValue(expectHit, xAOD::expectInnermostPixelLayerHit);
    }

    // Next-to-innermost pixel layer
    if(!tp.summaryValue(nNextInPixHits, xAOD::numberOfNextToInnermostPixelLayerHits)){
      nNextInPixHits = zero;
    }
    if(nNextInPixHits>0){
      ATH_MSG_DEBUG("Next-to-innermost pixel Layer hit on track, so we expect an next-to-innermost pixel layer hit");
      uint8_t one = static_cast<uint8_t>(1);
      tp.setSummaryValue(one, xAOD::expectNextToInnermostPixelLayerHit);
    } else {
      uint8_t expectHit = static_cast<uint8_t>(m_testPixelLayerTool->expectHitInNextToInnermostPixelLayer(perigee));
      tp.setSummaryValue(expectHit, xAOD::expectNextToInnermostPixelLayerHit);
    }

  } // end else if nContribPixLayers>0

}

void
TrackParticleCreatorTool::addOutlierHitInformation(const Trk::TrackStates* trackStates, xAOD::TrackParticle& tp) const
{

  uint8_t nPixOutliers = 0, nInPixOutliers = 0, nNInPixOutliers = 0, nSCTOutliers = 0;
  uint8_t nInEndcapPixOutliers = 0, nNInEndcapPixOutliers = 0;

  for (const TrackStateOnSurface* tsos : *trackStates) {

    bool isOutlier = tsos->type(Trk::TrackStateOnSurface::Outlier);
    if(!isOutlier) continue;

    const Trk::MeasurementBase* mesb = tsos->measurementOnTrack();
    if(mesb==nullptr) continue;
    // Check if the measurement type is RIO on Track (ROT)
    const RIO_OnTrack* rot = nullptr;
    if (mesb->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
      rot = static_cast<const RIO_OnTrack*>(mesb);
    }
    if (rot == nullptr)
      continue;

    const Identifier& id = rot->identify();

    if (m_pixelID->is_pixel(id)) {
      nPixOutliers++;
      int layer = m_pixelID->layer_disk(id);
      if (m_pixelID->is_barrel(id)) {
        if (layer == 0){
          nInPixOutliers++;
        }
        else if (layer == 1){
          nNInPixOutliers++;
        }
      } else if (m_doITk) {  // isITk && isEndCap -> ITk specific counters
        if (layer == 0){
          nInEndcapPixOutliers++;
        }
        else if (layer == 1 || layer == 2){
          nNInEndcapPixOutliers++;  // L0.5 + L1 disks
        }
      }
    }
    else if (m_sctID->is_sct(id)) {
      nSCTOutliers++;
    }

    // TRT outliers are already filled in the InDetTrackSummaryHelperTool as
    // used in the ambi solver
  }

  tp.setSummaryValue(nPixOutliers,    xAOD::numberOfPixelOutliers);
  tp.setSummaryValue(nInPixOutliers,  xAOD::numberOfInnermostPixelLayerOutliers);
  tp.setSummaryValue(nNInPixOutliers, xAOD::numberOfNextToInnermostPixelLayerOutliers);
  tp.setSummaryValue(nSCTOutliers,    xAOD::numberOfSCTOutliers);

  if(m_doITk){
    tp.setSummaryValue(nInEndcapPixOutliers,  xAOD::numberOfInnermostPixelLayerEndcapOutliers);
    tp.setSummaryValue(nNInEndcapPixOutliers, xAOD::numberOfNextToInnermostPixelLayerEndcapOutliers);
  }

}

void
TrackParticleCreatorTool::addSharedHitInformation(const Track *track, xAOD::TrackParticle& tp) const
{

  uint8_t nPixSharedHits = 0, nInPixSharedHits = 0, nNInPixSharedHits = 0, nSCTSharedHits = 0, nTRTSharedHits = 0;
  uint8_t nInPixSharedEndcapHits = 0, nNInPixSharedEndcapHits = 0;
  uint8_t nPixSplitHits = 0, nInPixSplitHits = 0, nNInPixSplitHits = 0;
  uint8_t nInPixSplitEndcapHits = 0, nNInPixSplitEndcapHits = 0;

  const DataVector<const Trk::MeasurementBase>* measurements = track->measurementsOnTrack();
  if(!measurements){
    ATH_MSG_DEBUG("No measurement on track");
    return;
  }

  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<Trk::PRDtoTrackMap> prd_to_track_map(m_assoMapContainer, ctx);

  for(const auto* const ms : *measurements){
    // check if it's a rot
    const Trk::RIO_OnTrack* rot = nullptr;
    if(ms->type(Trk::MeasurementBaseType::RIO_OnTrack)){
      rot = static_cast<const Trk::RIO_OnTrack*>(ms);
    }
    if(!rot){
      ATH_MSG_DEBUG("Cannot cast measurement to RIO_OnTrack");
      continue;
    }

    const Identifier& id = rot->identify();

    if(m_doSharedSiHits && m_pixelID->is_pixel(id)){
      // check if split, when running TIDE
      bool hitIsSplit(false);
      if(m_runningTIDE_Ambi){
	const InDet::PixelClusterOnTrack* pix = nullptr;
	if(rot->rioType(Trk::RIO_OnTrackType::PixelCluster)){
	  pix = static_cast<const InDet::PixelClusterOnTrack*>(rot);
	}
	if(pix){
	  const InDet::PixelCluster* pixPrd = pix->prepRawData();
	  const Trk::ClusterSplitProbabilityContainer::ProbabilityInfo&
	    splitProb = getClusterSplittingProbability(pixPrd);
	  int layer = m_pixelID->layer_disk(id);
	  if(pixPrd and splitProb.isSplit()){
	    ATH_MSG_DEBUG("split Pixel hit found");
	    hitIsSplit = true;
	    nPixSplitHits++;
	    if(m_pixelID->is_barrel(id)){
	      if(layer==0)      nInPixSplitHits++;
	      else if(layer==1) nNInPixSplitHits++;
	    }
	    else if(m_doITk){ // isITk && isEndCap -> ITk specific counters
	      if(layer==0)                  nInPixSplitEndcapHits++;
	      else if(layer==1 || layer==2) nNInPixSplitEndcapHits++; // L0.5 + L1 disks
	    }
	  }
	}
      }

      // check if shared
      // if we are running the TIDE ambi don't count split hits as shared
      if(!hitIsSplit){
	if(prd_to_track_map->isShared(*(rot->prepRawData()))){
	  ATH_MSG_DEBUG("shared Pixel hit found");
	  nPixSharedHits++;
	  int layer = m_pixelID->layer_disk(id);
	  if(m_pixelID->is_barrel(id)){
	    if(layer==0)      nInPixSharedHits++;
	    else if(layer==1) nNInPixSharedHits++;
	  }
	  else if(m_doITk){ // isITk && isEndCap -> ITk specific counters
	    if(layer==0)                  nInPixSharedEndcapHits++;
	    else if(layer==1 || layer==2) nNInPixSharedEndcapHits++; // L0.5 + L1 disks
	  }
	}
      }

    } // end pixel

    else if(m_doSharedSiHits && m_sctID->is_sct(id)){
      if(prd_to_track_map->isShared(*(rot->prepRawData()))){
	ATH_MSG_DEBUG("shared SCT hit found");
	nSCTSharedHits++;
      }
    }

    else if(m_doSharedTRTHits && m_trtID->is_trt(id)){
      if(prd_to_track_map->isShared(*(rot->prepRawData()))){
	ATH_MSG_DEBUG("shared TRT hit found");
	nTRTSharedHits++;
      }
    }

  }

  tp.setSummaryValue(nPixSplitHits,     xAOD::numberOfPixelSplitHits);
  tp.setSummaryValue(nInPixSplitHits,   xAOD::numberOfInnermostPixelLayerSplitHits);
  tp.setSummaryValue(nNInPixSplitHits,  xAOD::numberOfNextToInnermostPixelLayerSplitHits);

  tp.setSummaryValue(nPixSharedHits,    xAOD::numberOfPixelSharedHits);
  tp.setSummaryValue(nInPixSharedHits,  xAOD::numberOfInnermostPixelLayerSharedHits);
  tp.setSummaryValue(nNInPixSharedHits, xAOD::numberOfNextToInnermostPixelLayerSharedHits);
  tp.setSummaryValue(nSCTSharedHits,    xAOD::numberOfSCTSharedHits);
  tp.setSummaryValue(nTRTSharedHits,    xAOD::numberOfTRTSharedHits);

  if(m_doITk){
    tp.setSummaryValue(nInPixSplitEndcapHits,   xAOD::numberOfInnermostPixelLayerSplitEndcapHits);
    tp.setSummaryValue(nNInPixSplitEndcapHits,  xAOD::numberOfNextToInnermostPixelLayerSplitEndcapHits);
    tp.setSummaryValue(nInPixSharedEndcapHits,  xAOD::numberOfInnermostPixelLayerSharedEndcapHits);
    tp.setSummaryValue(nNInPixSharedEndcapHits, xAOD::numberOfNextToInnermostPixelLayerSharedEndcapHits);
  }

}


void
TrackParticleCreatorTool::addDummyEndcapSharedHitInformation(xAOD::TrackParticle& tp) const
{

  uint8_t nInPixSharedEndcapHits = 0, nNInPixSharedEndcapHits = 0;
  uint8_t nInPixSplitEndcapHits = 0, nNInPixSplitEndcapHits = 0;

  tp.setSummaryValue(nInPixSplitEndcapHits,   xAOD::numberOfInnermostPixelLayerSplitEndcapHits);
  tp.setSummaryValue(nNInPixSplitEndcapHits,  xAOD::numberOfNextToInnermostPixelLayerSplitEndcapHits);
  tp.setSummaryValue(nInPixSharedEndcapHits,  xAOD::numberOfInnermostPixelLayerSharedEndcapHits);
  tp.setSummaryValue(nNInPixSharedEndcapHits, xAOD::numberOfNextToInnermostPixelLayerSharedEndcapHits);

}


const Trk::ClusterSplitProbabilityContainer::ProbabilityInfo&
TrackParticleCreatorTool::getClusterSplittingProbability(const InDet::PixelCluster* pix) const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  if (!pix || m_clusterSplitProbContainer.key().empty()) {
    return Trk::ClusterSplitProbabilityContainer::getNoSplitProbability();
  }
  SG::ReadHandle<Trk::ClusterSplitProbabilityContainer> splitProbContainer(m_clusterSplitProbContainer, ctx);
  if (!splitProbContainer.isValid()) {
    ATH_MSG_FATAL("Failed to get cluster splitting probability container "
		  << m_clusterSplitProbContainer);
  }
  return splitProbContainer->splitProbability(pix);
}


const InDet::BeamSpotData*
TrackParticleCreatorTool::CacheBeamSpotData(const EventContext& ctx) const
{
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
  return beamSpotHandle.cptr();
}

} // end of namespace Trk
